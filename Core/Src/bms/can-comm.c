/**
 * @file can-comm.c
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to handle CAN communication with other devices
 */

#include "can-comm.h"

#include <string.h>

#include "ring-buffer.h"
#include "bms_network.h"
#include "bms_watchdog.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

#define CAN_COMM_X_MACRO \
    CAN_COMM_X(CELLBOARD_STATUS, cellboard_status) \
    CAN_COMM_X(CELLS_VOLTAGES, cells_voltages) \
    CAN_COMM_X(CELLS_TEMPERATURES, cells_temperatures) \
    CAN_COMM_X(CELLBOARD_VERSION, cellboard_version) \
    CAN_COMM_X(MAINBOARD_STATUS, mainboard_status)

/**
 * @brief Generic definition of a function to translate the converted structure to the raw structure
 *
 * @param raw A pointer to the raw structure
 * @param converted A pointer to the converted structure
 */
typedef void (* can_comm_conversion_to_raw_callback)(void * raw, void * converted);

/**
 * @brief Generic definition of a function to translate the raw structure to the converted structure
 *
 * @param converted A pointer to the converted structure
 * @param raw A pointer to the raw structure
 */
typedef void (* can_comm_raw_to_conversion_callback)(void * converted, void * raw);

/**
 * @brief Generic definition of a function to pack the raw data
 *
 * @param buffer A pointer to a buffer of bytes
 * @param raw A pointer to the raw structure
 * @param size_t Length of the massage
 * @return int Sized of packed data or negative error code
 */
typedef int (* can_comm_pack_callback)(uint8_t * buffer, void * raw, size_t length);

/**
 * @brief Generic definition of a function to unpack the raw data
 *
 * @param buffer A pointer to a buffer of bytes
 * @param raw A pointer to the raw structure
 * @param size_t Length of the massage
 * @return int Zero or negative error code
 */
typedef int (* can_comm_unpack_callback)(uint8_t * buffer, void * raw, size_t length);


// TODO: Fill the arrays with the CAN messages info
#define CAN_COMM_X(upcase, lowcase) [BMS_##upcase##_INDEX] = BMS_##upcase##_BYTE_SIZE,
static uint8_t can_comm_byte_size[] = { CAN_COMM_X_MACRO };
#undef CAN_COMM_X

#define CAN_COMM_X(upcase, lowcase) [BMS_##upcase##_INDEX] = bms_##lowcase##_conversion_to_raw_struct,
static can_comm_conversion_to_raw_callback can_comm_conversion_to_raw[] = { CAN_COMM_X_MACRO };
#undef CAN_COMM_X

#define CAN_COMM_X(upcase, lowcase) [BMS_##upcase##_INDEX] = bms_##lowcase##_raw_to_conversion_struct,
static can_comm_raw_to_conversion_callback can_comm_raw_to_conversion[] = { CAN_COMM_X_MACRO };
#undef CAN_COMM_X

#define CAN_COMM_X(upcase, lowcase) [BMS_##upcase##_INDEX] = bms_##lowcase##_pack,
static can_comm_pack_callback can_comm_pack[] = { CAN_COMM_X_MACRO };
#undef CAN_COMM_X

#define CAN_COMM_X(upcase, lowcase) [BMS_##upcase##_INDEX] = bms_##lowcase##_unpack,
static can_comm_unpack_callback can_comm_unpack[] = { CAN_COMM_X_MACRO };
#undef CAN_COMM_X

static can_comm_update_canlib_payload_callback can_comm_update_canlib_payload[] = {
    // [CAN_ALIAS_MAINBOARD_STATUS] = ;
};

/**
 * @brief CAN manager handler structure
 *
 * @param enabled Flag used to enable or disable the CAN communication
 * @param tx_buf Transmission messages circular buffer
 * @param rx_buf Reception messages circular buffer
 */
static struct {
    bool enabled;
    RingBuffer(CanMessage, CELLBOARD_CAN_TX_BUFFER_BYTE_SIZE) tx_buf;
    RingBuffer(CanMessage, CELLBOARD_CAN_RX_BUFFER_BYTE_SIZE) rx_buf;

    can_comm_transmit_callback send;
} hcan_comm;

CanCommReturnCode can_comm_init(can_comm_transmit_callback send) {
    if (send == NULL)
        return CAN_COMM_NULL_POINTER;

    hcan_comm.enabled = false;
    hcan_comm.send = send;

    // Return values are ignored becuase the buffer addresses are always not NULL
    (void)ring_buffer_init(&hcan_comm.tx_buf, CanMessage, CELLBOARD_CAN_TX_BUFFER_BYTE_SIZE, NULL, NULL);
    (void)ring_buffer_init(&hcan_comm.rx_buf, CanMessage, CELLBOARD_CAN_RX_BUFFER_BYTE_SIZE, NULL, NULL);

    return CAN_COMM_OK;
}

