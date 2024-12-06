/**
 * @file test_programmer.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the programmer module
 */

#include "unity.h"
#include "programmer.h"
#include "identity.h"
#include "cellboard-def.h"
#include "fsm.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _ProgrammerHandler hprogrammer;

bool reset_called = false;
void reset() {
    reset_called = true;
}

void setUp() {
    programmer_init(reset);
    identity_init(CELLBOARD_ID);

    reset_called = false;
}

void tearDown() {}

void test_programmer_init_ok() {
    TEST_ASSERT_EQUAL(PROGRAMMER_OK, programmer_init(NULL));
}

void test_programmer_init_reset() {
    TEST_ASSERT_EQUAL(reset, hprogrammer.reset);
}

void test_programmer_init_flash_event_type() {
    TEST_ASSERT_EQUAL(FSM_EVENT_TYPE_FLASH_REQUEST, hprogrammer.flash_event.type);
}

void test_programmer_init_can_payload_cellboard_id() {
    TEST_ASSERT_EQUAL(CELLBOARD_ID, hprogrammer.can_payload.cellboard_id);
}

void test_programmer_init_can_payload_ready() {
    TEST_ASSERT_TRUE(hprogrammer.can_payload.ready);
}

void test_programmer_init_target() {
    TEST_ASSERT_EQUAL(MAINBOARD_ID, hprogrammer.target);
}

void test_programmer_routine_called() {
    hprogrammer.flashing = true;
    hprogrammer.target = identity_get_cellboard_id();
    programmer_routine();
    TEST_ASSERT_TRUE(reset_called);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_programmer_init_ok);
    RUN_TEST(test_programmer_init_reset);
    RUN_TEST(test_programmer_init_flash_event_type);
    RUN_TEST(test_programmer_init_can_payload_cellboard_id);
    RUN_TEST(test_programmer_init_can_payload_ready);
    RUN_TEST(test_programmer_init_target);
    RUN_TEST(test_programmer_routine_called);
    return UNITY_END();
}
