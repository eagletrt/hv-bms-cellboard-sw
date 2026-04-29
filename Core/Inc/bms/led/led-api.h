/*!
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
#include "eagletrt-api.h"

#ifdef CONF_LED_MODULE_ENABLE

/*!
 * \brief Initialize the internal LED handler
 *
 * \details The LED handler is disabled by default after initialization
 *
 * \param set The callback used to set the status of the LED
 * \param toggle The callback used to toggle the status of the LED
 *
 * \retval LED_RC_NULL_POINTER if the set or toggle callbacks are NULL
 * \retval LED_RC_OK otherwise
 */
enum LedReturnCode led_init(const led_set_state_callback set, const led_toggle_state_callback toggle);

/*!
 * \brief Enable or disable the LED handler
 *
 * \param enabled True to enable the handler, false otherwise
 */
void led_set_enable(const bool enabled);

/*!
 * \brief Routine used to set the LED state
 *
 * \param t The current time in ms
 *
 * \retval LED_RC_NULL_POINTER if the set or toggle callbacks are NULL
 * \retval LED_RC_OK otherwise
 */
enum LedReturnCode led_routine(const milliseconds_t t);

#else // CONF_LED_MODULE_ENABLE

#define led_init(set, toggle) (LED_RC_OK)
#define led_set_enable(enabled) EAGLETRT_API_NOP()
#define led_routine(t) (LED_RC_OK)

#endif // CONF_LED_MODULE_ENABLE

#endif // LED_API_H
