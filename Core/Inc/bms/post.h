/**
 * @file post.h
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#ifndef POST_H
#define POST_H

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "can-comm.h"
#include "bms-monitor.h"
#include "led.h"

/**
 * @brief Return code for the post module functions
 *
 * @details
 *     POST_OK the function executed succesfully
 *     POST_INVALID_CELLBOARD_ID invalid cellboard identifier
 *     POST_NULL_POINTER a NULL pointer was given to a function
 *     POST_WATCHDOG_INVALID_MESSAGE the watchdog are using an invalid message
 */
typedef enum {
    POST_OK,
    POST_INVALID_CELLBOARD_ID,
    POST_NULL_POINTER,
    POST_WATCHDOG_INVALID_MESSAGE
} PostReturnCode;

/**
 * @brief Structure definition for the initial data that are needed by the POST module
 *
 * @param id The current cellboard index
 * @param send A pointer to a function callback that should send data via CAN
 */
typedef struct {
    CellboardId id;
    system_reset_callback system_reset;
    can_comm_transmit_callback can_send;
    bms_monitor_send_callback spi_send;
    bms_monitor_send_receive_callback spi_send_receive;
    led_set_state_callback led_set;
    led_toggle_state_callback led_toggle;
} PostInitData;

#ifdef CONF_POST_MODULE_ENABLE

/**
 * @brief Run the power-on self test
 *
 * @details This function check if all the systems and peripherals work
 * as execpected, otherwise it returns an error code
 *
 * @param data The data needed by the POST module for initialization
 *
 * @return PostReturnCode
 *     POST_INVALID_CELLBOARD_ID if the given cellboard id is not valid
 *     POST_NULL_POINTER if the given function pointers are NULL
 *     POST_WATCHDOG_INVALID_MESSAGE the watchdog are using an invalid message
 *     POST_OK otherwise
 */
PostReturnCode post_run(PostInitData data);

#else  // CONF_POST_MODULE_ENABLE

#define post_run(data) (POST_OK)

#endif // CONF_POST_MODULE_ENABLE

#endif // POST_H
