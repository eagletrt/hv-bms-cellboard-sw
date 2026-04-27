/**
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

#include "blinky.h"

/** \brief Maximum size of the pattern */
#define LED_PATTERN_MAX_SIZE ((CELLBOARD_COUNT * 2U) + 1U)

/** \brief Timings for the led status in ms */
#define LED_SHORT_ON_MS (250U)
#define LED_SHORT_OFF_MS (250U)
#define LED_LONG_OFF_MS (1000U)

/**
 * \brief Return code for the LED module functions
 *
 * \details
 *     - LED_OK the function executed succesfully
 *     - LED_NULL_POINTER a NULL pointer was given to a function
 *     - LED_INVALID_CELLBOARD_ID invalid cellboard identifier
 */
enum LedReturnCode {
    LED_OK,
    LED_NULL_POINTER,
    LED_INVALID_CELLBOARD_ID
};

/**
 * \brief Status of a single LED
 *
 * \details
 *     - LED_OFF the led is turned off
 *     - LED_ON the led is turned on
 */
enum LedStatus {
    LED_OFF,
    LED_ON
};

/**
 * \brief Callback used to set the state of the LED
 *
 * \param state The state to set the LED to
 */
typedef void (*led_set_state_callback_t)(const enum LedStatus state);

/**
 * \brief Callback used to toggle the state of the LED
 */
typedef void (*led_toggle_state_callback_t)(void);

/**
 * \brief LED handler structure
 *
 * \attention This structure should not be used outside of this module
 *
 * \param set A pointer to the function callback used to set the led state
 * \param toggle A pointer to the function callback used to toggle the led state
 * \param blinker The structure handler used to blink the LED
 * \param pattern The main blinking pattern
 * \param pattern_size The size of the pattern
 */
struct _LedHandler {
    led_set_state_callback_t set;
    led_toggle_state_callback_t toggle;

    Blinky blinker;
    uint16_t pattern[LED_PATTERN_MAX_SIZE];
    size_t pattern_size;
};

#endif // LED_H
