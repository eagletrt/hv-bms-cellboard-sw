/*!
 * \file test_bal.c
 * \date 2024-06-23
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the bal module
 */

#include "unity.h"

#include <stdint.h>

#include "bal-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "volt-api.h"
#include "timebase.h"
#include "eagletrt-api.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern struct BmsManagerHandler bms_handler;
extern struct BalHandler balancing_handler;

void test_bal_init_ok() {

    struct BalHandler expected_handler = {
        .event.type = FSM_EVENT_TYPE_IGNORED,
        .params.target = BAL_TARGET_MAX_V,
        .params.threshold = BAL_THRESHOLD_MAX_V,
        .status = BAL_STATUS_STOPPED
    };

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_api_init(), "bal_init() failed to return BAL_OK");

    expected_handler.watchdog = balancing_handler.watchdog; // Watchdog is initialized in bal_init, so we need to set it in the expected handler for a proper comparison

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected_handler, &balancing_handler, sizeof(expected_handler), "bal_init() did not properly initialize the balancing handler");
}

void test_bal_set_balancing_status_handle_stop_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bal_api_balancing_set_handle(false, BAL_TARGET_MAX_V, BAL_THRESHOLD_MAX_V);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_balancing_set_handle() should not change the status when given a stop command while inactive");
}

void test_bal_set_balancing_status_handle_ok() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bal_api_balancing_set_handle(true, BAL_TARGET_MAX_V, BAL_THRESHOLD_MAX_V);

    TEST_ASSERT_EQUAL_MESSAGE(BAL_TARGET_MAX_V, balancing_handler.params.target, "bal_balancing_set_handle() should set the target voltage correctly");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_THRESHOLD_MAX_V, balancing_handler.params.threshold, "bal_balancing_set_handle() should set the threshold voltage correctly");
}

void test_bal_set_balancing_status_handle_target_out_of_range() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bal_api_balancing_set_handle(true, BAL_TARGET_MAX_V + 1.0f, BAL_THRESHOLD_MAX_V);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_TARGET_MAX_V, balancing_handler.params.target, "bal_balancing_set_handle() should clamp the target to BAL_TARGET_MAX_V when given an out of range value");
}

void test_bal_set_balancing_status_handle_threshold_out_of_range() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    bal_api_balancing_set_handle(true, BAL_TARGET_MAX_V, BAL_THRESHOLD_MAX_V + 0.1f);
    TEST_ASSERT_EQUAL_MESSAGE(BAL_THRESHOLD_MAX_V, balancing_handler.params.threshold, "bal_balancing_set_handle() should clamp the threshold to BAL_THRESHOLD_MAX_V when given an out of range value");
}

void test_bal_start_while_active() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_api_start();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_start() should return BAL_OK when called while already active");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_start() should not change the status when called while already active");
}

void test_bal_start_ok() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_api_start();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_start() should return BAL_OK when starting from an inactive state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_start() should set the status to BAL_STATUS_DISCHARCING when started");
    TEST_ASSERT_EQUAL_MESSAGE(volt_api_select_values_above_target(balancing_handler.params.target + balancing_handler.params.threshold), bms_manager_api_get_discharge_cells(), "bal_start() should set the correct discharge cells based on the target and threshold");
}

void test_bal_stop_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_api_stop();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_stop() should return BAL_OK when called while already inactive");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_stop() should not change the status when called while already inactive");
}

void test_bal_stop_ok() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_api_stop();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_stop() should return BAL_OK when stopping from an active state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_stop() should set the status to BAL_STATUS_STOPPED when stopped");
    TEST_ASSERT_EQUAL_MESSAGE(0U, bms_manager_api_get_discharge_cells(), "bal_stop() should set all discharge cells to 0 when stopped");
}

void test_bal_pause_while_inactive() {
    // Set initial status to inactive
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_api_pause();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when called while already inactive");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_pause() should not change the status when called while already inactive");
}

void test_bal_pause_while_paused() {
    // Set initial status to paused
    balancing_handler.status = BAL_STATUS_PAUSED;

    enum BalReturnCode result = bal_api_pause();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when called while already paused");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_PAUSED, balancing_handler.status, "bal_pause() should not change the status when called while already paused");
}

void test_bal_pause_ok() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_api_pause();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_pause() should return BAL_OK when pausing from an active state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_PAUSED, balancing_handler.status, "bal_pause() should set the status to BAL_STATUS_PAUSED when paused");
    TEST_ASSERT_EQUAL_MESSAGE(0U, bms_manager_api_get_discharge_cells(), "bal_pause() should set all discharge cells to 0 when paused");
}

