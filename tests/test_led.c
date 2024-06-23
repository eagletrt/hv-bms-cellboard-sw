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

void setUp() {}

void tearDown() {}

void test_led_init() {
    TEST_ASSERT_EQUAL(LED_NULL_POINTER, led_init(NULL, NULL));
    TEST_ASSERT_EQUAL(LED_OK, led_init((void*) 0x01, (void*) 0x01));

    led_init((void*) 0x01, (void*) 0x01);
    TEST_ASSERT_EQUAL((CELLBOARD_ID+1)*2+1, hled.pattern_size);
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_led_init);
    return UNITY_END();
}
