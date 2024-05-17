/**
 * @file can-comm.h
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to handle CAN communication with other devices
 */

#ifndef CAN_COMM_H
#define CAN_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"
#include "bms_network.h"


/**
 * @brief Return code for the CAN communication module functions
 *
 * @details
 *     - CAN_COMM_OK the function executed succesfully
 *     - CAN_COMM_NULL_POINTER a NULL pointer was given to a function
 *     - CAN_COMM_DISABLED the CAN manager is not running
 *     - CAN_COMM_OVERRUN the transmit buffer is full
 *     - CAN_COMM_INVALID_INDEX the given index does not correspond to any CAN message
 *     - CAN_COMM_INVALID_PAYLOAD_SIZE the payload size exceed the maximum possible length
 *     - CAN_COMM_INVALID_FRAME_TYPE the frame type does not correspond to any existing CAN frame type
 *     - CAN_COMM_CONVERSION_ERROR the message could not be converted correctly
 *     - CAN_COMM_TRANSMISSION_ERROR there was an error during the transmission of the message
 */
typedef enum {
    CAN_COMM_OK,
    CAN_COMM_NULL_POINTER,
    CAN_COMM_DISABLED,
    CAN_COMM_OVERRUN,
    CAN_COMM_INVALID_INDEX,
    CAN_COMM_INVALID_PAYLOAD_SIZE,
    CAN_COMM_INVALID_FRAME_TYPE,
    CAN_COMM_CONVERSION_ERROR,
    CAN_COMM_TRANSMISSION_ERROR
} CanCommReturnCode;

/**
 * @brief Enable bit flag positions
 *
 * @details
 *     - CAN_COMM_RX_ENABLE_BIT reception enable bit
 *     - CAN_COMM_TX_ENABLE_BIT transmission enable bit
 */
typedef enum {
    CAN_COMM_RX_ENABLE_BIT = 0U,
    CAN_COMM_TX_ENABLE_BIT,
    CAN_COMM_ENABLE_BIT_COUNT
} CanCommEnableBit;

/**
 * @brief Union used to choose the CAN payload based on transmission or reception
 *
 * @details This is needed because during transmission the size of the payload is unknown
 * otherwise the received payload cannot exceed the maximum possible size
 *
 * @param tx A pointer to the CAN payload that needs to be transmitted
 * @param rx The received CAN payload
 */
typedef union {
    uint8_t tx[bms_MAX_STRUCT_SIZE_CONVERSION];
    uint8_t rx[bms_MAX_STRUCT_SIZE_RAW];
} CanPayload;

/**
 * @brief Structure definition for the content of a CAN bus message
 *
 * @param index Index mapped to the CAN identifier
 * @param frame_type The frame type
 * @param payload A pointer to the actual content of the message
 */
typedef struct {
    // CanNetwork network; // Not needed because cellboards have only the BMS network
    can_index_t index;
    CanFrameType frame_type;
    CanPayload payload;
} CanMessage;


/**
 * @brief Function used to send CAN message via a network
 *
 * @param id The CAN identifier 
 * @param frame_type The CAN frame type
 * @param data The actual payload of the message
 * @param size The size of the payload
 *
 * @return CanCommReturnCode The return code value
 */
typedef CanCommReturnCode (* can_comm_transmit_callback_t)(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_id_t id,
    CanFrameType frame_type,
    const uint8_t * data,
    size_t size
);

/**
 * @brief Handle the received CAN payload data
 *
 * @details The payload parameter should be converted to the correct structure pointer
 *
 * @param payload A pointer to the converted canlib structure data
 */
typedef void (* can_comm_canlib_payload_handle_callback)(void * payload);


#ifdef CONF_CAN_COMM_MODULE_ENABLE

/**
 * @brief Initialize the CAN communication handler structure
 *
 * @param send The callback of a function that should send the data via a CAN network
 *
 * @return CanCommReturnCode
 *     - CAN_COMM_NULL_POINTER a NULL pointer was given as parameter
 *     - CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_init(can_comm_transmit_callback_t send);

/** @brief Enable the CAN manager */
void can_comm_enable_all(void);

