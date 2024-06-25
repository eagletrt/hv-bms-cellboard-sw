/**
 * @file can-comm.c
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to handle CAN communication with other devices
 */

#include "can-comm.h"

#include <string.h>

#include "fsm.h"
#include "programmer.h"
#include "watchdog.h"
#include "timebase.h"
#include "bal.h"

#include "ring-buffer.h"
#include "canlib_device.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

/** @brief Mask for the bits that defines if the CAN module is enabled or not */
#define CAN_COMM_ENABLED_ALL_MASK \
    ( \
        (1U << CAN_COMM_RX_ENABLE_BIT) | \
        (1U << CAN_COMM_TX_ENABLE_BIT) \
    )

/**
 * @brief Enable a single bit of the internal flag
 *
 * @param FLAG The internal flag
 * @param BIT The bit of the flag to set
 */
#define CAN_COMM_ENABLE(FLAG, BIT) ((FLAG) = CELLBOARD_BIT_SET(FLAG, BIT))
/**
 * @brief Disable a single bit of the internal flag
 *
 * @param FLAG The internal flag
 * @param BIT The bit of the flag to reset
 */
#define CAN_COMM_DISABLE(FLAG, BIT) ((FLAG) = CELLBOARD_BIT_RESET(FLAG, BIT))
/**
 * @brief Toggle a single bit of the internal flag
 *
 * @param FLAG The internal flag
 * @param BIT The bit of the flag to flip
 */
#define CAN_COMM_TOGGLE(FLAG, BIT) ((FLAG) = CELLBOARD_BIT_TOGGLE(FLAG, BIT))
/**
 * @brief Check if a specific bit of the internal flag is set
 *
 * @param FLAG The internal flag
 * @param BIT The bit of the flag to check
 *
 * @return bool True if the bit is set, false otherwise
 */
#define CAN_COMM_IS_ENABLED(FLAG, BIT) CELLBOARD_BIT_GET(FLAG, BIT)

/**
 * @brief Enable all the bits of the internal flag
 *
 * @param FLAG The internal flag
 */
#define CAN_COMM_ENABLE_ALL(FLAG) ((FLAG) |= CAN_COMM_ENABLED_ALL_MASK)
/**
 * @brief Disable all the bits of the internal flag
 *
 * @param FLAG The internal flag
 */
#define CAN_COMM_DISABLE_ALL(FLAG) ((FLAG) &= ~CAN_COMM_ENABLED_ALL_MASK)
/**
 * @brief Toggle all the bits of the internal flag
 *
 * @param FLAG The internal flag
 */
#define CAN_COMM_TOGGLE_ALL(FLAG) ((FLAG) ^= CAN_COMM_ENABLED_ALL_MASK)
/**
 * @brief Check if all the bits of the internal flag are set
 *
 * @param FLAG The internal flag
 *
 * @return bool True if all the bits are set, false otherwise
 */
#define CAN_COMM_IS_ENABLED_ALL(FLAG) (((FLAG) & CAN_COMM_ENABLED_ALL_MASK) == CAN_COMM_ENABLED_ALL_MASK)


/**
 * @brief CAN manager handler structure
 *
 * @details The enabled bit flag 
 *
 * @param enabled Flag used to enable or disable the CAN communication
 * @param tx_buf Transmission messages circular buffer
 * @param rx_buf Reception messages circular buffer
 * @param send A pointer to the callback used to send the data via CAN
 * @param rx_device The reception canlib message handler
 * @param rx_raw The reception raw data of the message
 * @param rx_conv The reception converted data of the message
 */
static struct CanCommHandler  {
    bit_flag8_t enabled;
    RingBuffer(CanMessage, CAN_COMM_TX_BUFFER_BYTE_SIZE) tx_buf;
    RingBuffer(CanMessage, CAN_COMM_RX_BUFFER_BYTE_SIZE) rx_buf;

    can_comm_transmit_callback_t send;

    // Canlib devices
    device_t rx_device;
    uint8_t rx_raw[bms_MAX_STRUCT_SIZE_RAW];
    uint8_t rx_conv[bms_MAX_STRUCT_SIZE_CONVERSION];
} hcan_comm;


void _can_comm_canlib_payload_handle_dummy(void * _) { }

can_comm_canlib_payload_handle_callback _can_comm_payload_handle(can_index_t index) {
    switch (index) {
        case BMS_CELLBOARD_FLASH_REQUEST_INDEX:
            return (can_comm_canlib_payload_handle_callback)programmer_flash_request_handle;
        case BMS_CELLBOARD_FLASH_INDEX:
            return (can_comm_canlib_payload_handle_callback)programmer_flash_handle;
        case BMS_CELLBOARD_SET_BALANCING_STATUS_INDEX:
            return (can_comm_canlib_payload_handle_callback)bal_set_balancing_status_handle;
        default:
            return _can_comm_canlib_payload_handle_dummy;
    }
}

