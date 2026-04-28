/**
 * @file test_volt.c
 * @date 2024-06-22
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the voltage module
 */

#include "unity.h"
#include "volt-api.h"
#include "cellboard-def.h"
#include "identity-api.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern struct VoltHandler volt_handler;

void test_volt_init_check_struct() {

    struct VoltHandler expected_hvolt = {
        .voltages = { 0 },
        .voltages_can_payload = {
            .cellboard_id = CELLBOARD_ID,
            .offset = 0,
            .voltage_0 = 0,
            .voltage_1 = 0,
            .voltage_2 = 0 }
    };

    enum VoltReturnCode rc = volt_init();
    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OK, "volt_init() failed to return VOLT_RC_OK");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_hvolt, &volt_handler, sizeof(expected_hvolt), "volt_init() failed to initialize volt_handler correctly");
}

void test_volt_update_value_with_valid_input() {
    TEST_ASSERT_EQUAL_MESSAGE(volt_update_value(0, VOLT_MIN_V + 2), VOLT_RC_OK, "volt_update_value() failed to return VOLT_RC_OK for valid input");
}

void test_volt_update_with_value_out_of_bounds() {
    TEST_ASSERT_EQUAL_MESSAGE(volt_update_value(CELLBOARD_SEGMENT_SERIES_COUNT + 1, 0), VOLT_RC_OUT_OF_BOUNDS, "volt_update_value() failed to return VOLT_RC_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_update_values_with_valid_input() {
    volt values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    enum VoltReturnCode rc = volt_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OK, "volt_update_values() failed to return VOLT_RC_OK for valid input");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(values, volt_handler.voltages, sizeof(values), "volt_update_values() failed to update volt_handler.voltages correctly");
}

void test_volt_update_values_with_values_out_of_bounds() {
    volt values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    enum VoltReturnCode rc = volt_update_values(CELLBOARD_SEGMENT_SERIES_COUNT + 1, values, CELLBOARD_SEGMENT_SERIES_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OUT_OF_BOUNDS, "volt_update_values() failed to return VOLT_RC_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_select_values() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    bit_flag32 expected = 0b111111111111111111111110;

    bit_flag32 bits = volt_select_values_strictly_greater(VOLT_MIN_V);

    TEST_ASSERT_BITS_HIGH_MESSAGE(expected, bits, "volt_select_values_strictly_greater() returned incorrect bitmask");
}

void test_volt_get_min() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt min = volt_get_min();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V, min, "volt_get_min() returned incorrect value");
}

void test_volt_get_max() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt max = volt_get_max();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V + CELLBOARD_SEGMENT_SERIES_COUNT - 1, max, "volt_get_max() returned incorrect value");
}

void test_volt_get_avg() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt avg = volt_get_avg();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V + (CELLBOARD_SEGMENT_SERIES_COUNT - 1) / 2.0f, avg, "volt_get_avg() returned incorrect value");
}

void test_volt_get_sum() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt sum = volt_get_sum();

    TEST_ASSERT_EQUAL_MESSAGE((VOLT_MIN_V * CELLBOARD_SEGMENT_SERIES_COUNT) + ((CELLBOARD_SEGMENT_SERIES_COUNT - 1) * CELLBOARD_SEGMENT_SERIES_COUNT) / 2.0f, sum, "volt_get_sum() returned incorrect value");
}

void test_volt_dump_values_null_pointer() {
    enum VoltReturnCode rc = volt_dump_values(NULL, 0, 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_NULL_POINTER, rc, "volt_dump_values() failed to return VOLT_RC_NULL_POINTER when given a NULL pointer");
}

void test_volt_dump_values_out_of_bounds() {
    volt out[1];
    enum VoltReturnCode rc = volt_dump_values(out, CELLBOARD_SEGMENT_SERIES_COUNT + 1, 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_OUT_OF_BOUNDS, rc, "volt_dump_values() failed to return VOLT_RC_OUT_OF_BOUNDS when given an out of bounds index");
}

void test_volt_dump_values() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt out[CELLBOARD_SEGMENT_SERIES_COUNT / 2 - 1];
    enum VoltReturnCode rc = volt_dump_values(out, CELLBOARD_SEGMENT_SERIES_COUNT / 2, CELLBOARD_SEGMENT_SERIES_COUNT / 2 - 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_OK, rc, "volt_dump_values() failed to return VOLT_RC_OK for valid input");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&volt_handler.voltages[CELLBOARD_SEGMENT_SERIES_COUNT / 2], out, sizeof(out), "volt_dump_values() failed to copy the correct values to the output array");
}

void test_volt_get_canlib_payload_size() {
    volt values[4];
    for (size_t i = 0; i < 4; ++i)
        values[i] = VOLT_MIN_V + i;

    volt_update_values(0, values, 4);

    size_t byte_size;
    bms_cellboard_cells_voltage_converted_t *payload = volt_get_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(sizeof(volt_handler.voltages_can_payload), byte_size, "volt_get_canlib_payload() returned incorrect size");
}

void test_volt_get_canlib_payload_voltage() {

    volt values[4];

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
    identity_api_init(CELLBOARD_ID);
    volt_init();
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_volt_init_check_struct);
    RUN_TEST(test_volt_update_value_with_valid_input);
    RUN_TEST(test_volt_update_with_value_out_of_bounds);
    RUN_TEST(test_volt_update_values_with_valid_input);
    RUN_TEST(test_volt_update_values_with_values_out_of_bounds);
    RUN_TEST(test_volt_select_values);
    RUN_TEST(test_volt_get_min);
    RUN_TEST(test_volt_get_max);
    RUN_TEST(test_volt_get_avg);
    RUN_TEST(test_volt_get_sum);
    RUN_TEST(test_volt_dump_values_null_pointer);
    RUN_TEST(test_volt_dump_values_out_of_bounds);
    RUN_TEST(test_volt_dump_values);
    RUN_TEST(test_volt_get_canlib_payload_size);
    RUN_TEST(test_volt_get_canlib_payload_voltage);
    return UNITY_END();
}
