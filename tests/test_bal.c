#include "unity.h"
#include "bal.h"
#include "mocks/Mockidentity.h"
#include "cellboard-def.h"

#define TEST_CELLBOARD_ID CELLBOARD_ID_3

extern _BalHandler hled;

void setUp() {}

void tearDown() {}

void test_bal_init() {
    TEST_ASSERT_EQUAL(bal_init(), BAL_OK);
}

int main() {
    UNITY_BEGIN();

    identity_get_cellboard_id_IgnoreAndReturn(TEST_CELLBOARD_ID);

    return UNITY_END();
}
