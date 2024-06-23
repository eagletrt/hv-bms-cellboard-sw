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

void setUp() {}

void tearDown() {}

void test_bal_init() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(FSM_EVENT_TYPE_IGNORED, hbal.event.type);
    TEST_ASSERT_EQUAL(CELLBOARD_ID, hbal.can_payload.cellboard_id);
    TEST_ASSERT_EQUAL(BAL_TARGET_MAX, hbal.params.target);
    TEST_ASSERT_EQUAL(BAL_THRESHOLD_MAX, hbal.params.threshold);
}

void test_bal_set_balancing_status_handle() {}

void test_bal_is_active() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(false, bal_is_active());
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_EQUAL(true, bal_is_active());
}

void test_bal_is_paused() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_FALSE(bal_is_paused());
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_EQUAL(BAL_OK, bal_pause());
    TEST_ASSERT_TRUE(bal_is_paused());
}

void test_bal_start() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_TRUE(hbal.active);
}

void test_bal_stop() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(BAL_OK, bal_stop());
    TEST_ASSERT_FALSE(hbal.active);
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_EQUAL(BAL_OK, bal_stop());
    TEST_ASSERT_FALSE(hbal.active);
}

void test_bal_pause() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(BAL_OK, bal_pause());
    TEST_ASSERT_FALSE(hbal.paused);
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_EQUAL(BAL_OK, bal_pause());
    TEST_ASSERT_TRUE(hbal.paused);
}

void test_bal_resume() {
    TEST_ASSERT_EQUAL(BAL_OK, bal_init());
    TEST_ASSERT_EQUAL(BAL_OK, bal_resume());
    TEST_ASSERT_EQUAL(BAL_OK, bal_start());
    TEST_ASSERT_EQUAL(BAL_OK, bal_pause());
    TEST_ASSERT_TRUE(hbal.paused);
    TEST_ASSERT_EQUAL(BAL_OK, bal_resume());
    TEST_ASSERT_FALSE(hbal.paused);
}

void test_bal_get_canlib_payload() {}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_bal_init);
    RUN_TEST(test_bal_set_balancing_status_handle);
    RUN_TEST(test_bal_is_active);
    RUN_TEST(test_bal_is_paused);
    RUN_TEST(test_bal_start);
    RUN_TEST(test_bal_stop);
    RUN_TEST(test_bal_pause);
    RUN_TEST(test_bal_resume);
    return UNITY_END();
}

