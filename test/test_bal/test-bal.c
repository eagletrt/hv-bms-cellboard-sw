/**
 * @file test_bal.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the bal module
 */

#include "unity.h"
#include "bal-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "bms-manager.h"
#include "timebase.h"
#include "eagletrt-api.h"
#define CELLBOARD_ID CELLBOARD_ID_1

extern struct BalHandler balancing_handler;
extern _BmsManagerHandler hmanager;

void test_bal_init_ok() {

    struct BalHandler expected_handler = {
        .event.type = FSM_EVENT_TYPE_IGNORED,
        .status_can_payload.cellboard_id = CELLBOARD_ID,
        .params.target = BAL_TARGET_MAX_V,
        .params.threshold = BAL_THRESHOLD_MAX_V,
        .status = BAL_STATUS_STOPPED
    };

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_init(), "bal_init() failed to return BAL_OK");

    expected_handler.watchdog = balancing_handler.watchdog; // Watchdog is initialized in bal_init, so we need to set it in the expected handler for a proper comparison

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &balancing_handler, sizeof(expected_handler), "bal_init() did not properly initialize the balancing handler");
}

void test_bal_set_balancing_status_handle_null_payload() {
    // Set initial status to active to check that it does not change
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_set_balancing_status_handle(NULL);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_NULL_POINTER, result, "bal_set_balancing_status_handle() should return BAL_NULL_POINTER when given a NULL payload");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_set_balancing_status_handle() should not change the status when given a NULL payload");
}

void test_bal_set_balancing_status_handle_stop_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bms_cellboard_set_balancing_status_converted_t payload = {
        .start = false,
        .target = BAL_TARGET_MAX_V,
        .threshold = BAL_THRESHOLD_MAX_V
    };

    enum BalReturnCode result = bal_set_balancing_status_handle(&payload);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_set_balancing_status_handle() should return BAL_OK when given a stop command while inactive");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_set_balancing_status_handle() should not change the status when given a stop command while inactive");
}

void test_bal_set_balancing_status_handle_watchdog_error() {
    // Simulate watchdog error by setting an invalid timeout value
    balancing_handler.watchdog.timed_out = true;

    bms_cellboard_set_balancing_status_converted_t payload = {
        .start = true,
        .target = BAL_TARGET_MAX_V,
        .threshold = BAL_THRESHOLD_MAX_V
    };

    enum BalReturnCode result = bal_set_balancing_status_handle(&payload);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_WATCHDOG_ERROR, result, "bal_set_balancing_status_handle() should return BAL_WATCHDOG_ERROR when there is a watchdog error");
}

void test_bal_set_balancing_status_handle_ok() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bms_cellboard_set_balancing_status_converted_t payload = {
        .start = true,
        .target = BAL_TARGET_MAX_V,
        .threshold = BAL_THRESHOLD_MAX_V,
    };

    enum BalReturnCode result = bal_set_balancing_status_handle(&payload);

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_set_balancing_status_handle() should return BAL_OK when given a valid payload");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_TARGET_MAX_V, balancing_handler.params.target, "bal_set_balancing_status_handle() should set the target voltage correctly");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_THRESHOLD_MAX_V, balancing_handler.params.threshold, "bal_set_balancing_status_handle() should set the threshold voltage correctly");
}

void test_bal_set_balancing_status_handle_target_out_of_range() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bms_cellboard_set_balancing_status_converted_t payload = {
        .start = true,
        .target = BAL_TARGET_MAX_V + 1.0f, // Out of range target
        .threshold = BAL_THRESHOLD_MAX_V
    };

    enum BalReturnCode result = bal_set_balancing_status_handle(&payload);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_set_balancing_status_handle() should return BAL_OK even if the target is out of range");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_TARGET_MAX_V, balancing_handler.params.target, "bal_set_balancing_status_handle() should clamp the target to BAL_TARGET_MAX_V when given an out of range value");
}

void test_bal_set_balancing_status_handle_threshold_out_of_range() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bms_cellboard_set_balancing_status_converted_t payload = {
        .start = true,
        .target = BAL_TARGET_MAX_V,
        .threshold = BAL_THRESHOLD_MAX_V + 0.1f // Out of range threshold
    };

    enum BalReturnCode result = bal_set_balancing_status_handle(&payload);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_set_balancing_status_handle() should return BAL_OK even if the threshold is out of range");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_THRESHOLD_MAX_V, balancing_handler.params.threshold, "bal_set_balancing_status_handle() should clamp the threshold to BAL_THRESHOLD_MAX_V when given an out of range value");
}

void test_bal_start_while_active() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_start();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_start() should return BAL_OK when called while already active");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_start() should not change the status when called while already active");
}

void test_bal_start_ok() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_start();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_start() should return BAL_OK when starting from an inactive state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_start() should set the status to BAL_STATUS_DISCHARCING when started");
    TEST_ASSERT_EQUAL_MESSAGE(volt_select_values_above_target(balancing_handler.params.target + balancing_handler.params.threshold), bms_manager_get_discharge_cells(), "bal_start() should set the correct discharge cells based on the target and threshold");
}

void test_bal_stop_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_stop();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_stop() should return BAL_OK when called while already inactive");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_stop() should not change the status when called while already inactive");
}

void test_bal_stop_ok() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_stop();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_stop() should return BAL_OK when stopping from an active state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_stop() should set the status to BAL_STATUS_STOPPED when stopped");
    TEST_ASSERT_EQUAL_MESSAGE(0U, bms_manager_get_discharge_cells(), "bal_stop() should set all discharge cells to 0 when stopped");
}

void test_bal_pause_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_pause();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when called while already inactive");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_pause() should not change the status when called while already inactive");
}

