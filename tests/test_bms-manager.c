/**
 * @file test_bms-manager.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the bms-manager module
 */

#include "unity.h"
#include "bms-manager.h"
#include "identity.h"
#include "cellboard-def.h"
#include "ltc6811.h"

#define CELLBOARD_ID CELLBOARD_ID_1

void send(uint8_t *data, size_t size) {
    // Do nothing
}

void send_receive(uint8_t *data, uint8_t *out, size_t size, size_t size_out) {
    // Do nothing
}

extern _BmsManagerHandler hmanager;

void setUp() {
    identity_init(CELLBOARD_ID);
    bms_manager_init(send, send_receive);
}

void tearDown() {}

void test_bms_manager_init_null() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_NULL_POINTER, bms_manager_init(NULL, NULL));
}

void test_bms_manager_init_ok() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_init(send, send_receive));
}

void test_bms_manager_init_config() {
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        TEST_ASSERT_EQUAL(hmanager.requested_config[i].REFON, 1U);
    }
}

void test_bms_manager_init_send() {
    TEST_ASSERT_EQUAL(send, hmanager.send);
}

void test_bms_manager_init_send_receive() {
    TEST_ASSERT_EQUAL(send_receive, hmanager.send_receive);
}

void test_bms_manager_set_discharge_cells_ok() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_set_discharge_cells(0xAAABBB));
}

void test_bms_manager_set_discharge_cells_config() {
    bms_manager_set_discharge_cells(0xAAA000);
    TEST_ASSERT_EQUAL(0xAAA, hmanager.requested_config[1].DCC);
}


void test_bms_manager_get_discharge_cells() {
    hmanager.actual_config[0].DCC = 0x123;
    hmanager.actual_config[1].DCC = 0x456;
    TEST_ASSERT_EQUAL(0x456123, bms_manager_get_discharge_cells());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_bms_manager_init_ok);
    RUN_TEST(test_bms_manager_init_null);
    RUN_TEST(test_bms_manager_init_send);
    RUN_TEST(test_bms_manager_init_send_receive);
    RUN_TEST(test_bms_manager_init_config);
    RUN_TEST(test_bms_manager_set_discharge_cells_ok); 
    RUN_TEST(test_bms_manager_set_discharge_cells_config); 
    RUN_TEST(test_bms_manager_get_discharge_cells);
    return UNITY_END();
}

