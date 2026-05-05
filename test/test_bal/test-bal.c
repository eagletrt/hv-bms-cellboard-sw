/**
 * @file test_bal.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the bal module
 */

#include "unity.h"
#include "bal.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "bms-manager-api.h"
#include "timebase.h"
#define CELLBOARD_ID CELLBOARD_ID_1

extern _BalHandler hbal;
extern struct BmsManagerHandler bms_handler;

void test_bal_init_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_init(), "bal_init() failed to return BAL_OK");
}

void test_bal_init_event_type() {
    TEST_ASSERT_EQUAL_MESSAGE(FSM_EVENT_TYPE_IGNORED, hbal.event.type, "Initial event type should be IGNORED");
}

void test_bal_init_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, hbal.status_can_payload.cellboard_id, "Cellboard ID mismatch in payload initialization");
}

void test_bal_init_target() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_TARGET_MAX_V, hbal.params.target, "Target voltage did not initialize to safe MAX_V");
}

void test_bal_init_threshold() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_THRESHOLD_MAX_V, hbal.params.threshold, "Threshold voltage did not initialize to safe MAX_V");
}

void test_bal_is_active_false() {
    TEST_ASSERT_FALSE_MESSAGE(bal_is_active(), "Module should be inactive by default");
}

void test_bal_is_active_true() {
    bal_start();
    TEST_ASSERT_TRUE_MESSAGE(bal_is_active(), "Module should be active after calling bal_start()");
}

void test_bal_is_paused_false() {
    TEST_ASSERT_FALSE_MESSAGE(bal_is_paused(), "Module should not be paused by default");
}

void test_bal_is_paused_true() {
    bal_start();
    bal_pause();
    TEST_ASSERT_TRUE_MESSAGE(bal_is_paused(), "Module should be paused after bal_start() and bal_pause()");
}

void test_bal_start_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_start(), "bal_start() failed to return BAL_OK");
}

void test_bal_stop_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_stop(), "bal_stop() failed to return BAL_OK");
}

void test_bal_stop_active() {
    bal_stop();
    TEST_ASSERT_FALSE_MESSAGE(bal_is_active(), "Module should be inactive after stop");
}

void test_bal_stop_after_start_active() {
    bal_start();
    bal_stop();
    TEST_ASSERT_FALSE_MESSAGE(bal_is_active(), "Module should be inactive after start then stop sequence");
}

void test_bal_pause_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_pause(), "bal_pause() failed to return BAL_OK");
}

void test_bal_pause_paused_false() {
    bal_pause();
    TEST_ASSERT_FALSE_MESSAGE(bal_is_paused(), "Module should not switch to paused state if not previously active");
}

void test_bal_pause_paused_true() {
    bal_start();
    bal_pause();
    TEST_ASSERT_TRUE_MESSAGE(bal_is_paused(), "Module failed to enter paused state");
}

void test_bal_resume_ok() {
    TEST_ASSERT_EQUAL_MESSAGE(BAL_OK, bal_resume(), "bal_resume() failed to return BAL_OK");
}

void test_bal_resume_paused_false() {
    bal_start();
    bal_pause();
    bal_resume();
    TEST_ASSERT_FALSE_MESSAGE(bal_is_paused(), "Module failed to exit paused state after resume");
}

void test_bal_get_canlib_payload_payload() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL_MESSAGE(&hbal.status_can_payload, payload, "Returned payload pointer does not match internal handler");
}

void test_bal_get_canlib_payload_byte_size() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL_MESSAGE(sizeof(hbal.status_can_payload), byte_size, "Returned payload size mismatch");
}

