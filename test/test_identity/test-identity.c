/**
 * @file test_identity.c
 * @date 2024-06-23
 * @author Riccardo Segala [riccardo.segala@icloud.com]
 *
 * @brief Test functions for the identity module
 */

#include "unity.h"
#include "identity.h"
#include "cellboard-def.h"

#define CELLBOARD_ID CELLBOARD_ID_1

extern _IdentityHandler hidentity;

void test_identity_init() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, hidentity.cellboard_id, "Identity init failed: cellboard_id mismatch");
}

void test_identity_get_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, identity_get_cellboard_id(), "Identity get_cellboard_id failed: cellboard_id mismatch");
}

void test_identity_get_canlib_build_time() {
    TEST_ASSERT_EQUAL_MESSAGE(CANLIB_BUILD_TIME, hidentity.version_can_payload.canlib_build_time, "CANLIB build time mismatch");
}

void test_identity_get_can_payload_cellboard_id() {
    TEST_ASSERT_EQUAL_MESSAGE(CELLBOARD_ID, hidentity.version_can_payload.cellboard_id, "CAN payload cellboard_id mismatch");
}

void test_identity_get_can_payload_build_time() {
    TEST_ASSERT_EQUAL_MESSAGE(CANLIB_BUILD_TIME, hidentity.version_can_payload.canlib_build_time, "CAN payload build_time mismatch");
}

void setUp() {
    identity_init(CELLBOARD_ID);
}

void tearDown() {
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_identity_init);
    RUN_TEST(test_identity_get_cellboard_id);
    RUN_TEST(test_identity_get_canlib_build_time);
    RUN_TEST(test_identity_get_can_payload_cellboard_id);
    RUN_TEST(test_identity_get_can_payload_build_time);
    return UNITY_END();
}
