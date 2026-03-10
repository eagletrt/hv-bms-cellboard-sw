#include "unity.h"
#include "timebase.h"
#include "tasks.h"

#include <string.h>

extern _TaskHandler htasks;

void test_tasks_init_ok(void) {
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_init((milliseconds_t)1U), "tasks_init() failed to return TASKS_OK");
}

void test_tasks_init_resolution_min(void) {
    (void)tasks_init((milliseconds_t)0U);

#define TASKS_X(NAME, ENABLED, START, INTERVAL, EXEC)                                                                                                                      \
    do {                                                                                                                                                                   \
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIMEBASE_TIME_TO_TICKS(INTERVAL, 1U), (uint32_t)tasks_get_interval(TASKS_NAME_TO_ID(NAME)), "Interval mismatch for task " #NAME); \
    } while (0U);
}

void test_tasks_set_enable_invalid_id(void) {
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_INVALID_ID, tasks_set_enable((TasksId)TASKS_ID_COUNT, true), "tasks_set_enable() should return TASKS_INVALID_ID for invalid ID");
}

void test_tasks_set_enable_valid_id(void) {
    TasksId id = TASKS_ID_SEND_POWER;
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(id, true), "tasks_set_enable() failed to enable valid task ID");
    TEST_ASSERT_TRUE_MESSAGE(tasks_is_enabled(id), "Task should be enabled after tasks_set_enable() with true");

    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(id, false), "tasks_set_enable() failed to disable valid task ID");
    TEST_ASSERT_FALSE_MESSAGE(tasks_is_enabled(id), "Task should be disabled after tasks_set_enable() with false");
}

// i don't think more tests are needed for the getters since they just return values from the struct

void setUp(void) {
    (void)tasks_init((milliseconds_t)1U);
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_tasks_init_ok);
    RUN_TEST(test_tasks_init_resolution_min);
    RUN_TEST(test_tasks_set_enable_invalid_id);
    RUN_TEST(test_tasks_set_enable_valid_id);
    return UNITY_END();
}
