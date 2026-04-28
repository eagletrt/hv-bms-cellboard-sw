/*!
 * \file test_temp.c
 * \date 2024-06-23
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the temp module
 */

#include "unity.h"
#include "temp-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "error.h"
#include "fff.h"

DEFINE_FFF_GLOBALS;

#define CELLBOARD_ID CELLBOARD_ID_1

extern struct TempHandler temp_handler;

FAKE_VOID_FUNC(mock_set_address, uint8_t);
FAKE_VOID_FUNC(mock_start_conversion);

void test_temp_init_ok() {

    const enum TempReturnCode rc = temp_init(mock_set_address, mock_start_conversion);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_init() failed to return TEMP_OK");
}

void test_temp_init_null_set_address() {

    const enum TempReturnCode rc = temp_init(NULL, mock_start_conversion);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_NULL_POINTER, rc, "temp_init() should fail with NULL set_address");
}

void test_temp_init_null_start_conversion() {

    const enum TempReturnCode rc = temp_init(mock_set_address, NULL);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_NULL_POINTER, rc, "temp_init() should fail with NULL start_conversion");
}

void test_temp_init_correct_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, temp_handler.temp_can_payload.cellboard_id, "Cellboard ID mismatch in payload initialization");
}

void test_temp_start_conversion_ok() {

    const enum TempReturnCode rc = temp_start_conversion();

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_start_conversion() failed to return TEMP_OK");
    TEST_ASSERT_TRUE_MESSAGE(temp_handler.busy, "Module should be busy after starting conversion");
    TEST_ASSERT_EQUAL_MESSAGE(1U, mock_set_address_fake.call_count, "temp_start_conversion() should call set_address() once");
    TEST_ASSERT_EQUAL_MESSAGE(1U, mock_start_conversion_fake.call_count, "temp_start_conversion() should call start_conversion() once");
    TEST_ASSERT_EQUAL_MESSAGE(1U, mock_set_address_fake.arg0_val, "temp_start_conversion() should select the next mux address");
}

void test_temp_start_conversion_busy() {
    temp_start_conversion();

    const enum TempReturnCode rc = temp_start_conversion();

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_BUSY, rc, "temp_start_conversion() should return TEMP_BUSY if already busy");
}

void test_temp_notify_conversion_complete_ok() {

    temp_start_conversion();
    volt_t raw_values[2] = { 1.5f, 1.5f };

    const enum TempReturnCode rc = temp_notify_conversion_complete(raw_values, 2U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_notify_conversion_complete() failed to return TEMP_OK");
    TEST_ASSERT_FALSE_MESSAGE(temp_handler.busy, "Module should not be busy after notification");
}

void test_temp_update_value_ok() {

    const celsius expected = 25.0f;

    const enum TempReturnCode rc = temp_update_value(3U, expected);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_update_value() failed to return TEMP_OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, expected, temp_handler.temperatures[3], "Temperature value not updated correctly");
}

void test_temp_update_value_out_of_bounds() {

    const enum TempReturnCode rc = temp_update_value(CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT + 1U, 25.0f);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, rc, "temp_update_value() should return TEMP_OUT_OF_BOUNDS for an invalid index");
}

void test_temp_update_values_ok() {

    celsius values[2] = { 20.0f, 30.0f };

    const enum TempReturnCode rc = temp_update_values(3U, values, 2U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_update_values() failed to return TEMP_OK");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 20.0f, temp_handler.temperatures[3], "First value mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 30.0f, temp_handler.temperatures[4], "Second value mismatch");
}

void test_temp_update_values_out_of_bounds() {

    celsius values[2] = { 20.0f, 30.0f };

    const enum TempReturnCode rc = temp_update_values(CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT, values, 1U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, rc, "temp_update_values() should return TEMP_OUT_OF_BOUNDS for an invalid range");
}

void test_temp_update_discharge_value_ok() {

    temp_handler.discharge_temperatures[3] = 0.0f;

    const enum TempReturnCode rc = temp_update_discharge_value(3U, 2.0f);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_update_discharge_value() failed to return TEMP_OK");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(0.0f, temp_handler.discharge_temperatures[3], "temp_update_discharge_value() failed to update the discharge temperature");
}

void test_temp_update_discharge_value_out_of_bounds() {

    const enum TempReturnCode rc = temp_update_discharge_value(CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT + 1U, 2.0f);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, rc, "temp_update_discharge_value() should return TEMP_OUT_OF_BOUNDS for an invalid index");
}

void test_temp_update_discharge_values_ok() {

    temp_handler.discharge_temperatures[1] = -999.0f;
    temp_handler.discharge_temperatures[2] = -999.0f;
    volt_t values[2] = { 2.0f, 2.1f };

    const enum TempReturnCode rc = temp_update_discharge_values(1U, values, 2U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_update_discharge_values() failed to return TEMP_OK");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(-999.0f, temp_handler.discharge_temperatures[1], "First discharge temperature was not updated");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(-999.0f, temp_handler.discharge_temperatures[2], "Second discharge temperature was not updated");
}

void test_temp_update_discharge_values_out_of_bounds() {

    volt_t values[1] = { 2.0f };

    const enum TempReturnCode rc = temp_update_discharge_values(CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT, values, 1U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, rc, "temp_update_discharge_values() should return TEMP_OUT_OF_BOUNDS for an invalid range");
}

void test_temp_get_min() {

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i)
        temp_handler.temperatures[i] = 60.0f;
    temp_handler.temperatures[0] = 10.0f;
    temp_handler.temperatures[1] = 20.0f;
    temp_handler.temperatures[2] = 30.0f;

    const celsius min = temp_get_min();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 10.0f, min, "temp_get_min() failed");
}

