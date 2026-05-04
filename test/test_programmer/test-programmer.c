/*!
 * \file test_programmer.c
 * \date 2024-06-23
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 * 
 * \brief Test functions for the programmer module
 */

#include "unity.h"
#include "programmer-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "fsm.h"
#include "timebase.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

#define CELLBOARD_ID CELLBOARD_ID_1

extern struct ProgrammerHandler programmer_handler;

FAKE_VOID_FUNC(reset);

void prv_programmer_flash_timeout(void);
void prv_programmer_flash_stop(void);
void prv_programmer_flash_reset_flags(void);

void test_programmer_init_with_null_pointer(void) {
    TEST_ASSERT_EQUAL_MESSAGE(PROGRAMMER_RC_NULL_POINTER, programmer_init(NULL), "programmer_init should return PROGRAMMER_RC_NULL_POINTER when called with a null pointer");
}

void test_programmer_init_with_correct_value(void) {
    TEST_ASSERT_EQUAL_MESSAGE(reset, programmer_handler.reset, "programmer_handler.reset should be initialized to the provided reset function");
}

void test_programmer_init_flash_event_type(void) {
    TEST_ASSERT_EQUAL_MESSAGE(FSM_EVENT_TYPE_FLASH_REQUEST, programmer_handler.flash_event.type, "programmer_handler.flash_event.type should be initialized to FSM_EVENT_TYPE_FLASH_REQUEST");
}

void test_programmer_init_can_payload_cellboard_id(void) {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, programmer_handler.can_payload.cellboard_id, "programmer_handler.can_payload.cellboard_id should be initialized to the correct cellboard ID");
}

void test_programmer_init_can_payload_ready(void) {
    TEST_ASSERT_TRUE_MESSAGE(programmer_handler.can_payload.ready, "programmer_handler.can_payload.ready should be initialized to true");
}

void test_programmer_init_target(void) {
    TEST_ASSERT_EQUAL_MESSAGE(MAINBOARD_ID, programmer_handler.target, "programmer_handler.target should be initialized to the correct mainboard ID");
}

void test_programmer_routine_called(void) {
    programmer_handler.flashing = true;
    programmer_handler.target = identity_api_get_cellboard_id();
    enum ProgrammerReturnCode result = programmer_routine();
    TEST_ASSERT_TRUE_MESSAGE(reset_fake.call_count > 0, "reset function should be called when programmer_routine is called with flashing set to true and target equal to the current cellboard ID");
    // Technically this would never return as the micro would reset
    TEST_ASSERT_EQUAL_MESSAGE(PROGRAMMER_RC_BUSY, result, "programmer_routine should return PROGRAMMER_RC_BUSY when called with flashing set to true and target equal to the current cellboard ID");
}

void test_programmer_routine_not_direct_target(void) {

    programmer_handler.flashing = true;
    programmer_handler.target = (CELLBOARD_ID + 1); // Set a different target
    enum ProgrammerReturnCode result = programmer_routine();
    TEST_ASSERT_FALSE_MESSAGE(reset_fake.call_count > 0, "reset function should not be called when programmer_routine is called with flashing set to true and target not equal to the current cellboard ID");
    TEST_ASSERT_EQUAL_MESSAGE(PROGRAMMER_RC_BUSY, result, "programmer_routine should return PROGRAMMER_RC_BUSY when called with flashing set to true and target not equal to the current cellboard ID");
}

void test_programmer_routine_not_direct_target_stop_routine(void) {

    programmer_handler.flashing = true;
    programmer_handler.flash_stop = true;           // Stop the routine
    programmer_handler.target = (CELLBOARD_ID + 1); // Set a different target
    enum ProgrammerReturnCode result = programmer_routine();
    TEST_ASSERT_FALSE_MESSAGE(reset_fake.call_count > 0, "reset function should not be called when programmer_routine is called with flashing set to true and target not equal to the current cellboard ID");
    TEST_ASSERT_EQUAL_MESSAGE(PROGRAMMER_RC_OK, result, "programmer_routine should return PROGRAMMER_RC_OK when called with flashing set to true and target not equal to the current cellboard ID");
}

void test_programmer_flash_timeout(void) {
    memset(&programmer_handler, 0xFFU, sizeof(programmer_handler));
    prv_programmer_flash_timeout();
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flash_request, "programmer_handler.flash_request should be false after flash timeout");
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flashing, "programmer_handler.flashing should be false after flash timeout");
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flash_stop, "programmer_handler.flash_stop should be false after flash timeout");
}

void test_programmer_flash_stop(void) {
    memset(&programmer_handler, 0xFFU, sizeof(programmer_handler));
    programmer_handler.flash_stop = false;
    prv_programmer_flash_stop();
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flash_request, "programmer_handler.flash_request should be false after flash stop");
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flashing, "programmer_handler.flashing should be false after flash stop");
    TEST_ASSERT_TRUE_MESSAGE(programmer_handler.flash_stop, "programmer_handler.flash_stop should be true after flash stop");
}

void test_programmer_flash_reset_flags(void) {
    memset(&programmer_handler, 0xFFU, sizeof(programmer_handler));
    prv_programmer_flash_reset_flags();
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flash_request, "programmer_handler.flash_request should be false after flash reset flags");
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flashing, "programmer_handler.flashing should be false after flash reset flags");
    TEST_ASSERT_FALSE_MESSAGE(programmer_handler.flash_stop, "programmer_handler.flash_stop should be false after flash reset flags");
}

void setUp() {

    timebase_init(500U);
    programmer_init(reset);
    identity_api_init(CELLBOARD_ID);

    RESET_FAKE(reset);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_programmer_init_with_null_pointer);
    RUN_TEST(test_programmer_init_with_correct_value);
    RUN_TEST(test_programmer_init_flash_event_type);
    RUN_TEST(test_programmer_init_can_payload_cellboard_id);
    RUN_TEST(test_programmer_init_can_payload_ready);
    RUN_TEST(test_programmer_init_target);
    RUN_TEST(test_programmer_routine_called);
    RUN_TEST(test_programmer_flash_timeout);
    RUN_TEST(test_programmer_flash_stop);
    RUN_TEST(test_programmer_flash_reset_flags);
    return UNITY_END();
}
