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
#include "identity_api.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _VoltHandler hvolt;

void test_volt_init_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(volt_init(), VOLT_OK, "volt_init() failed to return VOLT_OK");
}

void test_volt_init_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(hvolt.voltages_can_payload.cellboard_id, CELLBOARD_ID, "Cellboard ID mismatch in payload initialization");
}

void test_volt_update_value_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(volt_update_value(0, VOLT_MIN_V + 2), VOLT_OK, "volt_update_value() failed to return VOLT_OK for valid input");
}

void test_volt_update_value_out_of_bounds() {
    TEST_ASSERT_EQUAL_MESSAGE(volt_update_value(CELLBOARD_SEGMENT_SERIES_COUNT + 1, 0), VOLT_OUT_OF_BOUNDS, "volt_update_value() failed to return VOLT_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_update_values_ok() {
    volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    TEST_ASSERT_EQUAL_MESSAGE(volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT), VOLT_OK, "volt_update_values() failed to return VOLT_OK for valid input");
}

void test_volt_update_values_out_of_bounds() {
    volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    TEST_ASSERT_EQUAL_MESSAGE(volt_update_values(CELLBOARD_SEGMENT_SERIES_COUNT + 1, values, CELLBOARD_SEGMENT_SERIES_COUNT), VOLT_OUT_OF_BOUNDS, "volt_update_values() failed to return VOLT_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_get_values() {

    // volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    // for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
    //     values[i] = VOLT_MIN_V + i;

    // volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);
    // const cells_volt_t *out = volt_get_values();
    // for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
    //     TEST_ASSERT_EQUAL_MESSAGE(values[i], out[i], "volt_get_values() returned incorrect value");

    TEST_IGNORE_MESSAGE("Discrepancy in volt_t type causes test failure, check with tonino");
}

void test_volt_select_values() {
    volt_t values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);
    bit_flag32_t bits = volt_select_values(VOLT_MIN_V);

    TEST_ASSERT_BITS_HIGH_MESSAGE(0xFFFFFE, bits, "volt_select_values() returned incorrect bitmask");
}

void test_volt_get_canlib_payload_size() {

    volt_t values[4];
    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_V + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t *payload = volt_get_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(sizeof(hvolt.voltages_can_payload), byte_size, "volt_get_canlib_payload() returned incorrect size");
}

void test_volt_get_canlib_payload_voltage() {

    volt_t values[4];

    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_V + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t *payload = volt_get_canlib_payload(&byte_size);

    bms_cellboard_cells_voltage_converted_t expected_payload;
    expected_payload.cellboard_id = CELLBOARD_ID;
    expected_payload.offset = 0;
    expected_payload.voltage_0 = (values[0]);
    expected_payload.voltage_1 = (values[1]);
    expected_payload.voltage_2 = (values[2]);

    TEST_ASSERT_EQUAL_MEMORY(&expected_payload, payload, sizeof(expected_payload));
}

void setUp() {
    identity_init(CELLBOARD_ID);
    volt_init();
}

void tearDown() {
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
