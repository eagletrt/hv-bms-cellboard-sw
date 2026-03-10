#include "unity.h"
#include "timebase.h"
#include "tasks.h"
#include "watchdog.h"

#include <string.h>

extern _TaskHandler htasks;

extern _TimebaseHandler htimebase;

int mock_watchdog_calls = 0;

int mock_tasks_calls[3] = { 0 };

void mock_watchdog_callback(void) {
    mock_watchdog_calls++;
}

void mock_task_1_callback(void) {
    mock_tasks_calls[0]++;
}

void mock_task_2_callback(void) {
    mock_tasks_calls[1]++;
}

void mock_taks_3_callback(void) {
    mock_tasks_calls[2]++;
}

static void setup_three_tasks(Task **task_1, Task **task_2, Task **task_3) {
    *task_1 = tasks_get_task(TASKS_ID_SEND_STATUS);
    *task_2 = tasks_get_task(TASKS_ID_SEND_VERSION);
    *task_3 = tasks_get_task(TASKS_ID_SEND_ERROR);

    TEST_ASSERT_NOT_NULL_MESSAGE(*task_1, "Task 1 pointer should not be NULL");
    TEST_ASSERT_NOT_NULL_MESSAGE(*task_2, "Task 2 pointer should not be NULL");
    TEST_ASSERT_NOT_NULL_MESSAGE(*task_3, "Task 3 pointer should not be NULL");

    (*task_1)->enabled = true;
    (*task_1)->interval = 3U;
    (*task_1)->exec = mock_task_1_callback;

    (*task_2)->enabled = false;
    (*task_2)->interval = 0U;
    (*task_2)->exec = mock_task_2_callback;

    (*task_3)->enabled = false;
    (*task_3)->interval = 5U;
    (*task_3)->exec = mock_taks_3_callback;
}

static void run_ticks(int count) {
    for (int i = 0; i < count; ++i) {
        TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_inc_tick(), "Failed to increment tick");
        TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run after tick increment");
    }
}

void test_timebase_init_OK(void) {
    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_init(5U), "timebase_init() should return TIMEBASE_OK when resolution is valid");
};

void test_timebase_init_zero_resolution_defaults_to_one(void) {
    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_init(0U), "timebase_init() should return TIMEBASE_OK when resolution is 0");
    TEST_ASSERT_EQUAL_MESSAGE(1U, timebase_get_resolution(), "timebase_init() should set resolution to 1 when 0 is passed");
};

void test_timebase_disabled_behaviour(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 4U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, watchdog_start(&wdg), "Failed to register watchdog");

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_DISABLED, timebase_inc_tick(), "Timebase should fail to increment tick");
};

void test_timebase_watchdog_stop(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 4U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, watchdog_stop(&wdg), "Failed to stop watchdog");
    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(0, mock_watchdog_calls, "Watchdog should NOT time-out at threshold when stopped");
};

void test_timebase_watchdog_reset_behaviout(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 6U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_restart(&wdg), "Failed to restart watchdog");
    TimebaseScheduledWatchdog *wdg_p = (TimebaseScheduledWatchdog *)min_heap_peek(&htimebase.scheduled_watchdogs);
    TEST_ASSERT_EQUAL_MESSAGE(wdg_p->t, timebase_get_tick() + TIMEBASE_MS_TO_TICKS(wdg.timeout, htimebase.resolution), "The timeout tick should be current tick + timeout");
};

void test_timebase_watchdog_double_start(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 4U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_BUSY, watchdog_start(&wdg), "Watchdog should answer buisy to double start");
};

void test_timebase_watchdog_timeout_state(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 4U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(3);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_watchdog_calls, "Watchdog should time-out at threshold tick");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_TIMED_OUT, watchdog_start(&wdg), "Wrong return value from start after time-out");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_TIMED_OUT, watchdog_stop(&wdg), "Wrong return value from stop after time-out");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_TIMED_OUT, watchdog_reset(&wdg), "Wrong return value from reset after time-out");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_restart(&wdg), "Wrong return value from restart after time-out");
};

void test_timebase_watchdog_does_not_timeout_before_threshold(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 6U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(2);
    TEST_ASSERT_EQUAL_MESSAGE(0, mock_watchdog_calls, "Watchdog should not time-out before threshold");
}

void test_timebase_watchdog_times_out_at_threshold(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 6U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(3);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_watchdog_calls, "Watchdog should time-out at threshold tick");
}

void test_timebase_watchdog_restart_prevents_pending_timeout(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 6U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(2);

    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_restart(&wdg), "Failed to restart watchdog");
    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(0, mock_watchdog_calls, "Watchdog should not time-out right after restart");
}

