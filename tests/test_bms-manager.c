#include "unity.h"
#include "bms-manager.h"
#include "identity.h"
#include "cellboard-def.h"
#include "ltc6811.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _BmsManagerHandler hmanager;

void setUp() {}

void tearDown() {}

void test_bms_manager_init() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_NULL_POINTER, bms_manager_init(NULL, NULL));
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_init(NULL, (void*) 0x01));

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        TEST_ASSERT_EQUAL(hmanager.requested_config[i].REFON, 1U);
    }
}

void test_bms_manager_set_discharge_cells() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_init(NULL, (void*) 0x01));
    
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_set_discharge_cells(0xAAABBB));

    //TEST_ASSERT_EQUAL(LTC8811_DCTO_30S, hmanager.requested_config[0].DCTO);
    //TEST_ASSERT_EQUAL(LTC8811_DCTO_30S, hmanager.requested_config[1].DCTO);

    TEST_ASSERT_EQUAL(0xBBB, hmanager.requested_config[0].DCC);
    TEST_ASSERT_EQUAL(0xAAA, hmanager.requested_config[1].DCC);
}

void test_bms_manager_get_discharge_cells() {
    TEST_ASSERT_EQUAL(BMS_MANAGER_OK, bms_manager_init(NULL, (void*) 0x01));

    hmanager.actual_config[0].DCC = 0x123;
    hmanager.actual_config[1].DCC = 0x456;
    TEST_ASSERT_EQUAL(0x456123, bms_manager_get_discharge_cells());
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_bms_manager_init);
    RUN_TEST(test_bms_manager_set_discharge_cells); 
    RUN_TEST(test_bms_manager_get_discharge_cells);
    return UNITY_END();
}

