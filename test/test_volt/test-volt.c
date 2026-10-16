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
        .voltages = { 0 }
    };

    enum VoltReturnCode rc = volt_api_init();
    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OK, "volt_init() failed to return VOLT_RC_OK");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_hvolt, &volt_handler, sizeof(expected_hvolt), "volt_init() failed to initialize volt_handler correctly");
}

void test_volt_update_value_with_valid_input() {
    enum VoltReturnCode rc = volt_api_update_value(0, VOLT_MIN_V + 2);
    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OK, "volt_update_value() failed to return VOLT_RC_OK for valid input");
    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V + 2, volt_handler.voltages[0], "volt_update_value() failed to update volt_handler.voltages correctly");
}

void test_volt_update_with_value_out_of_bounds() {
    enum VoltReturnCode rc = volt_api_update_value(CELLBOARD_SEGMENT_SERIES_COUNT + 1, 0);
    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OUT_OF_BOUNDS, "volt_update_value() failed to return VOLT_RC_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_update_values_with_valid_input() {
    volt values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    enum VoltReturnCode rc = volt_api_update_values(0, values, CELLBOARD_SEGMENT_SERIES_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OK, "volt_update_values() failed to return VOLT_RC_OK for valid input");
    TEST_ASSERT_EQUAL_FLOAT_ARRAY_MESSAGE(values, volt_handler.voltages, CELLBOARD_SEGMENT_SERIES_COUNT, "volt_update_values() failed to update volt_handler.voltages correctly");
}

void test_volt_update_values_with_values_out_of_bounds() {
    volt values[CELLBOARD_SEGMENT_SERIES_COUNT];
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        values[i] = VOLT_MIN_V + i;

    enum VoltReturnCode rc = volt_api_update_values(CELLBOARD_SEGMENT_SERIES_COUNT + 1, values, CELLBOARD_SEGMENT_SERIES_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_OUT_OF_BOUNDS, "volt_update_values() failed to return VOLT_RC_OUT_OF_BOUNDS for out of bounds input");
}

void test_volt_update_values_with_null_pointer() {
    enum VoltReturnCode rc = volt_api_update_values(0, NULL, CELLBOARD_SEGMENT_SERIES_COUNT);

    TEST_ASSERT_EQUAL_MESSAGE(rc, VOLT_RC_NULL_POINTER, "volt_update_values() failed to return VOLT_RC_NULL_POINTER when given a NULL pointer");
}

void test_volt_select_values_valid_input() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    bit_flag32 expected = 0b111111111111111111111110;

    bit_flag32 bits = volt_api_select_values_above_target(VOLT_MIN_V);

    TEST_ASSERT_BITS_HIGH_MESSAGE(expected, bits, "volt_select_values_above_target() returned incorrect bitmask");
}

void test_volt_get_min() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt min = volt_api_get_min();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V, min, "volt_get_min() returned incorrect value");
}

void test_volt_get_max() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt max = volt_api_get_max();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V + CELLBOARD_SEGMENT_SERIES_COUNT - 1, max, "volt_get_max() returned incorrect value");
}

void test_volt_get_avg() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt avg = volt_api_get_avg();

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_MIN_V + (CELLBOARD_SEGMENT_SERIES_COUNT - 1) / 2.0f, avg, "volt_get_avg() returned incorrect value");
}

void test_volt_get_sum() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt sum = volt_api_get_sum();

    TEST_ASSERT_EQUAL_MESSAGE((VOLT_MIN_V * CELLBOARD_SEGMENT_SERIES_COUNT) + ((CELLBOARD_SEGMENT_SERIES_COUNT - 1) * CELLBOARD_SEGMENT_SERIES_COUNT) / 2.0f, sum, "volt_get_sum() returned incorrect value");
}

void test_volt_dump_values_null_pointer() {
    enum VoltReturnCode rc = volt_api_dump_values(NULL, 0, 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_NULL_POINTER, rc, "volt_dump_values() failed to return VOLT_RC_NULL_POINTER when given a NULL pointer");
}

void test_volt_dump_values_out_of_bounds() {
    volt out[1];
    enum VoltReturnCode rc = volt_api_dump_values(out, CELLBOARD_SEGMENT_SERIES_COUNT + 1, 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_OUT_OF_BOUNDS, rc, "volt_dump_values() failed to return VOLT_RC_OUT_OF_BOUNDS when given an out of bounds index");
}

