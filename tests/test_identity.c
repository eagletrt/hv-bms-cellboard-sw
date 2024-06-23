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

void setUp() {}

void tearDown() {}

void test_identity_init() {
    identity_init(CELLBOARD_ID);
    TEST_ASSERT_EQUAL(CELLBOARD_ID, hidentity.can_payload.cellboard_id);
}

void test_identity_get_cellboard_id() {
    identity_init(CELLBOARD_ID);
    TEST_ASSERT_EQUAL(CELLBOARD_ID, identity_get_cellboard_id());
}

void test_identity_get_can_payload() {
    identity_init(CELLBOARD_ID);
    size_t size = 0;
    bms_cellboard_version_converted_t * payload = identity_get_can_payload(&size);
    TEST_ASSERT_EQUAL(sizeof(hidentity.can_payload), size);
    TEST_ASSERT_NOT_NULL(payload);
    TEST_ASSERT_EQUAL(CELLBOARD_ID, payload->cellboard_id);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_identity_init);
    RUN_TEST(test_identity_get_cellboard_id);
    RUN_TEST(test_identity_get_can_payload);
    return UNITY_END();
}
