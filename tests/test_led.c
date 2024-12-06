/**
 * @file test_led.c
 * @date 2024-06-21
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the led module
 */

#include "unity.h"
#include "led.h"
#include "identity.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _LedHandler hled;

bool led_set_called = false;
static void led_set(LedStatus state) {
    led_set_called = true;
}

static void led_toggle() {
    // Do nothing
}

void setUp() {
    identity_init(CELLBOARD_ID);
    led_set_called = false;
}

void tearDown() {}




void test_led_init_return_null() {
    TEST_ASSERT_EQUAL(LED_NULL_POINTER, led_init(NULL, NULL));
}

void test_led_init_return_ok() {
    TEST_ASSERT_EQUAL(LED_OK, led_init(led_set, led_toggle));
}

void test_led_init_return_pattern_size() {
    led_init(led_set, led_toggle);
    TEST_ASSERT_EQUAL((CELLBOARD_ID+1)*2+1, hled.pattern_size);
}

void test_led_routine_ok() {
    TEST_ASSERT_EQUAL(LED_OK, led_routine(0));
}

void test_led_routine_set_called() {
    led_routine(0);
    TEST_ASSERT_TRUE(led_set_called);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_led_init_return_null);
    RUN_TEST(test_led_init_return_ok);
    RUN_TEST(test_led_init_return_pattern_size);
    RUN_TEST(test_led_routine_ok);
    RUN_TEST(test_led_routine_set_called);
    return UNITY_END();
}
