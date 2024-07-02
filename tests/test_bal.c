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

#define CELLBOARD_ID CELLBOARD_ID_1

extern _BalHandler hbal;

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

// TODO
void test_bal_get_canlib_payload() {}

// TODO
void test_bal_set_balancing_status_handle() {}

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
    return UNITY_END();
}

