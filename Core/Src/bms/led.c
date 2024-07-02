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

/** @brief Maximum size of the pattern */
#define LED_PATTERN_MAX_SIZE ((CELLBOARD_COUNT * 2U) + 1U)

/** @brief Timings for the led status in ms */
#define LED_SHORT_ON (250U)
#define LED_SHORT_OFF (250U)
#define LED_LONG_OFF (1000U)

/**
 * @brief LED handler structure
 *
 * @param set A pointer to the function callback used to set the led state
 * @param toggle A pointer to the function callback used to toggle the led state
 * @param blinker The structure handler used to blink the LED
 * @param pattern The main blinking pattern
 * @param pattern_size The size of the pattern
 */
_STATIC struct LedHandler {
    led_set_state_callback_t set;
    led_toggle_state_callback_t toggle;

    Blinky blinker;
    uint16_t pattern[LED_PATTERN_MAX_SIZE];
    size_t pattern_size;
} hled;


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
