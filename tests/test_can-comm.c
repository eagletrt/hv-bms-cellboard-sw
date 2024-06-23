#include "unity.h"
#include "can-comm.h"
#include "identity.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _CanCommHandler hcan_comm;

void setUp() {}

void tearDown() {}

void test_can_comm_init() {
    TEST_ASSERT_EQUAL(CAN_COMM_NULL_POINTER, can_comm_init(NULL));
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
}

void test_can_comm_enable_all() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    can_comm_enable_all();
    TEST_ASSERT_EQUAL(0x03, hcan_comm.enabled);
}

void test_can_comm_disable_all() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    hcan_comm.enabled = 0x03;
    can_comm_disable_all();
    TEST_ASSERT_EQUAL(0x00, hcan_comm.enabled);
}

void test_can_comm_is_enabled_all() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    can_comm_enable_all();
    TEST_ASSERT_TRUE(can_comm_is_enabled_all());
}

void test_can_comm_enable() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    can_comm_enable(0);
    TEST_ASSERT_EQUAL(0x01, hcan_comm.enabled);
    can_comm_enable(1);
    TEST_ASSERT_EQUAL(0x03, hcan_comm.enabled);
}

void test_can_comm_disable() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    can_comm_enable_all();
    can_comm_disable(0);
    TEST_ASSERT_EQUAL(0x02, hcan_comm.enabled);
    can_comm_disable(1);
    TEST_ASSERT_EQUAL(0x00, hcan_comm.enabled);
}

void test_can_comm_is_enabled() {
    TEST_ASSERT_EQUAL(CAN_COMM_OK, can_comm_init((void*) 0x01));
    TEST_ASSERT_FALSE(can_comm_is_enabled(0));
    TEST_ASSERT_FALSE(can_comm_is_enabled(1));
    can_comm_enable_all();
    TEST_ASSERT_TRUE(can_comm_is_enabled(0));
    TEST_ASSERT_TRUE(can_comm_is_enabled(1));
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_can_comm_init);
    RUN_TEST(test_can_comm_enable_all);
    RUN_TEST(test_can_comm_disable_all);
    RUN_TEST(test_can_comm_is_enabled_all);
    RUN_TEST(test_can_comm_enable);
    RUN_TEST(test_can_comm_disable);
    RUN_TEST(test_can_comm_is_enabled);
    return UNITY_END();
}