void test_bal_pause_while_paused() {
    // Set initial status to paused
    balancing_handler.status = BAL_STATUS_PAUSED;

    enum BalReturnCode result = bal_pause();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when called while already paused");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_PAUSED, balancing_handler.status, "bal_pause() should not change the status when called while already paused");
}

void test_bal_pause_ok() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_pause();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when pausing from an active state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_PAUSED, balancing_handler.status, "bal_pause() should set the status to BAL_STATUS_PAUSED when paused");
    TEST_ASSERT_EQUAL_MESSAGE(0U, bms_manager_get_discharge_cells(), "bal_pause() should set all discharge cells to 0 when paused");
}

void test_bal_resume_while_active() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_resume();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when called while already active");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_resume() should not change the status when called while already active");
}

void test_bal_resume_while_not_paused() {
    // Set initial status to stopped
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_resume();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when called while not paused");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_resume() should not change the status when called while not paused");
}

void test_bal_resume_ok() {
    // Set initial status to paused
    balancing_handler.status = BAL_STATUS_PAUSED;

    enum BalReturnCode result = bal_resume();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when resuming from a paused state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_resume() should set the status to BAL_STATUS_DISCHARCING when resumed");
    TEST_ASSERT_EQUAL_MESSAGE(volt_select_values_above_target(balancing_handler.params.target + balancing_handler.params.threshold), bms_manager_get_discharge_cells(), "bal_resume() should set the correct discharge cells based on the target and threshold when resumed");
}

void test_bal_get_canlib_payload_payload() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL_MESSAGE(&balancing_handler.status_can_payload, payload, "Returned payload pointer does not match internal handler");
}

void test_bal_get_canlib_payload_byte_size() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL_MESSAGE(sizeof(balancing_handler.status_can_payload), byte_size, "Returned payload size mismatch");
}

void test_bal_get_canlib_payload_content() {

    uint32_t cells = bms_manager_get_discharge_cells();

    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_0, EAGLETRT_API_BIT_GET(cells, 0U), "Discharging cell 0 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_1, EAGLETRT_API_BIT_GET(cells, 1U), "Discharging cell 1 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_2, EAGLETRT_API_BIT_GET(cells, 2U), "Discharging cell 2 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_3, EAGLETRT_API_BIT_GET(cells, 3U), "Discharging cell 3 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_4, EAGLETRT_API_BIT_GET(cells, 4U), "Discharging cell 4 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_5, EAGLETRT_API_BIT_GET(cells, 5U), "Discharging cell 5 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_6, EAGLETRT_API_BIT_GET(cells, 6U), "Discharging cell 6 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_7, EAGLETRT_API_BIT_GET(cells, 7U), "Discharging cell 7 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_8, EAGLETRT_API_BIT_GET(cells, 8U), "Discharging cell 8 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_9, EAGLETRT_API_BIT_GET(cells, 9U), "Discharging cell 9 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_10, EAGLETRT_API_BIT_GET(cells, 10U), "Discharging cell 10 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_11, EAGLETRT_API_BIT_GET(cells, 11U), "Discharging cell 11 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_12, EAGLETRT_API_BIT_GET(cells, 12U), "Discharging cell 12 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_13, EAGLETRT_API_BIT_GET(cells, 13U), "Discharging cell 13 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_14, EAGLETRT_API_BIT_GET(cells, 14U), "Discharging cell 14 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_15, EAGLETRT_API_BIT_GET(cells, 15U), "Discharging cell 15 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_16, EAGLETRT_API_BIT_GET(cells, 16U), "Discharging cell 16 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_17, EAGLETRT_API_BIT_GET(cells, 17U), "Discharging cell 17 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_18, EAGLETRT_API_BIT_GET(cells, 18U), "Discharging cell 18 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_19, EAGLETRT_API_BIT_GET(cells, 19U), "Discharging cell 19 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_20, EAGLETRT_API_BIT_GET(cells, 20U), "Discharging cell 20 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_21, EAGLETRT_API_BIT_GET(cells, 21U), "Discharging cell 21 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_22, EAGLETRT_API_BIT_GET(cells, 22U), "Discharging cell 22 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(balancing_handler.status_can_payload.discharging_cell_23, EAGLETRT_API_BIT_GET(cells, 23U), "Discharging cell 23 status mismatch");
}

void setUp() {

    identity_api_init(CELLBOARD_ID);
    timebase_init(500U);
    bal_init();
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bal_init_ok);
    RUN_TEST(test_bal_set_balancing_status_handle_null_payload);
    RUN_TEST(test_bal_set_balancing_status_handle_stop_while_inactive);
    RUN_TEST(test_bal_set_balancing_status_handle_watchdog_error);
    RUN_TEST(test_bal_set_balancing_status_handle_ok);
    RUN_TEST(test_bal_set_balancing_status_handle_target_out_of_range);
    RUN_TEST(test_bal_set_balancing_status_handle_threshold_out_of_range);
    RUN_TEST(test_bal_start_while_active);
    RUN_TEST(test_bal_start_ok);
    RUN_TEST(test_bal_stop_while_inactive);
    RUN_TEST(test_bal_stop_ok);
    RUN_TEST(test_bal_pause_while_inactive);
    RUN_TEST(test_bal_pause_while_paused);
    RUN_TEST(test_bal_pause_ok);
    RUN_TEST(test_bal_resume_while_active);
    RUN_TEST(test_bal_resume_while_not_paused);
    RUN_TEST(test_bal_resume_ok);

    RUN_TEST(test_bal_get_canlib_payload_payload);
    RUN_TEST(test_bal_get_canlib_payload_byte_size);
    RUN_TEST(test_bal_get_canlib_payload_content);
    return UNITY_END();
}