/** @brief Disable the CAN manager */
void can_comm_disable_all(void);

/**
 * @brief Check if the CAN manager is enabled
 *
 * @return bool True if the manager is enabled, false otherwise
 */
bool can_comm_is_enabled_all(void);

/**
 * @brief Enable a single bit of the internal handler flag
 *
 * @param bit The bit to enable
 */
void can_comm_enable(CanCommEnableBit bit);

/**
 * @brief Disable a single bit of the internal handler flag
 *
 * @param bit The bit to disable
 */
void can_comm_disable(CanCommEnableBit bit);

/**
 * @brief Check if a single bit of the internal handler flag is enabled
 *
 * @param bit The bit to check
 *
 * @return bool True if the manager is enabled, false otherwise
 */
bool can_comm_is_enabled(CanCommEnableBit bit);

/**
 * @brief Immediately send the message via the CAN bus
 *
 * @attention This function should be used carefully because it can run the
 * routine multiple times internally
 *
 * @param index The CAN index mapped to its identifier
 * @param frame_type The frame type
 * @param data The payload of the message
 * @param size The payload size in bytes
 *
 * @return CanCommReturnCode
 *     - CAN_COMM_DISABLED the CAN manager is disabled
 *     - CAN_COMM_INVALID_INDEX if the given index does not match any valid CAN identifier
 *     - CAN_COMM_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 *     - CAN_COMM_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 *     - CAN_COMM_OVERRUN the transmission buffer is already full
 *     - CAN_COMM_CONVERSION_ERROR there was an error during the conversion of the message
 *     - CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_send_immediate(
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size
);

/**
 * @brief Add a message to the transmission buffer
 *
 * @details The message will be sent afterwards inside the routine
 *
 * @param index The CAN index mapped to its identifier
 * @param frame_type The frame type
 * @param data The payload of the message
 * @param size The payload size in bytes
 *
 * @return CanCommReturnCode
 *     - CAN_COMM_DISABLED the CAN manager is disabled
 *     - CAN_COMM_INVALID_INDEX if the given index does not match any valid CAN identifier
 *     - CAN_COMM_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 *     - CAN_COMM_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 *     - CAN_COMM_OVERRUN the transmission buffer is already full
 *     - CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_tx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size
);

/**
 * @brief Add a message to the reception buffer
 *
 * @details The message will be handled afterwards inside the routine
 *
 * @param index The CAN index mapped to its identifier
 * @param frame_type The frame type
 * @param data The payload of the message (can be NULL for REMOTE frames)
 * @param size The paylaod size in bytes
 *
 * @return CanCommReturnCode
 *     - CAN_COMM_DISABLED the CAN manager is disabled
 *     - CAN_COMM_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 *     - CAN_COMM_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 *     - CAN_COMM_OVERRUN the transmission buffer is already full
 *     - CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_rx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_index_t index,
    CanFrameType frame_type,
    uint8_t * data,
    size_t size
);

/**
 * @brief Routine used to manage the sent or received can data
 *
 * @return CanCommReturnCode
 *     - CAN_COMM_DISABLED the CAN manager is not running
 *     - CAN_COMM_CONVERSION_ERROR there was an error during the conversion of the message
 *     - CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_routine(void);

#else  // CONF_CAN_COMM_MODULE_ENABLE

#define can_comm_init(send) (CAN_COMM_OK)
#define can_comm_enable_all() CELLBOARD_NOPE()
#define can_comm_disable_all() CELLBOARD_NOPE()
#define can_comm_is_enabled_all() (false)
#define can_comm_enable(bit) CELLBOARD_NOPE()
#define can_comm_disable(bit) CELLBOARD_NOPE()
#define can_comm_is_enabled(bit) (false)
#define can_comm_send_immidiate(index, frame_type, data, size) (CAN_COMM_OK)
#define can_comm_tx_add(index, frame_type, data, size) (CAN_COMM_OK)
#define can_comm_rx_add(index, frame_type, data, size) (CAN_COMM_OK)
#define can_comm_routine() (CAN_COMM_OK)

#endif // CONF_CAN_COMM_MODULE_ENABLE

#endif  // CAN_COMM_H