void test_timebase_watchdog_restart_shifts_timeout_window(void) {
    Watchdog wdg = { 0 };
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_init(&wdg, 6U, mock_watchdog_callback), "Failed to initialize watchdog");
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_start(&wdg), "Failed to register watchdog");
    (void)timebase_set_enable(true);

    run_ticks(2);
    TEST_ASSERT_EQUAL_MESSAGE(WATCHDOG_OK, watchdog_restart(&wdg), "Failed to restart watchdog");
    run_ticks(3);

    TEST_ASSERT_EQUAL_MESSAGE(1, mock_watchdog_calls, "Watchdog should time-out according to restarted schedule");
}

void test_timebase_task_periodic_enabled_runs_on_tick_zero(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_tasks_calls[0], "Enabled periodic task should run at tick 0");
}

void test_timebase_task_oneshot_disabled_does_not_run(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    TEST_ASSERT_EQUAL_MESSAGE(0, mock_tasks_calls[1], "Disabled one-shot task should not run");
}

void test_timebase_task_periodic_runs_on_each_interval(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    run_ticks(3);
    TEST_ASSERT_EQUAL_MESSAGE(2, mock_tasks_calls[0], "Periodic task should run at ticks 0 and 3");
}

void test_timebase_task_oneshot_runs_after_enable(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_VERSION, true), "Unable to enable task 2");
    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_tasks_calls[1], "One-shot task should run after enabling");
}

void test_timebase_task_oneshot_runs_only_once_per_enable(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_VERSION, true), "Unable to enable task 2");
    run_ticks(4);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_tasks_calls[1], "One-shot task should execute only once per enable");
}

void test_timebase_task_periodic_disabled_misses_slot_and_waits_next_period(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    run_ticks(3);
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_ERROR, true), "Unable to enable task 3");
    run_ticks(3);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_tasks_calls[2], "Task 3 should run at next period after enabling");
}

void test_timebase_task_disable_stops_future_periodic_executions(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    run_ticks(6);
    TEST_ASSERT_EQUAL_MESSAGE(3, mock_tasks_calls[0], "Task 1 should run at ticks 0, 3 and 6");
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_STATUS, false), "Unable to disable task 1");

    run_ticks(4);
    TEST_ASSERT_EQUAL_MESSAGE(3, mock_tasks_calls[0], "Disabled periodic task should not run");
}

void test_timebase_task_oneshot_can_be_reenabled_and_run_again(void) {
    Task *task_1 = NULL;
    Task *task_2 = NULL;
    Task *task_3 = NULL;

    setup_three_tasks(&task_1, &task_2, &task_3);
    (void)timebase_set_enable(true);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEBASE_OK, timebase_routine(), "Routine should run at tick 0");
    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_VERSION, true), "Unable to enable task 2");
    run_ticks(2);
    TEST_ASSERT_EQUAL_MESSAGE(1, mock_tasks_calls[1], "Task 2 should have executed once");

    TEST_ASSERT_EQUAL_MESSAGE(TASKS_OK, tasks_set_enable(TASKS_ID_SEND_VERSION, true), "Unable to re-enable task 2");
    run_ticks(1);
    TEST_ASSERT_EQUAL_MESSAGE(2, mock_tasks_calls[1], "Task 2 should execute again after re-enable");
}

void setUp(void) {
    mock_watchdog_calls = 0;
    mock_tasks_calls[0] = 0;
    mock_tasks_calls[1] = 0;
    mock_tasks_calls[2] = 0;
    (void)timebase_init(2U);
    // Disable all tasks else they will call random callbacks
    for (size_t i = 0; i < TASKS_ID_COUNT; ++i)
        tasks_set_enable((TasksId)i, false);
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_timebase_init_OK);
    RUN_TEST(test_timebase_init_zero_resolution_defaults_to_one);
    RUN_TEST(test_timebase_disabled_behaviour);
    RUN_TEST(test_timebase_watchdog_stop);
    RUN_TEST(test_timebase_watchdog_reset_behaviout);
    RUN_TEST(test_timebase_watchdog_double_start);
    RUN_TEST(test_timebase_watchdog_timeout_state);
    RUN_TEST(test_timebase_watchdog_does_not_timeout_before_threshold);
    RUN_TEST(test_timebase_watchdog_times_out_at_threshold);
    RUN_TEST(test_timebase_watchdog_restart_prevents_pending_timeout);
    RUN_TEST(test_timebase_watchdog_restart_shifts_timeout_window);
    RUN_TEST(test_timebase_task_periodic_enabled_runs_on_tick_zero);
    RUN_TEST(test_timebase_task_oneshot_disabled_does_not_run);
    RUN_TEST(test_timebase_task_periodic_runs_on_each_interval);
    RUN_TEST(test_timebase_task_oneshot_runs_after_enable);
    RUN_TEST(test_timebase_task_oneshot_runs_only_once_per_enable);
    RUN_TEST(test_timebase_task_periodic_disabled_misses_slot_and_waits_next_period);
    RUN_TEST(test_timebase_task_disable_stops_future_periodic_executions);
    RUN_TEST(test_timebase_task_oneshot_can_be_reenabled_and_run_again);

    return UNITY_END();
}
