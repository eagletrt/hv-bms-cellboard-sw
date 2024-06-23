/**
 * @file test_temp.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the temp module
 */

#include "unity.h"
#include "temp.h"
#include "identity.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _TempHandler htemp;

void setUp() {}

void tearDown() {}

void test_temp_init() {
    TEST_ASSERT_EQUAL(TEMP_OK, temp_init());
    TEST_ASSERT_EQUAL(CELLBOARD_ID, htemp.can_payload.cellboard_id);
}

void test_temp_update_value() {
    TEST_ASSERT_EQUAL(TEMP_OK, temp_init());
    TEST_ASSERT_EQUAL(TEMP_OK, temp_update_value(0, 0x01));
    TEST_ASSERT_EQUAL(0x01, htemp.temperatures[0]);
}

void test_temp_update_values() {
    TEST_ASSERT_EQUAL(TEMP_OK, temp_init());
    TEST_ASSERT_EQUAL(TEMP_OUT_OF_BOUNDS, temp_update_values(0, NULL, CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT));
    raw_temp_t values[2] = {0x01, 0x02};
    TEST_ASSERT_EQUAL(TEMP_OK, temp_update_values(0, values, 2));
    TEST_ASSERT_EQUAL(0x01, htemp.temperatures[0]);
    TEST_ASSERT_EQUAL(0x02, htemp.temperatures[1]);
}

void test_temp_update_discharge_value() {
}

void test_temp_update_discharge_values() {
}

void test_temp_get_values() {
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    return UNITY_END();
}
