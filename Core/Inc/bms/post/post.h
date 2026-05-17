/*!
 * \file post.h
 * \date 2024-04-16
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#ifndef POST_H
#define POST_H

#include "cellboard-def.h"

#include "can-comm-api.h"
#include "bms-manager.h"
#include "led.h"
#include "temp.h"

/*!
 * \brief Return code for the post module functions
 *
 */
enum PostReturnCode {
    POST_RC_OK,                   /*!< Function executed successfully */
    POST_RC_UNINITIALIZED,        /*!< A module has not been initialized correctly */
    POST_RC_INVALID_CELLBOARD_ID, /*!< Invalid cellboard identifier */
    POST_RC_NULL_POINTER,         /*!< A NULL pointer was given to a function */
};

/*!
 * \brief Structure definition for the initial data that are needed by the POST module
 */
struct PostInitData {
    enum CellboardId id;                                  /*!< The current cellboard index */
    system_reset_callback_t system_reset;                 /*!< A pointer to a function that resets the microcontroller */
    interrupt_critical_section_enter_t cs_enter;          /*!< A pointer to a function that enters an interrupt critical section */
    interrupt_critical_section_exit_t cs_exit;            /*!< A pointer to a function that exits an interrupt critical section */
    can_comm_transmit_callback_t can_send;                /*!< A pointer to a function that can send data via the CAN bus */
    bms_manager_send_callback_t spi_send;                 /*!< A pointer to a function that can send data via the SPI peripheral */
    bms_manager_send_receive_callback_t spi_send_receive; /*!< A pointer to a function that can send and receive data via the SPI peripheral */
    led_set_state_callback led_set;                       /*!< A pointer to a function that sets the state of a LED */
    led_toggle_state_callback led_toggle;                 /*!< A pointer to a function that toggles the state of a LED */
    temp_set_mux_address_callback gpio_set_address;       /*!< A pointer to a function that sets the address of the mux used for the temperature measurement */
    temp_start_conversion_callback adc_start;             /*!< A pointer to a function that starts the ADC conversion for the temperature measurement */
};

#endif // POST_H
