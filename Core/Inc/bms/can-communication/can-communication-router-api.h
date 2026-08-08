/*!
 * \file can-communication-router-api.h
 * \date 2024-06-21
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Generic API interface for routing abstracted CAN frames via PAL.
 */

#ifndef CAN_COMM_API_H
#define CAN_COMM_API_H

#include "can-communication.h"
#include "cellboard-conf.h"

#ifdef CONF_CAN_COMM_MODULE_ENABLE

/*!
 * \brief Function for incoming CAN frames on BMS network.
 * \param[in] frame The frame just removed from the RX queue.
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR if dispatch fails.
 */
enum CanCommunicationReturnCode can_communication_router_api_receive_bms(struct CanCommunicationFrame *frame);

#else // CONF_CAN_COMM_MODULE_ENABLE

#define can_communication_router_api_receive_bms(frame) (CAN_COMMUNICATION_RC_OK)

#endif // CONF_CAN_COMM_MODULE_ENABLE

#endif // CAN_COMM_API_H
