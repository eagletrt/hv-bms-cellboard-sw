/**
 * \file test_identity.c
 * \date 2024-06-23
 * \author Riccardo Segala [riccardo.segala@icloud.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Test functions for the identity module
 */

#include "unity.h"
#include <time.h>
#include "identity_api.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_2

extern struct IdentityHandler identity_handler;

void test_identity_init() {

    struct IdentityHandler expected = {
        .cellboard_id = CELLBOARD_ID,
        .version_can_payload.cellboard_id = (bms_cellboard_version_cellboard_id)CELLBOARD_ID,
        .version_can_payload.component_build_time = identity_handler.build_time >> 3U,
        .version_can_payload.canlib_build_time = CANLIB_BUILD_TIME
    };

    struct tm tm = { 0 };
    if (strptime(IDENTITY_BUILD_TIME_STRING, "%b %d %Y %H:%M:%S", &tm) != NULL)
        expected.build_time = mktime(&tm);

    identity_init(CELLBOARD_ID);

    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, identity_handler.cellboard_id, "Identity init failed: cellboard_id mismatch");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected, &identity_handler, sizeof(identity_handler), "Handler not filled correctly");
}

void test_identity_init_when_id_is_negative() {

    struct IdentityHandler expected = { 0 };

    TEST_ASSERT_EQUAL_MESSAGE(IDENTITY_RC_INVALID_ID, identity_init(-1), "Identity init didn't return invalid");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected, &identity_handler, sizeof(identity_handler), "Handler not empty");
}

void test_identity_init_when_id_is_out_of_bounds() {

    struct IdentityHandler expected = { 0 };

    TEST_ASSERT_EQUAL_MESSAGE(IDENTITY_RC_INVALID_ID, identity_init(CELLBOARD_ID_COUNT), "Identity init didn't return invalid");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expected, &identity_handler, sizeof(identity_handler), "Handler not empty");
}

void test_identity_get_can_payload_cellboard_id() {

    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, identity_handler.version_can_payload.cellboard_id, "CAN payload cellboard_id mismatch");
}

void test_identity_get_can_payload_build_time() {

    TEST_ASSERT_EQUAL_MESSAGE(CANLIB_BUILD_TIME, identity_handler.version_can_payload.canlib_build_time, "CAN payload build_time mismatch");
}

void setUp() {
    identity_init(CELLBOARD_ID);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_identity_init);
    RUN_TEST(test_identity_init_when_id_is_negative);
    RUN_TEST(test_identity_init_when_id_is_out_of_bounds);
    RUN_TEST(test_identity_get_can_payload_cellboard_id);
    RUN_TEST(test_identity_get_can_payload_build_time);

    return UNITY_END();
}