void can_comm_set_enable(bool enabled) {
    hcan_comm.enabled = enabled;
}

bool can_comm_is_enabled(void) {
    return hcan_comm.enabled;
}

CanCommReturnCode can_comm_tx_add(can_index index, void * payload) {
    if (!hcan_comm.enabled)
        return CAN_COMM_DISABLED;

    // Check parameters validity
    if (index >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (payload == NULL)
        return CAN_COMM_NULL_POINTER;

    // Prepare and push message to the buffer
    CanMessage msg = {
        .index = index,
        .payload.tx = payload
    };
    if (ring_buffer_push_back(&hcan_comm.tx_buf, &msg) == RING_BUFFER_FULL)
        return CAN_COMM_OVERRUN;
    return CAN_COMM_OK;
}

CanCommReturnCode can_comm_rx_add(can_index index, uint8_t * data, size_t size) {    
    if (!hcan_comm.enabled)
        return CAN_COMM_DISABLED;

    // Check parameters validity
    if (index >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (data == NULL)
        return CAN_COMM_NULL_POINTER;
    if (size > CELLBOARD_CAN_MAX_PAYLOAD_BYTE_SIZE)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;

    // Prepare and push message to the buffer
    CanMessage msg = { .index = index };
    memcpy(msg.payload.rx, data, size);

    if (ring_buffer_push_back(&hcan_comm.rx_buf, &msg) == RING_BUFFER_FULL)
        return CAN_COMM_OVERRUN;
    return CAN_COMM_OK;
}

CanCommReturnCode can_comm_routine(void) {
    if (!hcan_comm.enabled)
        return CAN_COMM_DISABLED;

    // Handler transmit and receive data
    CanMessage tx_msg, rx_msg;
    if (ring_buffer_pop_front(&hcan_comm.tx_buf, &tx_msg) == RING_BUFFER_OK) {
        // Convert and pack message data
        uint8_t raw[CAN_COMM_MAX_PAYLOAD_BYTE_SIZE];
        uint8_t data[CAN_COMM_MAX_PAYLOAD_BYTE_SIZE];
        can_comm_conversion_to_raw[tx_msg.index](raw, tx_msg.payload.tx);
        int data_size = can_comm_pack[tx_msg.index](data, raw, can_comm_byte_size[tx_msg.index]);
        if (data_size < 0)
            return CAN_COMM_CONVERSION_ERROR;
        
        // TODO: Handle send errors?
        hcan_comm.send(bms_id_from_index(tx_msg.index), data, data_size);
    }
    if (ring_buffer_pop_front(&hcan_comm.rx_buf, &rx_msg) == RING_BUFFER_OK) {
        // Unpack and convert message data
        uint8_t raw[CAN_COMM_MAX_PAYLOAD_BYTE_SIZE];
        if (can_comm_unpack[rx_msg.index](raw, rx_msf.payload.rx, can_comm_byte_size[rx_msg.index]) < 0)
            return CAN_COMM_CONVERSION_ERROR;

        // TODO: Handle received data
        // can_comm_raw_to_conversion[rx_msg.index](converted, raw);
    }

    return CAN_COMM_OK;
}

#ifdef CONF_CAN_COMM_STRINGS_ENABLE

static char * can_comm_module_name = "can communication";

static char * can_comm_return_code_name[] = {
    [CAN_COMM_OK] = "ok",
    [CAN_COMM_NULL_POINTER] = "null pointer",
    [CAN_COMM_DISABLED] = "disabled",
    [CAN_COMM_OVERRUN] = "overrun",
    [CAN_COMM_INVALID_INDEX] = "invalid index",
    [CAN_COMM_INVALID_PAYLOAD_SIZE] = "invalid payload size",
    [CAN_COMM_CONVERSION_ERROR] = "conversion error",
    [CAN_COMM_TRANSMISSION_ERROR] = "transmission error"
};

static char * can_comm_return_code_description[] = {
    [CAN_COMM_OK] = "executed succesfully",
    [CAN_COMM_NULL_POINTER] = "attempt to dereference a NULL Pointer"
    [CAN_COMM_DISABLED] = "the can manager is not enabled"
    [CAN_COMM_OVERRUN] = "the transmission buffer is full"
    [CAN_COMM_INVALID_INDEX] = "the given index does not correspond to any valid message",
    [CAN_COMM_INVALID_PAYLOAD_SIZE] = "the payload size is greater than the maximum allowed length"
    [CAN_COMM_CONVERSION_ERROR] = "can't convert the message correctly",
    [CAN_COMM_TRANSMISSION_ERROR] = "error during message transmission"
};

#endif // CONF_CAN_COMM_STRINGS_ENABLE

#endif // CONF_CAN_COMM_MODULE_ENABLE
