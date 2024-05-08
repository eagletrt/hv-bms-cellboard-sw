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


/**
 * @brief Return code for the CAN communication module functions
 *
 * @details
 *     CAN_COMM_OK the function executed succesfully
 *     CAN_COMM_NULL_POINTER a NULL pointer was given to a function
 *     CAN_COMM_DISABLED the CAN manager is not running
 *     CAN_COMM_OVERRUN the transmit buffer is full
 *     CAN_COMM_INVALID_INDEX the given index does not correspond to any CAN message
 *     CAN_COMM_INVALID_PAYLOAD_SIZE the payload size exceed the maximum possible length
 *     CAN_COMM_CONVERSION_ERROR the message could not be converted correctly
 *     CAN_COMM_TRANSMISSION_ERROR there was an error during the transmission of the message
 */
typedef enum {
    CAN_COMM_OK,
    CAN_COMM_NULL_POINTER,
    CAN_COMM_DISABLED,
    CAN_COMM_OVERRUN,
    CAN_COMM_INVALID_INDEX,
    CAN_COMM_INVALID_PAYLOAD_SIZE,
    CAN_COMM_CONVERSION_ERROR,
    CAN_COMM_TRANSMISSION_ERROR
} CanCommReturnCode;

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
    void * tx;
    uint8_t rx[CELLBOARD_CAN_MAX_PAYLOAD_BYTE_SIZE];
} CanPayload;

/**
 * @brief Structure definition for the content of a CAN bus message
 *
 * @param index Index mapped to the CAN identifier
 * @param payload A pointer to the actual content of the message
 */
typedef struct {
    // CanNetwork network; // Not needed because cellboards have only the BMS network
    can_index index;
    CanPayload payload;
} CanMessage;


/**
 * @brief Function used to send CAN message via a network
 *
 * @param id The CAN identifier 
 * @param data The actual payload of the message
 * @param size The size of the payload
 */
typedef void (* can_comm_transmit_callback)(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_id id,
    const uint8_t * data,
    size_t size
);

/**
 *
 */
typedef void (* can_comm_update_canlib_payload_callback)(void * payload);


#ifdef CONF_CAN_COMM_MODULE_ENABLE

/**
 * @brief Initialize the CAN communication handler structure
 *
 * @param send The callback of a function that should send the data via a CAN network
 *
 * @return CanCommReturnCode
 *     CAN_COMM_NULL_POINTER a null pointer was given as parameter
 *     CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_init(can_comm_transmit_callback send);

/**
 * @brief Enable or disable the CAN manager
 *
 * @pararm enabled True to enable the manager, false to disable
 */
void can_comm_set_enable(bool enabled);

/**
 * @brief Check if the CAN manager is enabled
 *
 * @pararm enabled True if the manager is enabled, false to disable
 */
void can_comm_is_enabled(bool enabled);

/**
 * @brief Add a message to the transmission buffer
 *
 * @details The message will be sent afterwards inside the routine
 *
 * @param index The CAN index mapped to its identifier
 * @param payload The payload of the message
 *
 * @return CanCommReturnCode
 *     CAN_COMM_DISABLED the CAN manager is disabled
 *     CAN_COMM_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 *     CAN_COMM_OVERRUN the transmission buffer is already full
 *     CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_tx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_index index,
    void * payload
);

/**
 * @brief Add a message to the reception buffer
 *
 * @details The message will be handled afterwards inside the routine
 *
 * @param index The CAN index mapped to its identifier
 * @param data The payload of the message
 * @param size The paylaod size in bytes
 *
 * @return CanCommReturnCode
 *     CAN_COMM_DISABLED the CAN manager is disabled
 *     CAN_COMM_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 *     CAN_COMM_OVERRUN the transmission buffer is already full
 *     CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_rx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    can_index index,
    uint8_t * data,
    size_t size
);

/**
 * @brief Routine used to manage the sent or received can data
 *
 * @return CanCommReturnCode
 *     CAN_COMM_DISABLED the CAN manager is not running
 *     CAN_COMM_CONVERSION_ERROR there was an error during the conversion of the message
 *     CAN_COMM_OK otherwise
 */
CanCommReturnCode can_comm_routine(void);

#else  // CONF_CAN_COMM_MODULE_ENABLE

#define can_comm_init(send) (CAN_COMM_OK)
#define can_comm_set_enable(enabled) CELLBOARD_NOPE()
#define can_comm_is_enabled() (false)
#define can_comm_tx_add(index, payload) (CAN_COMM_OK)
#define can_comm_rx_add(index, data) (CAN_COMM_OK)
#define can_comm_routine() (CAN_COMM_OK)

#endif // CONF_CAN_COMM_MODULE_ENABLE

#endif  // CAN_COMM_H
