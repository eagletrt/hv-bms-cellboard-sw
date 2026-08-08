/*!
 * \file can-communication.h
 * \date 2026-05-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \ Data types, enums and structures for the CAN communication module.
 */

#ifndef CAN_COMMUNICATION_H
#define CAN_COMMUNICATION_H

#include <stdint.h>

#include "pal.h"
#include "arena-allocator.h"

/*!
 * \brief Maximum payload size of a classic-CAN frame, in bytes.
 *
 * \details TODO: Should be defined by the libcan
 */
#define CAN_COMMUNICATION_FRAME_DATA_SIZE (8U)

/*!
 * \brief Size of the per-network reception queue.
 */
#define CAN_COMMUNICATION_RX_QUEUE_CAPACITY (128U)

/*!
 * \brief Size of the per-network transmission queue.
 */
#define CAN_COMMUNICATION_TX_QUEUE_CAPACITY (128U)

/*!
 * \brief System return and status codes for CAN communication operations.
 */
enum CanCommunicationReturnCode : uint8_t {
    CAN_COMMUNICATION_RC_OK,                    /*!< Operation completed successfully. */
    CAN_COMMUNICATION_RC_NULL_POINTER,          /*!< A required pointer argument was NULL. */
    CAN_COMMUNICATION_RC_INVALID_NETWORK,       /*!< Network ID is out of range or already initialized. */
    CAN_COMMUNICATION_RC_INVALID_LENGTH,        /*!< Frame length is larger than CAN_COMMUNICATION_FRAME_DATA_SIZE. */
    CAN_COMMUNICATION_RC_QUEUE_FULL,            /*!< Target queue has no space left. */
    CAN_COMMUNICATION_RC_QUEUE_EMPTY,           /*!< Target queue is empty. */
    CAN_COMMUNICATION_RC_TRANSMISSION_ERROR,    /*!< The user-supplied send callback reported failure. */
    CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR, /*!< The user-supplied receive callback reported failure. */
    CAN_COMMUNICATION_RC_ERROR,                 /*!< Generic failure (PAL/arena/internal). */
};

/*!
 * \brief Identifiers for the physical CAN networks managed by the ECU.
 */
enum CanCommunicationNetwork : uint8_t {
    CAN_COMMUNICATION_NETWORK_BMS,  /*!< Target the Primary CAN bus. */
    CAN_COMMUNICATION_NETWORK_COUNT /*!< Sentinel value used exclusively to verify network parameter validity. */
};

/*!
 * \brief Hardware-agnostic container representing a single raw CAN frame payload.
 */
struct [[gnu::packed]] CanCommunicationFrame {
    uint32_t id;                                     /*!< CAN identifier (Standard 11-bit or Extended 29-bit) */
    uint8_t length;                                  /*!< Count of valid data bytes within the array (0..8) */
    uint8_t data[CAN_COMMUNICATION_FRAME_DATA_SIZE]; /*!< Raw payload byte buffer */
};

/*!
 * \brief Callback invoked when a frame need to be sent through real hardware.
 * \param[in] frame The frame to transmit.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_TRANSMISSION_ERROR on a hardware-side failure.
 */
typedef enum CanCommunicationReturnCode (*can_communication_send_callback)(const struct CanCommunicationFrame *frame);

/*!
 * \brief Callback invoked once per frame removed by can_communication_api_process_rx.
 * \details This function is used to allocate the logic based on the deserialization using
 * canlib.
 *
 * \param[in] frame The frame removed from the RX queue.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR if dispatch fails.
 */
typedef enum CanCommunicationReturnCode (*can_communication_receive_callback)(struct CanCommunicationFrame *frame);

/*!
 * \brief Critical-section enter/exit callback.
 * \details Such callback can be passed at init time of even \c NULL if not required.
 */
typedef void (*can_communication_critical_section_callback)(void);

/*!
 * \brief Configuration parameters required to open a single bus channel.
 */
struct CanCommunicationNetworkConfig {
    can_communication_send_callback send;                 /*!< Peripheral driver frame transmitter function hook */
    can_communication_receive_callback on_receive;        /*!< Deserialization routing engine function hook */
    can_communication_critical_section_callback cs_enter; /*!< Optional: Thread-safety block lock initializer (May be \c NULL) */
    can_communication_critical_section_callback cs_exit;  /*!< Optional: Thread-safety block lock release initializer (May be \c NULL) */
};

/*!
 * \brief Explicit tracking layout block dedicated to a singular bus network slot.
 */
struct CanCommunicationNetworkState {
    struct PalHandler pal;                         /*!< Local PAL tracking instance allocated for this network's queues */
    can_communication_send_callback send;          /*!< Buffered reference pointing to driver transmitter */
    can_communication_receive_callback on_receive; /*!< Buffered reference pointing to deserializer */
};

/*!
 * \brief CAN manager handler structure
 *
 * \details One handler covers every network. The arena allocator is shared across
 *          all PAL instances since PAL only allocates at init time.
 */
struct CanCommunicationHandler {
    struct ArenaAllocatorHandler arena;                                            /*!< Internal memory arena shared across network handlers */
    struct CanCommunicationNetworkState networks[CAN_COMMUNICATION_NETWORK_COUNT]; /*!< State structures managing individual physical networks */
};

#endif // CAN_COMMUNICATION_H
