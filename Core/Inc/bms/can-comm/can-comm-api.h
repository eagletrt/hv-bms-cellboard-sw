/*!
 * \file can-comm-api.h
 * \date 2026-05-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions to handle CAN communication with other devices
 */

#ifndef CAN_COMM_API_H
#define CAN_COMM_API_H

#include "can-comm.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

/*!
 * \brief Initialize the CAN communication handler structure
 *
 * \param send The callback of a function that should send the data via a CAN network
 *
 * \retval CAN_COMM_RC_NULL_POINTER a NULL pointer was given as parameter
 * \retval CAN_COMM_RC_OK otherwise
 */
enum CanCommReturnCode can_comm_init(const can_comm_transmit_callback_t send);

/*! \brief Enable the CAN manager */
void can_comm_enable_all(void);

/*! \brief Disable the CAN manager */
void can_comm_disable_all(void);

/*!
 * \brief Check if the CAN manager is enabled
 *
 * \returns bool True if the manager is enabled, false otherwise
 */
bool can_comm_is_enabled_all(void);

/*!
 * \brief Enable a single bit of the internal handler flag
 *
 * \param bit The bit to enable
 */
void can_comm_enable(const enum CanCommEnableBit bit);

/*!
 * \brief Disable a single bit of the internal handler flag
 *
 * \param bit The bit to disable
 */
void can_comm_disable(const enum CanCommEnableBit bit);

/*!
 * \brief Check if a single bit of the internal handler flag is enabled
 *
 * \param bit The bit to check
 *
 * \returns bool True if the manager is enabled, false otherwise
 */
bool can_comm_is_enabled(const enum CanCommEnableBit bit);

/*!
 * \brief Immediately send the message via the CAN bus
 *
 * \attention This function should be used carefully because it can run the
 * routine multiple times internally
 *
 * \param index The CAN index mapped to its identifier
 * \param frame_type The frame type
 * \param data The payload of the message
 * \param size The payload size in bytes
 *
 * \retval CAN_COMM_RC_DISABLED the CAN manager is disabled
 * \retval CAN_COMM_RC_INVALID_INDEX if the given index does not match any valid CAN identifier
 * \retval CAN_COMM_RC_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 * \retval CAN_COMM_RC_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 * \retval CAN_COMM_RC_OVERRUN the transmission buffer is already full
 * \retval CAN_COMM_RC_CONVERSION_ERROR there was an error during the conversion of the message
 * \retval CAN_COMM_RC_OK otherwise
 */
enum CanCommReturnCode can_comm_send_immediate(
    const can_index_t index,
    const CanFrameType frame_type,
    const uint8_t *const data,
    const size_t size);

/*!
 * \brief Add a message to the transmission buffer
 *
 * \details The message will be sent afterwards inside the routine
 *
 * \param index The CAN index mapped to its identifier
 * \param frame_type The frame type
 * \param data The payload of the message
 * \param size The payload size in bytes
 *
 * \retval CAN_COMM_RC_DISABLED the CAN manager is disabled
 * \retval CAN_COMM_RC_INVALID_INDEX if the given index does not match any valid CAN identifier
 * \retval CAN_COMM_RC_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 * \retval CAN_COMM_RC_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 * \retval CAN_COMM_RC_OVERRUN the transmission buffer is already full
 * \retval CAN_COMM_RC_OK otherwise
 */
enum CanCommReturnCode can_comm_tx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    const can_index_t index,
    const CanFrameType frame_type,
    const uint8_t *const data,
    const size_t size);

/*!
 * \brief Add a message to the reception buffer
 *
 * \details The message will be handled afterwards inside the routine
 *
 * \param index The CAN index mapped to its identifier
 * \param frame_type The frame type
 * \param data The payload of the message (can be NULL for REMOTE frames)
 * \param size The paylaod size in bytes
 *
 * \retval CAN_COMM_RC_DISABLED the CAN manager is disabled
 * \retval CAN_COMM_RC_INVALID_PAYLOAD_SIZE the given payload size exceed the maximum possible length
 * \retval CAN_COMM_RC_INVALID_FRAME_TYPE the given frame type is not a valid CAN frame type
 * \retval CAN_COMM_RC_OVERRUN the transmission buffer is already full
 * \retval CAN_COMM_RC_OK otherwise
 */
enum CanCommReturnCode can_comm_rx_add(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    const can_index_t index,
    const CanFrameType frame_type,
    const uint8_t *const data,
    const size_t size);

/*!
 * \brief Routine used to manage the sent or received can data
 *
 * \retval CAN_COMM_RC_DISABLED the CAN manager is not running
 * \retval CAN_COMM_RC_CONVERSION_ERROR there was an error during the conversion of the message
 * \retval CAN_COMM_RC_OK otherwise
 */
enum CanCommReturnCode can_comm_routine(void);

#else // CONF_CAN_COMM_MODULE_ENABLE

#define can_comm_init(send) (CAN_COMM_RC_OK)
#define can_comm_enable_all() CELLBOARD_NOPE()
#define can_comm_disable_all() CELLBOARD_NOPE()
#define can_comm_is_enabled_all() (false)
#define can_comm_enable(bit) CELLBOARD_NOPE()
#define can_comm_disable(bit) CELLBOARD_NOPE()
#define can_comm_is_enabled(bit) (false)
#define can_comm_send_immediate(index, frame_type, data, size) (CAN_COMM_RC_OK)
#define can_comm_tx_add(index, frame_type, data, size) (CAN_COMM_RC_OK)
#define can_comm_rx_add(index, frame_type, data, size) (CAN_COMM_RC_OK)
#define can_comm_routine() (CAN_COMM_RC_OK)

#endif // CONF_CAN_COMM_MODULE_ENABLE

#endif // CAN_COMM_API_H
