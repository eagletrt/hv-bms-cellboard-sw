/**
 * @file led.h
 * @date 2024-05-08
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used to manage the on-board LED
 */

#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

/**
 * @brief Callback used to set the state of the LED
 *
 * @param state The state to set the LED to
 */
typedef void (* led_set_state_callback)(LedStatus state);

/**
 * @brief Callback used to toggle the state of the LED
 */
typedef void (* led_toggle_state_callback)(void);

/**
 * @brief Return code for the LED module functions
 *
 * @details
 *     LED_OK the function executed succesfully
 *     LED_NULL_POINTER a NULL pointer was given to a function
 *     LED_INVALID_CELLBOARD_ID invalid cellboard identifier
 */
typedef enum {
    LED_OK,
    LED_NULL_POINTER,
    LED_INVALID_CELLBOARD_ID
} LedReturnCode;

#ifdef CONF_LED_MODULE_ENABLE 

/**
 * @brief Initialize the internal LED handler
 *
 * @details The LED handler is disabled by default after initialization
 *
 * @param set The callback used to set the status of the LED
 * @param toggle The callback used to toggle the status of the LED
 *
 * @return LedReturnCode
 *     LED_NULL_POINTER if the set or toggle callbacks are NULL
 *     LED_OK otherwise
 */
LedReturnCode led_init(led_set_state_callback set, led_toggle_state_callback toggle);

/**
 * @param Enable or disable the LED handler
 *
 * @param enabled True to enable the handler, false otherwise
 */
void led_set_enable(bool enabled);

/**
 * @brief Routine used to set the LED state
 *
 * @param t The current time in ms
 *
 * @return LedReturnCode
 *     LED_OK
 */
LedReturnCode led_routine(time t);


#else  // CONF_LED_MODULE_ENABLE 

#define led_init(id) (LED_OK)
#define led_set_enable(enabled) CELLBOARD_NOPE()
#define led_routine(t) (LED_OK)

#endif // CONF_LED_MODULE_ENABLE 