CanCommReturnCode can_comm_init(can_comm_transmit_callback_t send) {
    if (send == NULL)
        return CAN_COMM_NULL_POINTER;

    CAN_COMM_DISABLE_ALL(hcan_comm.enabled);
    hcan_comm.send = send;

    // Return values are ignored becuase the buffer addresses are always not NULL
    (void)ring_buffer_init(&hcan_comm.tx_buf, CanMessage, CAN_COMM_TX_BUFFER_BYTE_SIZE, NULL, NULL);
    // TODO: Add callbacks to stop CAN reception interrupt during ring buffer operations
    (void)ring_buffer_init(&hcan_comm.rx_buf, CanMessage, CAN_COMM_RX_BUFFER_BYTE_SIZE, NULL, NULL);

    // Initialize the canlib device
    device_init(&hcan_comm.rx_device);
    device_set_address(
        &hcan_comm.rx_device,
        &hcan_comm.rx_raw,
        bms_MAX_STRUCT_SIZE_RAW,
        &hcan_comm.rx_conv,
        bms_MAX_STRUCT_SIZE_CONVERSION
    );

    return CAN_COMM_OK;
}

void can_comm_enable_all(void) {
    CAN_COMM_ENABLE_ALL(hcan_comm.enabled);
}

void can_comm_disable_all(void) {
    CAN_COMM_DISABLE_ALL(hcan_comm.enabled);
}

bool can_comm_is_enabled_all(void) {
    return CAN_COMM_IS_ENABLED_ALL(hcan_comm.enabled);
}

void can_comm_enable(CanCommEnableBit bit) {
    if (bit >= CAN_COMM_ENABLE_BIT_COUNT)
        return;
    CAN_COMM_ENABLE(hcan_comm.enabled, bit);
}

void can_comm_disable(CanCommEnableBit bit) {
    if (bit >= CAN_COMM_ENABLE_BIT_COUNT)
        return;
    CAN_COMM_DISABLE(hcan_comm.enabled, bit);
}

bool can_comm_is_enabled(CanCommEnableBit bit) {
    if (bit >= CAN_COMM_ENABLE_BIT_COUNT)
        return false;
    return CAN_COMM_IS_ENABLED(hcan_comm.enabled, bit);
}

