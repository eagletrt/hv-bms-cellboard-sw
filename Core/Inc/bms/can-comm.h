/*!
 * \file can-comm.h
 * \date 2026-05-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions to handle CAN communication with other devices
 */

#ifndef CAN_COMM_H
#define CAN_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"
#include "ring-buffer-api.h"
#include "eagletrt-api.h"

/*! \brief Maximum number of bytes of the payload in a CAN message */
#define CAN_COMM_MAX_PAYLOAD_BYTE_SIZE (8U)

/*! \brief Mask used to check the bits of a CAN identifier */
#define CAN_COMM_ID_MASK (0x7FFU)
#define CAN_COMM_EXT_ID_MASK (0x1FFFFFFFU)

/*! \brief Maximum number of CAN messages that can be saved inside the transmission and reception buffers */
#define CAN_COMM_MESSAGE_COUNT (bms_MESSAGE_COUNT)
#define CAN_COMM_TX_BUFFER_BYTE_SIZE (CAN_COMM_MESSAGE_COUNT)
#define CAN_COMM_RX_BUFFER_BYTE_SIZE (CAN_COMM_MESSAGE_COUNT)

/*! \brief Mask for the bits that defines if the CAN module is enabled or not */
#define CAN_COMM_ENABLED_ALL_MASK        \
    (                                    \
        (1U << CAN_COMM_RX_ENABLE_BIT) | \
        (1U << CAN_COMM_TX_ENABLE_BIT))

/*!
 * \brief Enable a single bit of the internal flag
 *
 * \param FLAG The internal flag
 * \param BIT The bit of the flag to set
 */
#define CAN_COMM_ENABLE(FLAG, BIT) ((FLAG) = EAGLETRT_API_BIT_SET(FLAG, BIT))
/*!
 * \brief Disable a single bit of the internal flag
 *
 * \param FLAG The internal flag
 * \param BIT The bit of the flag to reset
 */
#define CAN_COMM_DISABLE(FLAG, BIT) ((FLAG) = EAGLETRT_API_BIT_RESET(FLAG, BIT))
/*!
 * \brief Toggle a single bit of the internal flag
 *
 * \param FLAG The internal flag
 * \param BIT The bit of the flag to flip
 */
#define CAN_COMM_TOGGLE(FLAG, BIT) ((FLAG) = EAGLETRT_API_BIT_TOGGLE(FLAG, BIT))
/*!
 * \brief Check if a specific bit of the internal flag is set
 *
 * \param FLAG The internal flag
 * \param BIT The bit of the flag to check
 *
 * \return bool True if the bit is set, false otherwise
 */
#define CAN_COMM_IS_ENABLED(FLAG, BIT) EAGLETRT_API_BIT_GET(FLAG, BIT)

/*!
 * \brief Enable all the bits of the internal flag
 *
 * \param FLAG The internal flag
 */
#define CAN_COMM_ENABLE_ALL(FLAG) ((FLAG) |= CAN_COMM_ENABLED_ALL_MASK)
/*!
 * \brief Disable all the bits of the internal flag
 *
 * \param FLAG The internal flag
 */
#define CAN_COMM_DISABLE_ALL(FLAG) ((FLAG) &= ~CAN_COMM_ENABLED_ALL_MASK)
/*!
 * \brief Toggle all the bits of the internal flag
 *
 * \param FLAG The internal flag
 */
#define CAN_COMM_TOGGLE_ALL(FLAG) ((FLAG) ^= CAN_COMM_ENABLED_ALL_MASK)
/*!
 * \brief Check if all the bits of the internal flag are set
 *
 * \param FLAG The internal flag
 *
 * \return bool True if all the bits are set, false otherwise
 */
#define CAN_COMM_IS_ENABLED_ALL(FLAG) (((FLAG) & CAN_COMM_ENABLED_ALL_MASK) == CAN_COMM_ENABLED_ALL_MASK)

/*!
 * \brief Return code for the CAN communication module functions
 */
