/*!
 * \file led-api.c
 * \date 2024-05-08
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * \brief Functions used to manage the on-board LED
 */

#include "led.h"

#include "eagletrt.h"
#include "identity-api.h"
#include "blinky-api.h"

#ifdef CONF_LED_MODULE_ENABLE

EAGLETRT_STATIC struct LedHandler led_handler;

enum LedReturnCode led_api_init(const led_set_state_callback set, const led_toggle_state_callback toggle) {
    memset(&led_handler, 0, sizeof(led_handler));

    if (set == NULL || toggle == NULL) {
        return LED_RC_NULL_POINTER;
    }

    led_handler.set = set;
    led_handler.toggle = toggle;
    led_handler.pattern_size = 0U;

    // Set pattern
    size_t pattern_size = 0;
    for (size_t i = 0U; i <= identity_api_get_cellboard_id(); ++i) {
        led_handler.pattern[pattern_size] = LED_SHORT_OFF_MS;
        led_handler.pattern[pattern_size + 1] = LED_SHORT_ON_MS;
        pattern_size += 2;
    }
    led_handler.pattern[pattern_size] = LED_LONG_OFF_MS;
    ++pattern_size;
    led_handler.pattern_size = pattern_size;

    // Initialize the blinker structure
    blinky_api_init(&led_handler.blinker, led_handler.pattern, led_handler.pattern_size, true, BLINKY_LOW);
    blinky_api_enable(&led_handler.blinker, false);

    return LED_RC_OK;
}

void led_api_set_enable(const bool enabled) {
    blinky_api_enable(&led_handler.blinker, enabled);
}

enum LedReturnCode led_api_routine(const milliseconds_t time) {
    const enum LedStatus state = (enum LedStatus)blinky_api_routine(&led_handler.blinker, time);
    led_handler.set(state);
    return LED_RC_OK;
}

#ifdef CONF_LED_STRINGS_ENABLE

EAGLETRT_STATIC char *led_module_name = "led";

EAGLETRT_STATIC char *led_return_code_name[] = {
    [LED_RC_OK] = "ok",
    [LED_RC_NULL_POINTER] = "null pointer"
};

EAGLETRT_STATIC char *led_return_code_description[] = {
    [LED_RC_OK] = "executed succesfully",
    [LED_RC_NULL_POINTER] = "attempt to dereference a NULL pointer"
};

#endif // CONF_LED_STRINGS_ENABLE

#endif // CONF_LED_MODULE_ENABLE
