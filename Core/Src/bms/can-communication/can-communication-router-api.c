/*!
 * \file can-communication-router-api.c
 * \date 2024-06-21
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Implementation of the CAN-bus communication module.
 */

#include "can-communication-router-api.h"

#include "bal-api.h"
#include "cellboard-conf.h"
#include "can-communication.h"
#include "can-bms-api.h"
#include "can-bms.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

enum CanCommunicationReturnCode can_communication_router_api_receive_bms(struct CanCommunicationFrame *frame) {
    if (frame == nullptr) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    if (!can_bms_api_id_is_valid(frame->id)) {
        return CAN_COMMUNICATION_RC_INVALID_NETWORK;
    }

    union CanBmsMessages message = { 0 };
    if (can_bms_api_deserialize_from_id(frame->id, frame->data, &message) != 0) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    switch (frame->id) {
        case CAN_BMS_MESSAGE_FRAME_ID_TSACMAINBOARDBALANCINGSET:
            bal_api_balancing_set_handle(
                message.tsacmainboardbalancingset.start,
                message.tsacmainboardbalancingset.target,
                message.tsacmainboardbalancingset.threshold);
            break;
        default:
            break;
    }

    return CAN_COMMUNICATION_RC_OK;
}

#endif // CONF_CAN_COMM_MODULE_ENABLE