enum CanCommReturnCode {
    CAN_COMM_RC_OK,                   /*!< Function executed successfully */
    CAN_COMM_RC_NULL_POINTER,         /*!< A NULL pointer was given to a function */
    CAN_COMM_RC_DISABLED,             /*!< The CAN manager is not running */
    CAN_COMM_RC_OVERRUN,              /*!< The transmit buffer is full */
    CAN_COMM_RC_INVALID_INDEX,        /*!< The given index does not correspond to any CAN message */
    CAN_COMM_RC_INVALID_PAYLOAD_SIZE, /*!< The payload size exceed the maximum possible length */
    CAN_COMM_RC_INVALID_FRAME_TYPE,   /*!< The frame type does not correspond to any existing CAN frame type */
    CAN_COMM_RC_CONVERSION_ERROR,     /*!< The message could not be converted correctly */
    CAN_COMM_RC_TRANSMISSION_ERROR    /*!< There was an error during the transmission of the message */
};

/*!
 * \brief Enable bit flag positions
 */
enum CanCommEnableBit {
    CAN_COMM_RX_ENABLE_BIT = 0U, /*!< Reception enable bit position */
    CAN_COMM_TX_ENABLE_BIT,      /*!< Transmission enable bit position */
    CAN_COMM_ENABLE_BIT_COUNT    /*!< Number of enable bits */
};

/*!
 * \brief Union used to choose the CAN payload based on transmission or reception
 *
 * \details This is needed because during transmission the size of the payload is unknown
 * otherwise the received payload cannot exceed the maximum possible size
 */
union CanPayload {
    uint8_t tx[bms_MAX_STRUCT_SIZE_CONVERSION]; /*!< Transmission payload */
    uint8_t rx[bms_MAX_STRUCT_SIZE_RAW];        /*!< Reception payload */
};

/*!
 * \brief Structure definition for the content of a CAN bus message
 */
struct CanMessage {
    can_index_t index;        /*!< Index mapped to the CAN identifier */
    CanFrameType frame_type;  /*!< The frame type */
    union CanPayload payload; /*!< The actual content of the message */
};

/*!
 * \brief Function used to send CAN message via a network
 *
 * \param id The CAN identifier
 * \param frame_type The CAN frame type
 * \param data The actual payload of the message
 * \param size The size of the payload
 *
 * \returns enum CanCommReturnCode The return code value
 */
typedef enum CanCommReturnCode (*can_comm_transmit_callback_t)(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    const can_id_t id,
    const CanFrameType frame_type,
    const uint8_t *const data,
    const size_t size);

/*!
 * \brief Handle the received CAN payload data
 *
 * \details The payload parameter should be converted to the correct structure pointer
 *
 * \param payload A pointer to the converted canlib structure data
 */
typedef int32_t (*can_comm_canlib_payload_handle_callback_t)(const void *const payload);

/*!
 * \brief CAN manager handler structure
 *
 * \details The enabled bit flag
 */
struct CanCommHandler {
    bit_flag8_t enabled;                  /*!< Flag used to enable or disable the CAN communication */
    bool tx_busy[CAN_COMM_MESSAGE_COUNT]; /*!< Transmission messages flags to check if the message has not already been sent */
    bool rx_busy[CAN_COMM_MESSAGE_COUNT]; /*!< Reception messages flags to check if the message has not already been handled */

    struct ArenaAllocatorHandler tx_arena; /*!< Transmission messages arena allocator */
    struct ArenaAllocatorHandler rx_arena; /*!< Reception messages arena allocator */

    struct RingBufferHandler tx_buf; /*!< Transmission messages circular buffer */
    struct RingBufferHandler rx_buf; /*!< Reception messages circular buffer */

    can_comm_transmit_callback_t send; /*!< A pointer to the callback used to send the data via CAN */

    // Canlib devices
    device_t rx_device;                              /*!< The reception canlib message handler */
    uint8_t rx_raw[bms_MAX_STRUCT_SIZE_RAW];         /*!< The reception raw data of the message */
    uint8_t rx_conv[bms_MAX_STRUCT_SIZE_CONVERSION]; /*!< The reception converted data of the message */
};

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

#endif // CAN_COMM_H
