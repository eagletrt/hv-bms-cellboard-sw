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

void setUp() {
    identity_init(CELLBOARD_ID);
    volt_init();
}

void tearDown() {}

void test_volt_init_ok() {
    TEST_ASSERT_EQUAL(volt_init(), VOLT_OK);
}

void test_volt_init_cellboard_id() {
    TEST_ASSERT_EQUAL(hvolt.can_payload.cellboard_id, CELLBOARD_ID);
}

void test_volt_update_value_ok() {
    TEST_ASSERT_EQUAL(volt_update_value(0, VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE)+2), VOLT_OK);
}

void test_volt_update_value_out_of_bounds() {
    TEST_ASSERT_EQUAL(volt_update_value(CELLBOARD_SEGMENT_SERIES_COUNT + 1, 0), VOLT_OUT_OF_BOUNDS);
}

void test_volt_update_values_ok() {
    raw_volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE) + i;

    TEST_ASSERT_EQUAL(volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT), VOLT_OK);
}

void test_volt_update_values_out_of_bounds() {
    raw_volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_VALUE) + i;

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

void test_volt_get_canlib_payload_size() {

    raw_volt_t values[4];
    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_VALUE + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t * payload = volt_get_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL(sizeof(hvolt.can_payload), byte_size);
}

void test_volt_get_canlib_payload_voltage() {

    raw_volt_t values[4];

    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_VALUE + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t* payload = volt_get_canlib_payload(&byte_size);

    bms_cellboard_cells_voltage_converted_t expected_payload;
    expected_payload.cellboard_id = CELLBOARD_ID;
    expected_payload.offset = 0;
    expected_payload.voltage_0 = VOLT_VALUE_TO_MILLIVOLT(values[0]);
    expected_payload.voltage_1 = VOLT_VALUE_TO_MILLIVOLT(values[1]);
    expected_payload.voltage_2 = VOLT_VALUE_TO_MILLIVOLT(values[2]);
    expected_payload.voltage_3 = VOLT_VALUE_TO_MILLIVOLT(values[3]);

    TEST_ASSERT_EQUAL_MEMORY(&expected_payload, payload, sizeof(expected_payload));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_volt_init_ok);
    RUN_TEST(test_volt_init_cellboard_id);
    RUN_TEST(test_volt_update_value_ok);
    RUN_TEST(test_volt_update_value_out_of_bounds);
    RUN_TEST(test_volt_update_values_ok);
    RUN_TEST(test_volt_update_values_out_of_bounds);
    RUN_TEST(test_volt_get_values);
    RUN_TEST(test_volt_select_values);
    RUN_TEST(test_volt_get_canlib_payload_size);
    RUN_TEST(test_volt_get_canlib_payload_voltage);
    return UNITY_END();
}
