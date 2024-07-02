/**
 * @file test_can-comm.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the can-comm module
 */


#include "unity.h"
#include "can-comm.h"
#include "identity.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _CanCommHandler hcan_comm;

void setUp() {
    identity_init(CELLBOARD_ID);
    can_comm_init((void*) 0x01);
}

void tearDown() {}

void test_can_comm_init_null() {
    TEST_ASSERT_EQUAL(CAN_COMM_NULL_POINTER, can_comm_init(NULL));
}

void test_can_comm_init_ok() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
}

void test_can_comm_enable_all() {
    can_comm_enable_all();
    TEST_ASSERT_EQUAL(0x03, hcan_comm.enabled);
}

void test_can_comm_disable_all() {
    hcan_comm.enabled = 0x03;
    can_comm_disable_all();
    TEST_ASSERT_EQUAL(0x00, hcan_comm.enabled);
}

void test_can_comm_is_enabled_all() {
    can_comm_enable_all();
    TEST_ASSERT_TRUE(can_comm_is_enabled_all());
}

void test_can_comm_enable() {
    can_comm_enable(0);
    can_comm_enable(1);
    TEST_ASSERT_EQUAL(0x03, hcan_comm.enabled);
}

void test_can_comm_disable() {
    can_comm_enable_all();
    can_comm_disable(0);
    can_comm_disable(1);
    TEST_ASSERT_EQUAL(0x00, hcan_comm.enabled);
}

void test_can_comm_is_enabled_false() {
    TEST_ASSERT_FALSE(can_comm_is_enabled(0));
}

void test_can_comm_is_enabled_true() {
    can_comm_enable(0);
    TEST_ASSERT_TRUE(can_comm_is_enabled(0));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_can_comm_init_null);
    RUN_TEST(test_can_comm_init_ok);
    RUN_TEST(test_can_comm_enable_all);
    RUN_TEST(test_can_comm_disable_all);
    RUN_TEST(test_can_comm_is_enabled_all);
    RUN_TEST(test_can_comm_enable);
    RUN_TEST(test_can_comm_disable);
    RUN_TEST(test_can_comm_is_enabled_false);
    RUN_TEST(test_can_comm_is_enabled_true);
    return UNITY_END();
}

