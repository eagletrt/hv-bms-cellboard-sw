#include "unity.h"
#include "led.h"
#include "mocks/Mockidentity.h"
#include "cellboard-def.h"

#define TEST_CELLBOARD_ID CELLBOARD_ID_3

extern _LedHandler hled;

void setUp() {}

void tearDown() {}

void test_led_init() {
    TEST_ASSERT_EQUAL(led_init(NULL, NULL), LED_NULL_POINTER);
    TEST_ASSERT_EQUAL(led_init((void*) 0x01, (void*) 0x01), LED_OK);

    led_init((void*) 0x01, (void*) 0x01);
    TEST_ASSERT_EQUAL(hled.pattern_size, (TEST_CELLBOARD_ID+1)*2+1);
}

int main() {
    UNITY_BEGIN();

    identity_get_cellboard_id_IgnoreAndReturn(TEST_CELLBOARD_ID);

    RUN_TEST(test_led_init);
    return UNITY_END();
}
