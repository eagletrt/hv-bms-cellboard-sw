/*!
 * \file led.h
 * \date 2024-05-08
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 * 
 * \brief Functions used to manage the on-board LED
 */

#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <string.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "blinky-api.h"

/*! \brief Maximum size of the pattern */
#define LED_PATTERN_MAX_SIZE ((CELLBOARD_COUNT * 2U) + 1U)

/*! \defgroup LED_TIMINGS Timings for the LED status
 *  @{
 */

/*! Short ON time in ms */
#define LED_SHORT_ON_MS (250U)

/*! Short OFF time in ms */
#define LED_SHORT_OFF_MS (250U)

/*! Long OFF time in ms */
#define LED_LONG_OFF_MS (1000U)

/*! @} */

/*!
 * \brief Return code for the LED module functions
 */
enum LedReturnCode {
    LED_RC_OK,                  /*!< The function executed successfully */
    LED_RC_NULL_POINTER,        /*!< A NULL pointer was given to a function */
    LED_RC_INVALID_CELLBOARD_ID /*!< An invalid cellboard identifier was used */
};

/*!
 * \brief Status of a single LED
 */
enum LedStatus {
    LED_STATUS_OFF, /*!< The led is turned off */
    LED_STATUS_ON   /*!< The led is turned on */
};

/*!
 * \brief Callback used to set the state of the LED
 *
 * \param state The state to set the LED to
 */
typedef void (*led_set_state_callback)(const enum LedStatus state);

/*!
 * \brief Callback used to toggle the state of the LED
 */
typedef void (*led_toggle_state_callback)(void);

/*!
 * \brief LED handler structure
 *
 * \attention This structure should not be used outside of this module
 */
struct LedHandler {
    led_set_state_callback set;       /*!< A pointer to the function callback used to set the led state */
    led_toggle_state_callback toggle; /*!< A pointer to the function callback used to toggle the led state */

    struct Blinky blinker;                  /*!< The structure handler used to blink the LED */
    uint16_t pattern[LED_PATTERN_MAX_SIZE]; /*!< The main blinking pattern */
    size_t pattern_size;                    /*!< The size of the pattern */
};

#endif // LED_H