void test_volt_dump_values_valid_input() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    volt out[CELLBOARD_SEGMENT_SERIES_COUNT / 2 - 1];
    enum VoltReturnCode rc = volt_api_dump_values(out, CELLBOARD_SEGMENT_SERIES_COUNT / 2, CELLBOARD_SEGMENT_SERIES_COUNT / 2 - 1);

    TEST_ASSERT_EQUAL_MESSAGE(VOLT_RC_OK, rc, "volt_dump_values() failed to return VOLT_RC_OK for valid input");
    TEST_ASSERT_EQUAL_FLOAT_ARRAY_MESSAGE(&volt_handler.voltages[CELLBOARD_SEGMENT_SERIES_COUNT / 2], out, CELLBOARD_SEGMENT_SERIES_COUNT / 2 - 1, "volt_dump_values() failed to copy the correct values to the output array");
}

void test_volt_get_voltage_canlib_payload_size() {
    size_t byte_size;
    volt_api_get_voltage_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(can_bms_byte_size_tsaccellboard2voltage, byte_size, "volt_get_voltage_canlib_payload() returned incorrect size");
}

void test_volt_get_voltage_canlib_payload_voltage() {
    volt values[12];
    for (size_t i = 0; i < 12; ++i)
        values[i] = VOLT_MIN_V + i;

    volt_api_update_values(0, values, 12);

    // The first call selects the group 1, which contains the cells from 7 to 12
    union CanBmsMessages *payload = volt_api_get_voltage_canlib_payload(NULL);

    TEST_ASSERT_EQUAL_MESSAGE(1, payload->tsaccellboard2voltage.group, "volt_get_voltage_canlib_payload() selected incorrect group");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(values[6], payload->tsaccellboard2voltage.group_payload.mux_1.cell7, "volt_get_voltage_canlib_payload() returned incorrect cell7");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(values[11], payload->tsaccellboard2voltage.group_payload.mux_1.cell12, "volt_get_voltage_canlib_payload() returned incorrect cell12");
}

void test_volt_get_voltage_info_canlib_payload_size() {
    size_t byte_size;
    volt_api_get_voltage_info_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(can_bms_byte_size_tsaccellboard2voltageinfo, byte_size, "volt_get_voltage_info_canlib_payload() returned incorrect size");
}

void test_volt_get_voltage_info_canlib_payload_voltage() {
    for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i)
        volt_handler.voltages[i] = VOLT_MIN_V + i;

    union CanBmsMessages *payload = volt_api_get_voltage_info_canlib_payload(NULL);

    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(volt_api_get_sum(), payload->tsaccellboard2voltageinfo.total, "volt_get_voltage_info_canlib_payload() returned incorrect total");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(volt_api_get_avg(), payload->tsaccellboard2voltageinfo.average, "volt_get_voltage_info_canlib_payload() returned incorrect average");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(volt_api_get_min(), payload->tsaccellboard2voltageinfo.min, "volt_get_voltage_info_canlib_payload() returned incorrect min");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(volt_api_get_max(), payload->tsaccellboard2voltageinfo.max, "volt_get_voltage_info_canlib_payload() returned incorrect max");
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    volt_api_init();
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
    RUN_TEST(test_volt_update_values_with_null_pointer);
    RUN_TEST(test_volt_select_values_valid_input);
    RUN_TEST(test_volt_get_min);
    RUN_TEST(test_volt_get_max);
    RUN_TEST(test_volt_get_avg);
    RUN_TEST(test_volt_get_sum);
    RUN_TEST(test_volt_dump_values_null_pointer);
    RUN_TEST(test_volt_dump_values_out_of_bounds);
    RUN_TEST(test_volt_dump_values_valid_input);
    RUN_TEST(test_volt_get_voltage_canlib_payload_size);
    RUN_TEST(test_volt_get_voltage_canlib_payload_voltage);
    RUN_TEST(test_volt_get_voltage_info_canlib_payload_size);
    RUN_TEST(test_volt_get_voltage_info_canlib_payload_voltage);
    return UNITY_END();
}
