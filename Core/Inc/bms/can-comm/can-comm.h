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
 * \param can_id The CAN identifier
 * \param frame_type The CAN frame type
 * \param data The actual payload of the message
 * \param size The size of the payload
 *
 * \returns enum CanCommReturnCode The return code value
 */
typedef enum CanCommReturnCode (*can_comm_transmit_callback)(
    // CanNetwork network, // Not needed because the cellboards have only the BMS network
    const can_id_t can_id,
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
typedef int32_t (*can_comm_canlib_payload_handle_callback)(const void *const payload);

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

    can_comm_transmit_callback send; /*!< A pointer to the callback used to send the data via CAN */

    // Canlib devices
    device_t rx_device;                              /*!< The reception canlib message handler */
    uint8_t rx_raw[bms_MAX_STRUCT_SIZE_RAW];         /*!< The reception raw data of the message */
    uint8_t rx_conv[bms_MAX_STRUCT_SIZE_CONVERSION]; /*!< The reception converted data of the message */
};

#endif // CAN_COMM_H
