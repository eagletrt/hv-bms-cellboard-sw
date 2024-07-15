/**
 * @file test_bal.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the bal module
 */

#include "unity.h"
#include "bal.h"
#include "identity.h"
#include "cellboard-def.h"
#include "bms-manager.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _BalHandler hbal;
extern _BmsManagerHandler hmanager;

void setUp() {
    identity_init(CELLBOARD_ID);
    bal_init();
}

void tearDown() {}

void test_bal_init_ok() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
}

void test_bal_init_event_type() {
    TEST_ASSERT_EQUAL(FSM_EVENT_TYPE_IGNORED, hbal.event.type);
}

void test_bal_init_cellboard_id() {
    TEST_ASSERT_EQUAL(CELLBOARD_ID, hbal.can_payload.cellboard_id);
}

void test_bal_init_target() {
    TEST_ASSERT_EQUAL(BAL_TARGET_MAX, hbal.params.target);
}

void test_bal_init_threshold() {
    TEST_ASSERT_EQUAL(BAL_THRESHOLD_MAX, hbal.params.threshold);
}

void test_bal_is_active_false() {
    TEST_ASSERT_FALSE(bal_is_active());
}

void test_bal_is_active_true() {
    bal_start();
    TEST_ASSERT_TRUE(bal_is_active());
}

void test_bal_is_paused_false() {
    TEST_ASSERT_FALSE(bal_is_paused());
}

void test_bal_is_paused_true() {
    bal_start();
    bal_pause();
    TEST_ASSERT_TRUE(bal_is_paused());
}

void test_bal_start_ok() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
}

void test_bal_start_active() {
    bal_start();
    TEST_ASSERT_TRUE(hbal.active);
}

void test_bal_stop_ok() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_stop());
}

void test_bal_stop_active() {
    bal_stop();
    TEST_ASSERT_FALSE(hbal.active);
}

void test_bal_stop_after_start_active() {
    bal_start();
    bal_stop();
    TEST_ASSERT_FALSE(hbal.active);
}

void test_bal_pause_ok() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_pause());
}

void test_bal_pause_paused_false() {
    bal_pause();
    TEST_ASSERT_FALSE(hbal.paused);
}

void test_bal_pause_paused_true() {
    bal_start();
    bal_pause();
    TEST_ASSERT_TRUE(hbal.paused);
}

void test_bal_resume_ok() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_resume());
}

void test_bal_resume_paused_false() {
    bal_start();
    bal_pause();
    bal_resume();
    TEST_ASSERT_FALSE(hbal.paused);
}

void test_bal_get_canlib_payload_payload() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL(&hbal.can_payload, payload);
}

void test_bal_get_canlib_payload_byte_size() {
    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_canlib_payload(&byte_size);
    TEST_ASSERT_EQUAL(sizeof(hbal.can_payload), byte_size);
}

void test_bal_get_canlib_payload_content() {

    uint32_t cells = bms_manager_get_discharge_cells();

    size_t byte_size;
    bms_cellboard_balancing_status_converted_t *payload = bal_get_canlib_payload(&byte_size);
    
    // Check if every cell is set correctly
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_0, CELLBOARD_BIT_GET(cells, 0U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_1, CELLBOARD_BIT_GET(cells, 1U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_2, CELLBOARD_BIT_GET(cells, 2U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_3, CELLBOARD_BIT_GET(cells, 3U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_4, CELLBOARD_BIT_GET(cells, 4U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_5, CELLBOARD_BIT_GET(cells, 5U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_6, CELLBOARD_BIT_GET(cells, 6U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_7, CELLBOARD_BIT_GET(cells, 7U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_8, CELLBOARD_BIT_GET(cells, 8U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_9, CELLBOARD_BIT_GET(cells, 9U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_10, CELLBOARD_BIT_GET(cells, 10U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_11, CELLBOARD_BIT_GET(cells, 11U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_12, CELLBOARD_BIT_GET(cells, 12U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_13, CELLBOARD_BIT_GET(cells, 13U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_14, CELLBOARD_BIT_GET(cells, 14U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_15, CELLBOARD_BIT_GET(cells, 15U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_16, CELLBOARD_BIT_GET(cells, 16U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_17, CELLBOARD_BIT_GET(cells, 17U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_18, CELLBOARD_BIT_GET(cells, 18U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_19, CELLBOARD_BIT_GET(cells, 19U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_20, CELLBOARD_BIT_GET(cells, 20U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_21, CELLBOARD_BIT_GET(cells, 21U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_22, CELLBOARD_BIT_GET(cells, 22U));
    TEST_ASSERT_EQUAL(hbal.can_payload.discharging_cell_23, CELLBOARD_BIT_GET(cells, 23U));
}

void test_bal_set_balancing_status_handle_target() {
    bms_cellboard_set_balancing_status_converted_t  payload;
    payload.start = true;
    payload.target = BAL_TARGET_MIN-1;
    bal_set_balancing_status_handle(&payload);

    float delta = (BAL_TARGET_MIN+BAL_TARGET_MAX)/2;
    TEST_ASSERT_FLOAT_WITHIN(delta, BAL_TARGET_MIN+delta, hbal.params.target);
}

void test_bal_set_balancing_status_handle_threshold() {
    bms_cellboard_set_balancing_status_converted_t  payload;
    payload.start = true;
    payload.threshold = BAL_THRESHOLD_MIN-1;
    bal_set_balancing_status_handle(&payload);

    float delta = (BAL_TARGET_MIN+BAL_TARGET_MAX)/2;

    TEST_ASSERT_FLOAT_WITHIN(delta, BAL_THRESHOLD_MIN+delta, hbal.params.threshold);
}

void test_bal_set_balancing_status_handle_event() {
    bms_cellboard_set_balancing_status_converted_t  payload;
    payload.start = true;
    bal_set_balancing_status_handle(&payload);

    TEST_ASSERT_EQUAL(FSM_EVENT_TYPE_BALANCING_START, hbal.event.type);
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
    RUN_TEST(test_bal_start_active);
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

