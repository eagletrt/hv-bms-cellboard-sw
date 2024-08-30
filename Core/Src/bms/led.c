/**
 * @file led.c
 * @date 2024-05-08
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used to manage the on-board LED
 */

#include "led.h"

#include "identity.h"

#ifdef CONF_LED_MODULE_ENABLE

_STATIC _LedHandler hled;

LedReturnCode led_init(const led_set_state_callback_t set, const led_toggle_state_callback_t toggle) {
    if (set == NULL || toggle == NULL)
        return LED_NULL_POINTER;

    hled.set = set;
    hled.toggle = toggle;
    hled.pattern_size = 0U;

    // Set pattern
    for (size_t i = 0U; i <= identity_get_cellboard_id(); ++i) {
        hled.pattern[hled.pattern_size++] = LED_SHORT_OFF_MS;
        hled.pattern[hled.pattern_size++] = LED_SHORT_ON_MS;
    }
    hled.pattern[hled.pattern_size++] = LED_LONG_OFF_MS;
    
    // Initialize the blinker structure
    blinky_init(&hled.blinker, hled.pattern, hled.pattern_size, true, BLINKY_LOW);
    blinky_enable(&hled.blinker, false);

    return LED_OK;
}

void led_set_enable(const bool enabled) {
    blinky_enable(&hled.blinker, enabled);
}

LedReturnCode led_routine(const milliseconds_t t) {
    const LedStatus state = (LedStatus)blinky_routine(&hled.blinker, t);
    hled.set(state);
    return LED_OK;
}

#ifdef CONF_LED_STRINGS_ENABLE

_STATIC char * led_module_name = "led";

_STATIC char * led_return_code_name[] = {
    [LED_OK] = "ok",
    [LED_NULL_POINTER] = "null pointer"
};

_STATIC char * led_return_code_description[] = {
    [LED_OK] = "executed succesfully",
    [LED_NULL_POINTER] = "attempt to dereference a NULL pointer"
};

#endif // CONF_LED_STRINGS_ENABLE

#endif // CONF_LED_MODULE_ENABLE
