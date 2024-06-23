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

void setUp() {}

void tearDown() {}

void test_programmer_init() {
   TEST_ASSERT_EQUAL(PROGRAMMER_OK, programmer_init(NULL)); 
   TEST_ASSERT_EQUAL(NULL, hprogrammer.reset);
   TEST_ASSERT_EQUAL(FSM_EVENT_TYPE_FLASH_REQUEST, hprogrammer.flash_event.type);
   TEST_ASSERT_EQUAL(CELLBOARD_ID, hprogrammer.can_payload.cellboard_id);
   TEST_ASSERT_TRUE(hprogrammer.can_payload.ready);
   TEST_ASSERT_EQUAL(MAINBOARD_ID, hprogrammer.target);
}

int main() {

    identity_init(CELLBOARD_ID);

    UNITY_BEGIN();
    RUN_TEST(test_programmer_init);
    return UNITY_END();
}