void test_bal_resume_while_active() {
    // Set initial status to active
    balancing_handler.status = BAL_STATUS_DISCHARCING;

    enum BalReturnCode result = bal_api_resume();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when called while already active");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_resume() should not change the status when called while already active");
}

void test_bal_resume_while_not_paused() {
    // Set initial status to stopped
    balancing_handler.status = BAL_STATUS_STOPPED;

    enum BalReturnCode result = bal_api_resume();
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when called while not paused");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_STOPPED, balancing_handler.status, "bal_resume() should not change the status when called while not paused");
}

void test_bal_resume_ok() {
    // Set initial status to paused
    balancing_handler.status = BAL_STATUS_PAUSED;

    enum BalReturnCode result = bal_api_resume();

    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, result, "bal_resume() should return BAL_OK when resuming from a paused state");
    TEST_ASSERT_EQUAL_MESSAGE(BAL_STATUS_DISCHARCING, balancing_handler.status, "bal_resume() should set the status to BAL_STATUS_DISCHARCING when resumed");
    TEST_ASSERT_EQUAL_MESSAGE(volt_api_select_values_above_target(balancing_handler.params.target + balancing_handler.params.threshold), bms_manager_api_get_discharge_cells(), "bal_resume() should set the correct discharge cells based on the target and threshold when resumed");
}

void test_bal_is_active() {
    balancing_handler.status = BAL_STATUS_STOPPED;
    TEST_ASSERT_FALSE_MESSAGE(bal_api_is_active(), "bal_is_active() should return false when stopped");

    balancing_handler.status = BAL_STATUS_PAUSED;
    TEST_ASSERT_TRUE_MESSAGE(bal_api_is_active(), "bal_is_active() should return true when paused");

    balancing_handler.status = BAL_STATUS_DISCHARCING;
    TEST_ASSERT_TRUE_MESSAGE(bal_api_is_active(), "bal_is_active() should return true when discharging");
}

void test_bal_is_paused() {
    balancing_handler.status = BAL_STATUS_DISCHARCING;
    TEST_ASSERT_FALSE_MESSAGE(bal_api_is_paused(), "bal_is_paused() should return false when discharging");

    balancing_handler.status = BAL_STATUS_PAUSED;
    TEST_ASSERT_TRUE_MESSAGE(bal_api_is_paused(), "bal_is_paused() should return true when paused");
}

void test_bal_get_canlib_payload_payload() {
    union CanBmsMessages *payload = bal_api_get_canlib_payload(NULL);
    TEST_ASSERT_EQUAL_MESSAGE(&balancing_handler.libcan_message_balancing, payload, "Returned payload pointer does not match internal handler");
}

void test_bal_get_canlib_payload_byte_size() {
    size_t byte_size;
    bal_api_get_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL_MESSAGE(can_bms_byte_size_tsaccellboard2balancing, byte_size, "Returned payload size mismatch");
}

void test_bal_get_canlib_payload_content() {
    uint32_t cells = bms_manager_api_get_discharge_cells();

    union CanBmsMessages *payload = bal_api_get_canlib_payload(NULL);

    TEST_ASSERT_EQUAL_MESSAGE(EAGLETRT_API_BIT_GET(cells, 0U), payload->tsaccellboard2balancing.cell1, "Discharging cell 1 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(EAGLETRT_API_BIT_GET(cells, 1U), payload->tsaccellboard2balancing.cell2, "Discharging cell 2 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(EAGLETRT_API_BIT_GET(cells, 22U), payload->tsaccellboard2balancing.cell23, "Discharging cell 23 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(EAGLETRT_API_BIT_GET(cells, 23U), payload->tsaccellboard2balancing.cell24, "Discharging cell 24 status mismatch");
}

void setUp() {

    identity_api_init(CELLBOARD_ID);
    timebase_init(500U);
    bal_api_init();
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bal_init_ok);
    RUN_TEST(test_bal_set_balancing_status_handle_stop_while_inactive);
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

    RUN_TEST(test_bal_is_active);
    RUN_TEST(test_bal_is_paused);

    RUN_TEST(test_bal_get_canlib_payload_payload);
    RUN_TEST(test_bal_get_canlib_payload_byte_size);
    RUN_TEST(test_bal_get_canlib_payload_content);
    return UNITY_END();
}
