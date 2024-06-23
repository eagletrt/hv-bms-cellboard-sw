/**
 * @file test_volt.c
 * @date 2024-06-22
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the voltage module
 */

#include "unity.h"
#include "volt.h"
#include "cellboard-def.h"
#include "identity.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _VoltHandler hvolt;

void setUp() {}

void tearDown() {}

void test_volt_init() {
    TEST_ASSERT_EQUAL(volt_init(), VOLT_OK);

    volt_init();
    TEST_ASSERT_EQUAL(hvolt.can_payload.cellboard_id, CELLBOARD_ID);
}

void test_volt_update_value() {
    TEST_ASSERT_EQUAL(volt_update_value(0, VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE)+2), VOLT_OK);
    TEST_ASSERT_EQUAL(volt_update_value(CELLBOARD_SEGMENT_SERIES_COUNT + 1, 0), VOLT_OUT_OF_BOUNDS);
}

void test_volt_update_values() {
    raw_volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE) + i;

    TEST_ASSERT_EQUAL(volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT), VOLT_OK);
    TEST_ASSERT_EQUAL(volt_update_values(CELLBOARD_SEGMENT_SERIES_COUNT + 1, values, CELLBOARD_SEGMENT_SERIES_COUNT), VOLT_OUT_OF_BOUNDS);
}

void test_volt_get_values() {
    raw_volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE) + i;

    volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);
    const raw_volt_t * out = volt_get_values();
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        TEST_ASSERT_EQUAL(out[i], values[i]);
}

void test_volt_select_values() {
    raw_volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_VALUE + i;

    volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);
    bit_flag32_t bits = volt_select_values(VOLT_VALUE_TO_MILLIVOLT(VOLT_MIN_VALUE));

    TEST_ASSERT_BITS_HIGH(0xFFFFFE, bits);
}

void test_volt_get_canlib_payload() {

    raw_volt_t values[4];
    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_VALUE + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t * payload = volt_get_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL(sizeof(hvolt.can_payload), byte_size);
    TEST_ASSERT_EQUAL(payload->voltage_0, VOLT_VALUE_TO_MILLIVOLT(values[0]));
    TEST_ASSERT_EQUAL(payload->voltage_1, VOLT_VALUE_TO_MILLIVOLT(values[1]));
    TEST_ASSERT_EQUAL(payload->voltage_2, VOLT_VALUE_TO_MILLIVOLT(values[2]));
    TEST_ASSERT_EQUAL(payload->voltage_3, VOLT_VALUE_TO_MILLIVOLT(values[3]));
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_volt_init);
    RUN_TEST(test_volt_update_value);
    RUN_TEST(test_volt_update_values);
    RUN_TEST(test_volt_get_values);
    RUN_TEST(test_volt_select_values);
    RUN_TEST(test_volt_get_canlib_payload);
    return UNITY_END();
}
