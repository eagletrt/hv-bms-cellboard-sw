/**
 * @file led.c
 * @date 2024-05-08
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used to manage the on-board LED
 */

#include "led.h"

#include "blinky.h"
#include "cellboard-def.h"
#include "identity.h"

#ifdef CONF_LED_MODULE_ENABLE

/** @brief Timings for the led status in ms */
#define LED_SHORT_ON (250U)
#define LED_SHORT_OFF (250U)
#define LED_LONG_OFF (1000U)

//WARNING: this structure should never be used outside this file
_STATIC _LedHandler hled;

LedReturnCode led_init(led_set_state_callback_t set, led_toggle_state_callback_t toggle) {
    if (set == NULL || toggle == NULL)
        return LED_NULL_POINTER;

    hled.set = set;
    hled.toggle = toggle;
    hled.pattern_size = 0U;

    // Set pattern
    for (size_t i = 0U; i <= identity_get_cellboard_id(); ++i) {
        hled.pattern[hled.pattern_size++] = LED_SHORT_OFF;
        hled.pattern[hled.pattern_size++] = LED_SHORT_ON;
    }
    hled.pattern[hled.pattern_size++] = LED_LONG_OFF;
    
    // Initialize the blinker structure
    blinky_init(&hled.blinker, hled.pattern, hled.pattern_size, true, BLINKY_LOW);
    blinky_enable(&hled.blinker, false);

    return LED_OK;
}

void led_set_enable(bool enabled) {
    blinky_enable(&hled.blinker, enabled);
}

LedReturnCode led_routine(milliseconds_t t) {
    LedStatus state = (LedStatus)blinky_routine(&hled.blinker, t);
    hled.set(state);
    return LED_OK;
}

#ifdef CONF_LED_STRINGS_ENABLE

static char * led_module_name = "led";

static char * led_return_code_name[] = {
    [LED_OK] = "ok",
    [LED_NULL_POINTER] = "null pointer",
    [LED_INVALID_CELLBOARD_ID] = "invalid cellboard id"
};

static char * led_return_code_description[] = {
    [LED_OK] = "executed succesfully",
    [LED_NULL_POINTER] = "attempt to dereference a NULL pointer",
    [LED_INVALID_CELLBOARD_ID] = "the given id does not correspond to any valid cellboard identifier"
};

#endif // CONF_LED_STRINGS_ENABLE

#endif // CONF_LED_MODULE_ENABLE
