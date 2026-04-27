/**
 * \file test_led.c
 * \date 2024-06-21
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the led module
 */

#include "unity.h"
#include "led-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

#define CELLBOARD_ID CELLBOARD_ID_2

extern struct _LedHandler hled;

FAKE_VOID_FUNC(led_set, const enum LedStatus);
FAKE_VOID_FUNC(led_toggle);

void test_led_init_with_null_pointer() {
    hled.pattern_size = 13;
    hled.set = led_set;
    hled.toggle = led_toggle;

    TEST_ASSERT_EQUAL_MESSAGE(LED_NULL_POINTER, led_init(NULL, NULL), "led_init did not return LED_NULL_POINTER when called with null pointers");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&(struct _LedHandler){ 0 }, &hled, sizeof(struct _LedHandler), "led_init modified the internal handler structure when called with null pointers");
}

void test_led_init_with_correct_values() {
    TEST_ASSERT_EQUAL_MESSAGE(LED_OK, led_init(led_set, led_toggle), "led_init did not return LED_OK when called with correct parameters");
}

void test_led_init_correct_struct_parameters() {
    struct _LedHandler expected = {
        .set = led_set,
        .toggle = led_toggle,
        .pattern_size = (CELLBOARD_ID + 1) * 2 + 1,
        .pattern = {
            [0] = LED_SHORT_OFF_MS,
            [1] = LED_SHORT_ON_MS,
            [2] = LED_SHORT_OFF_MS,
            [3] = LED_SHORT_ON_MS,
            [4] = LED_SHORT_OFF_MS,
            [5] = LED_SHORT_ON_MS,
            [6] = LED_LONG_OFF_MS }
    };

    led_init(led_set, led_toggle);

    expected.blinker = hled.blinker; // The blinker structure is initialized with a function call so it is not possible to set it as an expected value

    TEST_ASSERT_EQUAL_MESSAGE(expected.set, hled.set, "led_init did not correctly set the set callback");
    TEST_ASSERT_EQUAL_MESSAGE(expected.toggle, hled.toggle, "led_init did not correctly set the toggle callback");
    TEST_ASSERT_EQUAL_MESSAGE(expected.pattern_size, hled.pattern_size, "led_init did not correctly set the pattern size");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected.pattern, hled.pattern, sizeof(expected.pattern), "led_init did not correctly set the pattern");
}

void test_led_routine_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(LED_OK, led_routine(0), "led_routine did not return LED_OK");
}

void test_led_routine_set_called() {
    led_routine(0);
    TEST_ASSERT_TRUE_MESSAGE(led_set_fake.call_count > 0, "led_routine did not call led_set");
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    RESET_FAKE(led_set);
    RESET_FAKE(led_toggle);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_led_init_with_null_pointer);
    RUN_TEST(test_led_init_with_correct_values);
    RUN_TEST(test_led_init_correct_struct_parameters);
    RUN_TEST(test_led_routine_ok);
    RUN_TEST(test_led_routine_set_called);
    return UNITY_END();
}
