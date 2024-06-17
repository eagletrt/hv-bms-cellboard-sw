#include "unity.h"
#include "led.h"
#include "mocks/Mockidentity.h"

void setUp() {}

void tearDown() {}

void test_led_init() {
    TEST_ASSERT_EQUAL(led_init(NULL, NULL), LED_NULL_POINTER);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_led_init);
    return UNITY_END();
}
