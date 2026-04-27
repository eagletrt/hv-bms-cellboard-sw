/**
 * \file led-api.h
 * \date 2024-05-08
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions used to manage the on-board LED
 */

#ifndef LED_API_H
#define LED_API_H

#include "led.h"

#ifdef CONF_LED_MODULE_ENABLE

/**
 * \brief Initialize the internal LED handler
 *
 * \details The LED handler is disabled by default after initialization
 *
 * \param set The callback used to set the status of the LED
 * \param toggle The callback used to toggle the status of the LED
 *
 * \return enum LedReturnCode
 *     - LED_NULL_POINTER if the set or toggle callbacks are NULL
 *     - LED_OK otherwise
 */
enum LedReturnCode led_init(const led_set_state_callback_t set, const led_toggle_state_callback_t toggle);

/**
 * \brief Enable or disable the LED handler
 *
 * \param enabled True to enable the handler, false otherwise
 */
void led_set_enable(const bool enabled);

/**
 * \brief Routine used to set the LED state
 *
 * \param t The current time in ms
 *
 * \return enum LedReturnCode
 *     - LED_OK
 */
enum LedReturnCode led_routine(const milliseconds_t t);

#else // CONF_LED_MODULE_ENABLE

#define led_init(set, toggle) (LED_OK)
#define led_set_enable(enabled) CELLBOARD_NOPE()
#define led_routine(t) (LED_OK)

#endif // CONF_LED_MODULE_ENABLE

#endif // LED_API_H
