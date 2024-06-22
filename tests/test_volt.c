#include "unity.h"
#include "volt.h"
#include "mocks/Mockidentity.h"
#include "cellboard-def.h"

#define TEST_CELLBOARD_ID CELLBOARD_ID_3

extern _VoltHandler hvolt;

void setUp() {}

void tearDown() {}

void test_volt_init() {
    TEST_ASSERT_EQUAL(volt_init(), VOLT_OK);

    volt_init();
    TEST_ASSERT_EQUAL(hvolt.can_payload.cellboard_id, TEST_CELLBOARD_ID);
}

int main() {
    UNITY_BEGIN();

    identity_get_cellboard_id_IgnoreAndReturn(TEST_CELLBOARD_ID);
    RUN_TEST(test_volt_init);

    return UNITY_END();
}