CanCommReturnCode can_comm_send_immediate(
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size)
{
    if (!CAN_COMM_IS_ENABLED(hcan_comm.enabled, CAN_COMM_TX_ENABLE_BIT))
        return CAN_COMM_DISABLED;

    // Check parameters validity
    if (index >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (frame_type >= CAN_FRAME_TYPE_COUNT)
        return CAN_COMM_INVALID_FRAME_TYPE;
    if (size > CAN_COMM_MAX_PAYLOAD_BYTE_SIZE)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;
    if (data == NULL && frame_type != CAN_FRAME_TYPE_REMOTE)
        return CAN_COMM_NULL_POINTER;


    // Prepare and push message to the buffer
    CanMessage msg = {
        .index = index,
        .frame_type = frame_type
    };
    if (frame_type != CAN_FRAME_TYPE_REMOTE)
        memcpy(msg.payload.tx, data, size);

    // If the buffer is full run the routine to free space for the new message
    if (ring_buffer_is_full(&hcan_comm.tx_buf))
        (void)can_comm_routine();

    // Add and send the new message
    if (ring_buffer_push_front(&hcan_comm.tx_buf, &msg) == RING_BUFFER_OK)
        return can_comm_routine();
    return CAN_COMM_OVERRUN;
}

CanCommReturnCode can_comm_tx_add(
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size)
{
    if (!CAN_COMM_IS_ENABLED(hcan_comm.enabled, CAN_COMM_TX_ENABLE_BIT))
        return CAN_COMM_DISABLED;

    // Check parameters validity
    if (index >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (frame_type >= CAN_FRAME_TYPE_COUNT)
        return CAN_COMM_INVALID_FRAME_TYPE;
    if (size > CAN_COMM_MAX_PAYLOAD_BYTE_SIZE)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;
    if (data == NULL && frame_type != CAN_FRAME_TYPE_REMOTE)
        return CAN_COMM_NULL_POINTER;


    // Prepare and push message to the buffer
    CanMessage msg = {
        .index = index,
        .frame_type = frame_type
    };
    if (frame_type != CAN_FRAME_TYPE_REMOTE)
        memcpy(msg.payload.tx, data, size);

    if (ring_buffer_push_back(&hcan_comm.tx_buf, &msg) == RING_BUFFER_FULL)
        return CAN_COMM_OVERRUN;
    return CAN_COMM_OK;
}

CanCommReturnCode can_comm_rx_add(
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size)
{    
    if (!CAN_COMM_IS_ENABLED(hcan_comm.enabled, CAN_COMM_RX_ENABLE_BIT))
        return CAN_COMM_DISABLED;

    // Check parameters validity
    if (index >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (data == NULL && frame_type != CAN_FRAME_TYPE_REMOTE)
        return CAN_COMM_NULL_POINTER;
    if (size > CAN_COMM_MAX_PAYLOAD_BYTE_SIZE)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;
    if (frame_type >= CAN_FRAME_TYPE_COUNT)
        return CAN_COMM_INVALID_FRAME_TYPE;

    // Prepare and push message to the buffer
    CanMessage msg = {
        .index = index,
        .frame_type = frame_type
    };
    if (frame_type != CAN_FRAME_TYPE_REMOTE)
        memcpy(msg.payload.rx, data, size);

    if (ring_buffer_push_back(&hcan_comm.rx_buf, &msg) == RING_BUFFER_FULL)
        return CAN_COMM_OVERRUN;
    return CAN_COMM_OK;
}

// TODO: Set error if CAN communication is not working
CanCommReturnCode can_comm_routine(void) {
    if (!CAN_COMM_IS_ENABLED_ALL(hcan_comm.enabled))
        return CAN_COMM_DISABLED;

    // Handler transmit and receive data
    CanCommReturnCode ret = CAN_COMM_OK;
    CanMessage tx_msg, rx_msg;
    if (CAN_COMM_IS_ENABLED(hcan_comm.enabled, CAN_COMM_TX_ENABLE_BIT) &&
        ring_buffer_pop_front(&hcan_comm.tx_buf, &tx_msg) == RING_BUFFER_OK)
    {
        uint8_t data[CAN_COMM_MAX_PAYLOAD_BYTE_SIZE];
        int size = 0;
        const can_id_t can_id = bms_id_from_index(tx_msg.index);

        if (tx_msg.frame_type != CAN_FRAME_TYPE_REMOTE) {
            // Serialize message
            size = bms_serialize_from_id(tx_msg.payload.tx, can_id, data);
            if (size < 0)
                return CAN_COMM_CONVERSION_ERROR;
        }

        // Send message
        ret = hcan_comm.send(
            can_id,
            tx_msg.frame_type,
            data,
            size
        );
    }
    if (CAN_COMM_IS_ENABLED(hcan_comm.enabled, CAN_COMM_RX_ENABLE_BIT) &&
        ring_buffer_pop_front(&hcan_comm.rx_buf, &rx_msg) == RING_BUFFER_OK)
    {
        const can_id_t can_id = bms_id_from_index(rx_msg.index);

        // TODO: Reset canlib watchdog
        // (void)watchdog_reset(rx_msg.index, timebase_get_time());

        if (rx_msg.frame_type != CAN_FRAME_TYPE_REMOTE) {
            // Deserialize message
            bms_devices_deserialize_from_id(&hcan_comm.rx_device, can_id, rx_msg.payload.rx);

            // TODO: Handle errors?
            _can_comm_payload_handle(rx_msg.index)(hcan_comm.rx_device.message);
        }
        else {
            // TODO: Handler remote requests
        }
    }

    return ret;
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
    [CAN_COMM_INVALID_FRAME_TYPE] = "invalid frame type",
    [CAN_COMM_CONVERSION_ERROR] = "conversion error",
    [CAN_COMM_TRANSMISSION_ERROR] = "transmission error"
};

static char * can_comm_return_code_description[] = {
    [CAN_COMM_OK] = "executed succesfully",
    [CAN_COMM_NULL_POINTER] = "attempt to dereference a null pointer"
    [CAN_COMM_DISABLED] = "the can manager is not enabled"
    [CAN_COMM_OVERRUN] = "the transmission buffer is full"
    [CAN_COMM_INVALID_INDEX] = "the given index does not correspond to any valid message",
    [CAN_COMM_INVALID_PAYLOAD_SIZE] = "the payload size is greater than the maximum allowed length"
    [CAN_COMM_INVALID_FRAME_TYPE] = "the given frame type does not correspond to any existing can frame type",
    [CAN_COMM_CONVERSION_ERROR] = "can't convert the message correctly",
    [CAN_COMM_TRANSMISSION_ERROR] = "error during message transmission"
};

#endif // CONF_CAN_COMM_STRINGS_ENABLE

#endif // CONF_CAN_COMM_MODULE_ENABLE
