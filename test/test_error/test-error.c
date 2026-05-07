/*!
 * \file test_error.c
 * \date 2024-06-23
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the error module
 */

#include "unity.h"
#include "error-api.h"
#include "identity-api.h"
#include "cellboard-def.h"
#include "tasks.h"
#include "timebase.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(system_reset_fake);

#define CELLBOARD_ID CELLBOARD_ID_1

extern ErrorLibHandler error_handler;
extern bms_cellboard_error_converted_t error_can_payload;
extern system_reset_callback system_reset;
extern const size_t instances[];
extern const size_t thresholds[];

void test_error_init_null_pointer(void) {
    enum ErrorReturnCode rc = error_init(NULL);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_NULL_POINTER, rc, "error_init() should return ERROR_RC_NULL_POINTER when given a NULL pointer");
    TEST_ASSERT_NULL_MESSAGE(system_reset, "system_reset callback should not be set when error_init() is given a NULL pointer");
}

void test_error_init_ok(void) {

    enum ErrorReturnCode rc = error_init(system_reset_fake);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_OK, rc, "error_init() should return ERROR_RC_OK when given a valid system_reset callback");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(system_reset_fake, system_reset, "system_reset callback was not set correctly in error_init()");
}

void test_error_set_can_communication_expired(void) {
    // Set the CAN communication error until it expires
    for (size_t i = 0; i < thresholds[ERROR_GROUP_CAN_COMMUNICATION]; i++) {
        enum ErrorReturnCode rc = error_set(ERROR_GROUP_CAN_COMMUNICATION, 0U);
        TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_OK, rc, "error_set() should return ERROR_RC_OK when setting a valid error instance");
    }

    // Check if the system reset callback was called
    TEST_ASSERT_TRUE_MESSAGE(system_reset_fake_fake.call_count > 0, "system_reset should be called when CAN communication error expires");
}

void test_error_set_other_error_expired(void) {
    for (size_t i = 0; i < thresholds[ERROR_GROUP_UNDER_VOLTAGE]; i++) {
        enum ErrorReturnCode rc = error_set(ERROR_GROUP_UNDER_VOLTAGE, 0U);
        TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_OK, rc, "error_set() should return ERROR_RC_OK when setting a valid error instance");
    }

    //BUG
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_UNKNOWN, error_set(ERROR_GROUP_UNDER_VOLTAGE, 0U), "error_set() should return ERROR_RC_UNKNOWN when the error expires");

    TEST_ASSERT_EQUAL_MESSAGE(0, system_reset_fake_fake.call_count, "system_reset should not be called when a non-CAN communication error expires");

    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, error_can_payload.cellboard_id, "Error payload cellboard ID mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_GROUP_UNDER_VOLTAGE, error_can_payload.group, "Error payload group mismatch");
    TEST_ASSERT_EQUAL_MESSAGE(0U, error_can_payload.instance, "Error payload instance mismatch");

    TEST_ASSERT_TRUE_MESSAGE(tasks_is_enabled(TASKS_ID_SEND_ERROR), "Error sending task should be enabled when a non-CAN communication error expires");
}

void test_error_set_invalid_instance(void) {
    enum ErrorReturnCode rc = error_set(ERROR_GROUP_UNDER_VOLTAGE, instances[ERROR_GROUP_UNDER_VOLTAGE]);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_UNKNOWN, rc, "error_set() should return ERROR_RC_UNKNOWN when setting an invalid error instance");
}

void test_error_set_invalid_group(void) {
    enum ErrorReturnCode rc = error_set(ERROR_GROUP_COUNT, 0U);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_UNKNOWN, rc, "error_set() should return ERROR_RC_UNKNOWN when setting an invalid error group");
}

void test_error_set_ok(void) {
    enum ErrorReturnCode rc = error_set(ERROR_GROUP_UNDER_VOLTAGE, 0U);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_OK, rc, "error_set() should return ERROR_RC_OK when setting a valid error instance");
}

void test_error_reset_invalid_instance(void) {
    enum ErrorReturnCode rc = error_reset(ERROR_GROUP_UNDER_VOLTAGE, instances[ERROR_GROUP_UNDER_VOLTAGE]);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_UNKNOWN, rc, "error_reset() should return ERROR_RC_UNKNOWN when resetting an invalid error instance");
}

void test_error_reset_invalid_group(void) {
    enum ErrorReturnCode rc = error_reset(ERROR_GROUP_COUNT, 0U);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_UNKNOWN, rc, "error_reset() should return ERROR_RC_UNKNOWN when resetting an invalid error group");
}

void test_error_reset_ok(void) {

    enum ErrorReturnCode rc_set = error_set(ERROR_GROUP_UNDER_VOLTAGE, 0U);

    enum ErrorReturnCode rc_reset = error_reset(ERROR_GROUP_UNDER_VOLTAGE, 0U);
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_RC_OK, rc_reset, "error_reset() should return ERROR_RC_OK when resetting a valid error instance");
}

void test_error_get_payload_no_error_set(void) {
    size_t byte_size;
    bms_cellboard_error_converted_t *payload = error_get_error_canlib_payload(&byte_size);
    TEST_ASSERT_NOT_NULL_MESSAGE(payload, "error_get_error_canlib_payload() should return a valid pointer even if no error is set");
    TEST_ASSERT_EQUAL_MESSAGE(0U, payload->group, "Error payload group should be 0 when no error is set");
    TEST_ASSERT_EQUAL_MESSAGE(0U, payload->instance, "Error payload instance should be 0 when no error is set");
    TEST_ASSERT_EQUAL_MESSAGE(sizeof(error_can_payload), byte_size, "Error payload byte size mismatch when no error is set");
}

void test_error_get_payload_after_error_set(void) {
    for (int i = 0; i <= thresholds[ERROR_GROUP_UNDER_VOLTAGE]; i++) {
        error_set(ERROR_GROUP_UNDER_VOLTAGE, 0U);
    }

    size_t byte_size;
    bms_cellboard_error_converted_t *payload = error_get_error_canlib_payload(&byte_size);
    TEST_ASSERT_NOT_NULL_MESSAGE(payload, "error_get_error_canlib_payload() should return a valid pointer after an error is set");
    TEST_ASSERT_EQUAL_MESSAGE(ERROR_GROUP_UNDER_VOLTAGE, payload->group, "Error payload group mismatch after error is set");
    TEST_ASSERT_EQUAL_MESSAGE(0U, payload->instance, "Error payload instance mismatch after error is set");
    TEST_ASSERT_EQUAL_MESSAGE(sizeof(error_can_payload), byte_size, "Error payload byte size mismatch after error is set");
}

void setUp(void) {
    timebase_init(500U);
    identity_api_init(CELLBOARD_ID);
    RESET_FAKE(system_reset_fake);
    error_init(system_reset_fake);
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_error_init_null_pointer);
    RUN_TEST(test_error_init_ok);
    RUN_TEST(test_error_set_can_communication_expired);
    RUN_TEST(test_error_set_other_error_expired);
    RUN_TEST(test_error_set_invalid_instance);
    RUN_TEST(test_error_set_invalid_group);
    RUN_TEST(test_error_set_ok);
    RUN_TEST(test_error_reset_invalid_instance);
    RUN_TEST(test_error_reset_invalid_group);
    RUN_TEST(test_error_reset_ok);
    RUN_TEST(test_error_get_payload_no_error_set);
    RUN_TEST(test_error_get_payload_after_error_set);

    return UNITY_END();
}