void test_bal_get_canlib_payload_content() {

    uint32_t cells = bms_manager_get_discharge_cells();

    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_status_canlib_payload(&byte_size);

    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_0, CELLBOARD_BIT_GET(cells, 0U), "Discharging cell 0 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_1, CELLBOARD_BIT_GET(cells, 1U), "Discharging cell 1 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_2, CELLBOARD_BIT_GET(cells, 2U), "Discharging cell 2 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_3, CELLBOARD_BIT_GET(cells, 3U), "Discharging cell 3 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_4, CELLBOARD_BIT_GET(cells, 4U), "Discharging cell 4 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_5, CELLBOARD_BIT_GET(cells, 5U), "Discharging cell 5 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_6, CELLBOARD_BIT_GET(cells, 6U), "Discharging cell 6 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_7, CELLBOARD_BIT_GET(cells, 7U), "Discharging cell 7 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_8, CELLBOARD_BIT_GET(cells, 8U), "Discharging cell 8 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_9, CELLBOARD_BIT_GET(cells, 9U), "Discharging cell 9 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_10, CELLBOARD_BIT_GET(cells, 10U), "Discharging cell 10 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_11, CELLBOARD_BIT_GET(cells, 11U), "Discharging cell 11 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_12, CELLBOARD_BIT_GET(cells, 12U), "Discharging cell 12 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_13, CELLBOARD_BIT_GET(cells, 13U), "Discharging cell 13 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_14, CELLBOARD_BIT_GET(cells, 14U), "Discharging cell 14 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_15, CELLBOARD_BIT_GET(cells, 15U), "Discharging cell 15 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_16, CELLBOARD_BIT_GET(cells, 16U), "Discharging cell 16 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_17, CELLBOARD_BIT_GET(cells, 17U), "Discharging cell 17 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_18, CELLBOARD_BIT_GET(cells, 18U), "Discharging cell 18 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_19, CELLBOARD_BIT_GET(cells, 19U), "Discharging cell 19 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_20, CELLBOARD_BIT_GET(cells, 20U), "Discharging cell 20 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_21, CELLBOARD_BIT_GET(cells, 21U), "Discharging cell 21 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_22, CELLBOARD_BIT_GET(cells, 22U), "Discharging cell 22 status mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(hbal.status_can_payload.discharging_cell_23, CELLBOARD_BIT_GET(cells, 23U), "Discharging cell 23 status mismatch");
}

void test_bal_set_balancing_status_handle_target() {
    bms_cellboard_set_balancing_status_converted_t payload;
    payload.start = true;
    payload.target = BAL_TARGET_MIN_V - 1.0f;

    bal_set_balancing_status_handle(&payload);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(
        0.001f,
        BAL_TARGET_MIN_V,
        hbal.params.target,
        "Target voltage should be clamped to BAL_TARGET_MIN_V when input is too low");
}

void test_bal_set_balancing_status_handle_threshold() {
    bms_cellboard_set_balancing_status_converted_t payload;
    payload.start = true;
    payload.threshold = BAL_THRESHOLD_MIN_V - 1;
    bal_set_balancing_status_handle(&payload);

    float delta = (BAL_THRESHOLD_MIN_V + BAL_THRESHOLD_MAX_V) / 2;

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(delta, BAL_THRESHOLD_MIN_V + delta, hbal.params.threshold, "Threshold voltage was not correctly clamped/validated");
}

void test_bal_set_balancing_status_handle_event() {
    bms_cellboard_set_balancing_status_converted_t payload;
    payload.start = true;
    bal_set_balancing_status_handle(&payload);

    TEST_ASSERT_EQUAL_MESSAGE(FSM_EVENT_TYPE_BALANCING_START, hbal.event.type, "Event type was not updated to BALANCING_START");
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
    RUN_TEST(test_bal_init_event_type);
    RUN_TEST(test_bal_init_cellboard_id);
    RUN_TEST(test_bal_init_target);
    RUN_TEST(test_bal_init_threshold);
    RUN_TEST(test_bal_is_active_false);
    RUN_TEST(test_bal_is_active_true);
    RUN_TEST(test_bal_is_paused_false);
    RUN_TEST(test_bal_is_paused_true);
    RUN_TEST(test_bal_start_ok);
    RUN_TEST(test_bal_stop_ok);
    RUN_TEST(test_bal_stop_active);
    RUN_TEST(test_bal_stop_after_start_active);
    RUN_TEST(test_bal_pause_ok);
    RUN_TEST(test_bal_pause_paused_false);
    RUN_TEST(test_bal_pause_paused_true);
    RUN_TEST(test_bal_resume_ok);
    RUN_TEST(test_bal_resume_paused_false);
    RUN_TEST(test_bal_get_canlib_payload_payload);
    RUN_TEST(test_bal_get_canlib_payload_byte_size);
    RUN_TEST(test_bal_get_canlib_payload_content);
    RUN_TEST(test_bal_set_balancing_status_handle_target);
    RUN_TEST(test_bal_set_balancing_status_handle_threshold);
    RUN_TEST(test_bal_set_balancing_status_handle_event);
    return UNITY_END();
}