void test_temp_get_max() {

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i)
        temp_handler.temperatures[i] = 0.0f;
    temp_handler.temperatures[5] = 45.0f;

    const celsius max = temp_get_max();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 45.0f, max, "temp_get_max() failed");
}

void test_temp_get_sum() {

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i)
        temp_handler.temperatures[i] = 1.0f;

    const celsius sum = temp_get_sum();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1f, (float)CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT, sum, "temp_get_sum() failed");
}

void test_temp_get_avg() {

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2; ++i)
        temp_handler.temperatures[i] = 10.0f;

    for (size_t i = CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i)
        temp_handler.temperatures[i] = 20.0f;

    const celsius avg = temp_get_avg();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 15.0f, avg, "temp_get_avg() failed");
}

void test_temp_dump_values_null_pointer() {

    const enum TempReturnCode rc = temp_dump_values(NULL, 0U, 1U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_NULL_POINTER, rc, "temp_dump_values() should return TEMP_NULL_POINTER when given a NULL pointer");
}

void test_temp_dump_values_out_of_bounds() {

    celsius out[1];

    const enum TempReturnCode rc = temp_dump_values(out, CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT + 1U, 1U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, rc, "temp_dump_values() should return TEMP_OUT_OF_BOUNDS when given an out of bounds index");
}

void test_temp_dump_values_ok() {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i)
        temp_handler.temperatures[i] = 10.0f + (celsius)i;

    celsius out[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2 - 1U];

    celsius expected[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2 - 1U];
    for (size_t i = 0U; i < sizeof(out) / sizeof(out[0]); ++i)
        expected[i] = temp_handler.temperatures[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2 + i];

    const enum TempReturnCode rc = temp_dump_values(out, CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2U, CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 2U - 1U);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, rc, "temp_dump_values() failed to return TEMP_OK for valid input");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(expected, out, sizeof(out), "temp_dump_values() failed to copy the correct values to the output array");
}

void test_temp_get_cells_temp_canlib_payload_values() {

    temp_handler.temperatures[0] = 10.0f;
    temp_handler.temperatures[1] = 11.0f;
    temp_handler.temperatures[2] = 12.0f;
    temp_handler.temperatures[3] = 13.0f;

    bms_cellboard_cells_temperature_converted_t expected_payload = { 0 };
    expected_payload.cellboard_id = CELLBOARD_ID;
    expected_payload.offset = 0U;
    expected_payload.temperature_0 = 10.0f;
    expected_payload.temperature_1 = 11.0f;
    expected_payload.temperature_2 = 12.0f;
    expected_payload.temperature_3 = 13.0f;

    size_t size;
    bms_cellboard_cells_temperature_converted_t *payload = temp_get_cells_temp_canlib_payload(&size);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_payload, payload, sizeof(expected_payload), "temp_get_cells_temp_canlib_payload() returned incorrect payload contents");
}

void test_temp_get_discharge_temp_canlib_payload_values() {
    temp_handler.discharge_temperatures[0] = 50.0f;
    temp_handler.discharge_temperatures[1] = 51.0f;
    temp_handler.discharge_temperatures[2] = 52.0f;
    temp_handler.discharge_temperatures[3] = 53.0f;
    temp_handler.discharge_temperatures[4] = 54.0f;

    bms_cellboard_discharge_temperature_converted_t expected_payload = { 0 };
    expected_payload.temperature_0 = 50.0f;
    expected_payload.temperature_1 = 51.0f;
    expected_payload.temperature_2 = 52.0f;
    expected_payload.temperature_3 = 53.0f;
    expected_payload.temperature_4 = 54.0f;

    size_t size;
    bms_cellboard_discharge_temperature_converted_t *payload = temp_get_discharge_temp_canlib_payload(&size);

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_payload, payload, sizeof(expected_payload), "temp_get_discharge_temp_canlib_payload() returned incorrect payload contents");
}

void setUp() {
    RESET_FAKE(mock_set_address);
    RESET_FAKE(mock_start_conversion);
    FFF_RESET_HISTORY();
    identity_api_init(CELLBOARD_ID);
    temp_init(mock_set_address, mock_start_conversion);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_temp_init_ok);
    RUN_TEST(test_temp_init_null_set_address);
    RUN_TEST(test_temp_init_null_start_conversion);
    RUN_TEST(test_temp_init_correct_cellboard_id);
    RUN_TEST(test_temp_start_conversion_ok);
    RUN_TEST(test_temp_start_conversion_busy);
    RUN_TEST(test_temp_notify_conversion_complete_ok);
    RUN_TEST(test_temp_update_value_ok);
    RUN_TEST(test_temp_update_value_out_of_bounds);
    RUN_TEST(test_temp_update_values_ok);
    RUN_TEST(test_temp_update_values_out_of_bounds);
    RUN_TEST(test_temp_update_discharge_value_ok);
    RUN_TEST(test_temp_update_discharge_value_out_of_bounds);
    RUN_TEST(test_temp_update_discharge_values_ok);
    RUN_TEST(test_temp_update_discharge_values_out_of_bounds);
    RUN_TEST(test_temp_get_min);
    RUN_TEST(test_temp_get_max);
    RUN_TEST(test_temp_get_sum);
    RUN_TEST(test_temp_get_avg);
    RUN_TEST(test_temp_dump_values_null_pointer);
    RUN_TEST(test_temp_dump_values_out_of_bounds);
    RUN_TEST(test_temp_dump_values_ok);
    RUN_TEST(test_temp_get_cells_temp_canlib_payload_values);
    RUN_TEST(test_temp_get_discharge_temp_canlib_payload_values);
    return UNITY_END();
}
