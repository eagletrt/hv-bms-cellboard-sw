/**
 * @file test_temp.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the temp module
 */

#include "unity.h"
#include "temp.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "error.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _TempHandler htemp;

void mock_set_address(const uint8_t address) {
    (void)address;
}

void mock_start_conversion(void) {
}

void test_temp_init_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_init(mock_set_address, mock_start_conversion), "temp_init() failed to return TEMP_OK");
}

void test_temp_init_null_pointer() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_NULL_POINTER, temp_init(NULL, mock_start_conversion), "temp_init() should fail with NULL set_address");
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_NULL_POINTER, temp_init(mock_set_address, NULL), "temp_init() should fail with NULL start_conversion");
}

void test_temp_init_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, htemp.temp_can_payload.cellboard_id, "Cellboard ID mismatch in payload initialization");
}

void test_temp_start_conversion_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_start_conversion(), "temp_start_conversion() failed to return TEMP_OK");
    TEST_ASSERT_TRUE_MESSAGE(htemp.busy, "Module should be busy after starting conversion");
}

void test_temp_start_conversion_busy() {
    temp_start_conversion();
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_BUSY, temp_start_conversion(), "temp_start_conversion() should return TEMP_BUSY if already busy");
}

void test_temp_notify_conversion_complete() {
    temp_start_conversion();

    // Values in Volts to trigger polynomial conversion
    volt_t raw_values[2] = { 1.5f, 1.5f };

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_notify_conversion_complete(raw_values, 2), "temp_notify_conversion_complete() failed");
    TEST_ASSERT_FALSE_MESSAGE(htemp.busy, "Module should not be busy after notification");
}

void test_temp_update_value_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_update_value(0, 25.0f), "temp_update_value() failed");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 25.0f, htemp.temperatures[0], "Temperature value not updated correctly");
}

void test_temp_update_value_out_of_bounds() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, temp_update_value(CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT + 1, 25.0f), "Should return OUT_OF_BOUNDS for invalid index");
}

void test_temp_update_values_ok() {
    celsius_t values[2] = { 20.0f, 30.0f };
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_update_values(0, values, 2), "temp_update_values() failed");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 20.0f, htemp.temperatures[0], "First value mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 30.0f, htemp.temperatures[1], "Second value mismatch");
}

void test_temp_update_values_out_of_bounds() {
    celsius_t values[2] = { 20.0f, 30.0f };
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, temp_update_values(CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT, values, 1), "Should return OUT_OF_BOUNDS for invalid range");
}

void test_temp_update_discharge_value_ok() {
    // 2.0V corresponds to a specific temp in the polynomial, just checking return code here
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_update_discharge_value(0, 2.0f), "temp_update_discharge_value() failed");
}

void test_temp_update_discharge_value_out_of_bounds() {
    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OUT_OF_BOUNDS, temp_update_discharge_value(CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT + 1, 2.0f), "Should return OUT_OF_BOUNDS for invalid index");
}

void test_temp_get_values_ptr() {
    TEST_ASSERT_EQUAL_MESSAGE(&htemp.temperatures, temp_get_values(), "temp_get_values() returned incorrect pointer");
}

void test_temp_get_min() {
    temp_update_value(0, 10.0f);
    temp_update_value(1, 20.0f);
    temp_update_value(2, 30.0f);
    // Initialize others to safe max to ensure min logic works
    for (int i = 3; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; i++)
        temp_update_value(i, 60.0f);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 10.0f, temp_get_min(), "temp_get_min() failed");
}

void test_temp_get_max() {
    for (int i = 0; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; i++)
        temp_update_value(i, 0.0f);
    temp_update_value(5, 45.0f);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 45.0f, temp_get_max(), "temp_get_max() failed");
}

void test_temp_get_sum() {
    for (int i = 0; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; i++)
        temp_update_value(i, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1f, (float)CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT, temp_get_sum(), "temp_get_sum() failed");
}

void test_temp_get_avg() {
    for (int i = 0; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; i++)
        temp_update_value(i, 10.0f);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 10.0f, temp_get_avg(), "temp_get_avg() failed");
}

void test_temp_dump_values() {
    celsius_t out_buffer[2];
    temp_update_value(0, 15.0f);
    temp_update_value(1, 25.0f);

    TEST_ASSERT_EQUAL_MESSAGE(TEMP_OK, temp_dump_values(out_buffer, 0, 2), "temp_dump_values() failed");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 15.0f, out_buffer[0], "Dumped value 0 mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 25.0f, out_buffer[1], "Dumped value 1 mismatch");
}

void test_temp_get_cells_temp_canlib_payload() {
    size_t size;
    temp_update_value(0, 10.0f);
    temp_update_value(1, 11.0f);
    temp_update_value(2, 12.0f);
    temp_update_value(3, 13.0f);

    htemp.offset = 0; // Reset offset manually for deterministic test

    bms_cellboard_cells_temperature_converted_t *payload = temp_get_cells_temp_canlib_payload(&size);

    TEST_ASSERT_EQUAL_MESSAGE(sizeof(htemp.temp_can_payload), size, "Payload size mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(0, payload->offset, "Payload offset mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 10.0f, payload->temperature_0, "Payload temp 0 mismatch");
}

void test_temp_get_discharge_temp_canlib_payload() {
    size_t size;
    // Mock internal values directly or via update_discharge function
    htemp.discharge_temperatures[0] = 50.0f;

    bms_cellboard_discharge_temperature_converted_t *payload = temp_get_discharge_temp_canlib_payload(&size);

    TEST_ASSERT_EQUAL_MESSAGE(sizeof(htemp.discharge_temp_can_payload), size, "Payload size mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 50.0f, payload->temperature_0, "Payload discharge temp 0 mismatch");
}

void setUp() {
    identity_api_init(CELLBOARD_ID);
    temp_init(mock_set_address, mock_start_conversion);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_temp_init_ok);
    RUN_TEST(test_temp_init_null_pointer);
    RUN_TEST(test_temp_init_cellboard_id);
    RUN_TEST(test_temp_start_conversion_ok);
    RUN_TEST(test_temp_start_conversion_busy);
    RUN_TEST(test_temp_notify_conversion_complete);
    RUN_TEST(test_temp_update_value_ok);
    RUN_TEST(test_temp_update_value_out_of_bounds);
    RUN_TEST(test_temp_update_values_ok);
    RUN_TEST(test_temp_update_values_out_of_bounds);
    RUN_TEST(test_temp_update_discharge_value_ok);
    RUN_TEST(test_temp_update_discharge_value_out_of_bounds);
    RUN_TEST(test_temp_get_values_ptr);
    RUN_TEST(test_temp_get_min);
    RUN_TEST(test_temp_get_max);
    RUN_TEST(test_temp_get_sum);
    RUN_TEST(test_temp_get_avg);
    RUN_TEST(test_temp_dump_values);
    RUN_TEST(test_temp_get_cells_temp_canlib_payload);
    RUN_TEST(test_temp_get_discharge_temp_canlib_payload);
    return UNITY_END();
}
