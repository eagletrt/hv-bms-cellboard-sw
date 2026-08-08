/*!
 * \file can-communication-api.c
 * \date 2024-04-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions to handle CAN communication with other devices
 */

#include "can-communication-api.h"
#include "can-communication.h"

#include <string.h>

#include "cellboard-conf.h"
#include "eagletrt.h"
#include "eagletrt-api.h"
#include "pal-api.h"
#include "arena-allocator-api.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

EAGLETRT_STATIC struct CanCommunicationHandler handler;

/*!
 * \brief Shared body of the per-network PAL sending message.
 *
 * \param[in] network Network ID of network to which send the message.
 * \param[in] message PAL message whose payload is a raw CanCommunicationFrame.
 *
 * \retval PAL_RC_OK on success.
 * \retval PAL_RC_NULL_POINTER if \p message or the user send callback is \c NULL.
 * \retval PAL_RC_INVALID_ARGUMENT if the message size does not match a frame.
 * \retval PAL_RC_IO_ERROR if the user send callback reported failure.
 */
EAGLETRT_STATIC enum PalReturnCode prv_pal_send_dispatch(enum CanCommunicationNetwork network, const struct PalMessage *message) {
    if (message == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (message->size != sizeof(struct CanCommunicationFrame)) {
        return PAL_RC_INVALID_ARGUMENT;
    }

    struct CanCommunicationFrame frame;
    EAGLETRT_API_UNUSED(memcpy(&frame, message->payload, sizeof(frame)));

    const can_communication_send_callback user_send = handler.networks[network].send;
    if (user_send == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    if (user_send(&frame) != CAN_COMMUNICATION_RC_OK) {
        return PAL_RC_IO_ERROR;
    }
    return PAL_RC_OK;
}

/*!
 * \brief PAL send message for the BMS network.
 */
EAGLETRT_STATIC enum PalReturnCode prv_pal_send_bms(const struct PalMessage *message) {
    return prv_pal_send_dispatch(CAN_COMMUNICATION_NETWORK_BMS, message);
}

/*!
 * \brief Initialize one network slot in the module's handler.
 *
 * \param[in] network  Network ID to initialise.
 * \param[in] config   User-supplied callbacks for this network.
 * \param[in] pal_send PAL sending function to this network.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if any required callback is \c NULL.
 * \retval CAN_COMMUNICATION_RC_INVALID_NETWORK if \p network is out of range.
 * \retval CAN_COMMUNICATION_RC_ERROR if PAL fails to initialise.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_can_communication_api_init_network(enum CanCommunicationNetwork network, const struct CanCommunicationNetworkConfig config, pal_send_callback pal_send) {
    if (config.send == NULL || config.on_receive == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }
    if (network >= CAN_COMMUNICATION_NETWORK_COUNT) {
        return CAN_COMMUNICATION_RC_INVALID_NETWORK;
    }
    memset(&handler.networks[network], 0, sizeof(handler.networks[network]));

    handler.networks[network].send = config.send;
    handler.networks[network].on_receive = config.on_receive;

    if (pal_api_init(
            &handler.networks[network].pal,
            CAN_COMMUNICATION_RX_QUEUE_CAPACITY,
            CAN_COMMUNICATION_TX_QUEUE_CAPACITY,
            (uint32_t)sizeof(struct CanCommunicationFrame),
            NULL,
            pal_send,
            config.cs_enter,
            config.cs_exit,
            &handler.arena) != PAL_RC_OK) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    return CAN_COMMUNICATION_RC_OK;
}

/*!
 * \brief Shared body of can_communication_api_add_to_tx and can_communication_api_add_to_rx.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_enqueue(enum CanCommunicationNetwork network, const struct CanCommunicationFrame *frame, bool to_tx) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }
    if (network >= CAN_COMMUNICATION_NETWORK_COUNT) {
        return CAN_COMMUNICATION_RC_INVALID_NETWORK;
    }
    if (frame->length > CAN_COMMUNICATION_FRAME_DATA_SIZE) {
        return CAN_COMMUNICATION_RC_INVALID_LENGTH;
    }

    struct CanCommunicationFrame buffer = *frame;
    const enum PalReturnCode return_code = to_tx
                                               ? pal_api_add_to_tx_queue(&handler.networks[network].pal, &buffer, (uint32_t)sizeof(buffer))
                                               : pal_api_add_to_rx_queue(&handler.networks[network].pal, (uint8_t *)&buffer, (uint32_t)sizeof(buffer));

    switch (return_code) {
        case PAL_RC_OK:
            return CAN_COMMUNICATION_RC_OK;
        case PAL_RC_QUEUE_FULL:
            return CAN_COMMUNICATION_RC_QUEUE_FULL;
        case PAL_RC_NULL_POINTER:
            return CAN_COMMUNICATION_RC_NULL_POINTER;
        default:
            return CAN_COMMUNICATION_RC_ERROR;
    }
}

enum CanCommunicationReturnCode can_communication_api_init(const struct CanCommunicationNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT]) {
    if (configs == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    memset(&handler, 0, sizeof(handler));
    arena_allocator_api_init(&handler.arena);

    const pal_send_callback pal_send_callbacks[CAN_COMMUNICATION_NETWORK_COUNT] = {
        [CAN_COMMUNICATION_NETWORK_BMS] = prv_pal_send_bms
    };

    for (enum CanCommunicationNetwork network = 0; network < CAN_COMMUNICATION_NETWORK_COUNT; ++network) {
        const enum CanCommunicationReturnCode return_code = prv_can_communication_api_init_network(network, configs[network], pal_send_callbacks[network]);
        if (return_code != CAN_COMMUNICATION_RC_OK) {
            return return_code;
        }
    }

    return CAN_COMMUNICATION_RC_OK;
}

enum CanCommunicationReturnCode can_communication_api_add_to_tx(enum CanCommunicationNetwork network, const struct CanCommunicationFrame *frame) {
    return prv_enqueue(network, frame, true);
}

enum CanCommunicationReturnCode can_communication_api_add_to_rx(enum CanCommunicationNetwork network, const struct CanCommunicationFrame *frame) {
    return prv_enqueue(network, frame, false);
}

enum CanCommunicationReturnCode can_communication_api_process_tx(enum CanCommunicationNetwork network) {
    if (network >= CAN_COMMUNICATION_NETWORK_COUNT) {
        return CAN_COMMUNICATION_RC_INVALID_NETWORK;
    }

    enum CanCommunicationReturnCode result = CAN_COMMUNICATION_RC_OK;
    enum PalReturnCode return_code;

    do {
        return_code = pal_api_process_tx(&handler.networks[network].pal);

        if (return_code == PAL_RC_IO_ERROR) {
            result = CAN_COMMUNICATION_RC_TRANSMISSION_ERROR;
        }
    } while (return_code == PAL_RC_OK || return_code == PAL_RC_IO_ERROR);

    if (return_code != PAL_RC_OK && return_code != PAL_RC_QUEUE_EMPTY) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    return result;
}

enum CanCommunicationReturnCode can_communication_api_process_rx(enum CanCommunicationNetwork network) {
    if (network >= CAN_COMMUNICATION_NETWORK_COUNT) {
        return CAN_COMMUNICATION_RC_INVALID_NETWORK;
    }

    const can_communication_receive_callback dispatch = handler.networks[network].on_receive;
    enum CanCommunicationReturnCode result = CAN_COMMUNICATION_RC_OK;
    enum PalReturnCode return_code;

    do {
        struct CanCommunicationFrame frame;
        return_code = pal_api_process_rx(&handler.networks[network].pal, &frame);

        if (return_code == PAL_RC_OK) {
            if (dispatch(&frame) != CAN_COMMUNICATION_RC_OK) {
                result = CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR;
            }
        }
    } while (return_code == PAL_RC_OK);

    if (return_code != PAL_RC_OK && return_code != PAL_RC_QUEUE_EMPTY) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    return result;
}

#ifdef CONF_CAN_COMM_STRINGS_ENABLE

EAGLETRT_STATIC char *can_communication_module_name = "can communication";

EAGLETRT_STATIC char *can_communication_return_code_name[] = {
    [CAN_COMMUNICATION_RC_OK] = "ok",
    [CAN_COMMUNICATION_RC_NULL_POINTER] = "null pointer",
    [CAN_COMMUNICATION_RC_INVALID_NETWORK] = "invalid network",
    [CAN_COMMUNICATION_RC_INVALID_LENGTH] = "invalid length",
    [CAN_COMMUNICATION_RC_QUEUE_FULL] = "queue full",
    [CAN_COMMUNICATION_RC_QUEUE_EMPTY] = "queue empty",
    [CAN_COMMUNICATION_RC_TRANSMISSION_ERROR] = "transmission error",
    [CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR] = "receive handler error"
};

EAGLETRT_STATIC char *can_communication_return_code_description[] = {
    [CAN_COMMUNICATION_RC_OK] = "executed succesfully",
    [CAN_COMMUNICATION_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [CAN_COMMUNICATION_RC_INVALID_NETWORK] = "the chosen network does not exists",
    [CAN_COMMUNICATION_RC_INVALID_LENGTH] = "invalid payload length",
    [CAN_COMMUNICATION_RC_QUEUE_FULL] = "transmission queue is full",
    [CAN_COMMUNICATION_RC_QUEUE_EMPTY] = "transmission queue is empty",
    [CAN_COMMUNICATION_RC_TRANSMISSION_ERROR] = "error during data transmission",
    [CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR] = "message cannot be handled by the application"
};

#endif // CONF_CAN_COMM_STRINGS_ENABLE

#endif // CONF_CAN_COMM_MODULE_ENABLE
