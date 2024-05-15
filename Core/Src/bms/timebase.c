/**
 * @file timebase.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to manage periodic tasks at certain intervals
 */

#include "timebase.h"

#include "bms_network.h"
#include "min-heap.h"

#ifdef CONF_TIMEBASE_MODULE_ENABLE

// Include the tasks
#define TASKS_IMPLEMENTATION
#include "tasks.h"

/**
 * @brief Convert the time in ms to ticks
 *
 * @param T The time to convert
 * @param RES The resolution of a tick
 *
 * @return ticks The corresponing amount of ticks
 */
#define TIMEBASE_TIME_TO_TICKS(T, RES) ((ticks)((T) / (RES)))

/**
 * @brief Convert the ticks in ms
 *
 * @param T The ticks to convert
 * @param RES The resolution of a tick
 *
 * @return time The corresponing amount of ms
 */
#define TIMEBASE_TICKS_TO_TIME(T, RES) ((time)((T) * (RES)))


/** @brief Type definition for a function that excecutes a single task */
typedef void (* timebase_task_callback)(ticks tick, time ms);

/**
 * @brief Definition of a single task
 *
 * @details An interval of 0 means that the task is only run once
 *
 * @param start The time when the tasks is executed first
 * @param interval The amount of time that must elapsed before the tasks is re-executed
 * @param exec A pointer to the task callback
 */
typedef struct {
    ticks start;
    ticks interval;
    timebase_task_callback exec;
} TimebaseTask;

/**
 * @brief Definition of a scheduled task that has not been called yet
 *
 * @param t The time in which the task should be executed
 * @param task A pointer to the task to run
 */
typedef struct {
    ticks t;
    TimebaseTask * task;
} TimebaseScheduledTask;

/**
 * @brief Timebase handler structure
 *
 * @param enabled True if the timebase is running, false otherwise
 * @param resolution Number of ms that represent one tick
 * @param t The current number of ticks
 */
static struct {
    bool enabled;
    time resolution; // in ms
    ticks t;

    TimebaseTask tasks[TASKS_COUNT];
    MinHeap(TimebaseScheduledTask, TASKS_COUNT) scheduled;
} htimebase = {
    .enabled = false,
    .resolution = 1U,
    .t = 0
};

int8_t _timebase_scheduled_task_compare(void * a, void * b) {
    TimebaseScheduledTask * f = (TimebaseScheduledTask *)a;
    TimebaseScheduledTask * s = (TimebaseScheduledTask *)b;

    // Compare timestamps
    if (f->t < s->t)
        return -1;
    return f->t == s->t ? 0 : 1;
}

TimebaseReturnCode timebase_init(time resolution_ms) {
    if (resolution_ms > 0U)
        htimebase.resolution = resolution_ms;

    // Initialize the tasks
    htimebase.tasks[TASKS_ID_SEND_STATUS].start = 0U;
    htimebase.tasks[TASKS_ID_SEND_STATUS].interval = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_STATUS_CYCLE_TIME_MS, htimebase.resolution);
    htimebase.tasks[TASKS_ID_SEND_STATUS].exec = tasks_send_status;

    htimebase.tasks[TASKS_ID_SEND_VERSION].start = 1U;
    htimebase.tasks[TASKS_ID_SEND_VERSION].interval = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_VERSION_CYCLE_TIME_MS, htimebase.resolution);
    htimebase.tasks[TASKS_ID_SEND_VERSION].exec = tasks_send_version;

    htimebase.tasks[TASKS_ID_SEND_VOLTAGES].start = 2U;
    htimebase.tasks[TASKS_ID_SEND_VOLTAGES].interval = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_VOLTAGES_CYCLE_TIME_MS, htimebase.resolution);
    htimebase.tasks[TASKS_ID_SEND_VOLTAGES].exec = tasks_send_voltages;

    htimebase.tasks[TASKS_ID_SEND_TEMPERATURES].start = 3U;
    htimebase.tasks[TASKS_ID_SEND_TEMPERATURES].interval = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_TEMPERATURES_CYCLE_TIME_MS, htimebase.resolution);
    htimebase.tasks[TASKS_ID_SEND_TEMPERATURES].exec = tasks_send_temperatures;

    htimebase.tasks[TASKS_ID_CHECK_WATCHDOG].start = 4U;
    htimebase.tasks[TASKS_ID_CHECK_WATCHDOG].interval = TIMEBASE_TIME_TO_TICKS(200U, htimebase.resolution);
    htimebase.tasks[TASKS_ID_CHECK_WATCHDOG].exec = tasks_check_watchdog;

    // Initialize the heap
    (void)min_heap_init(&htimebase.scheduled, TimebaseScheduledTask, TASKS_COUNT, _timebase_scheduled_task_compare);
    for (size_t i = 0; i < TASKS_COUNT; ++i) {
        TimebaseScheduledTask aux = {
            .t = htimebase.tasks[i].start,
            .task = &htimebase.tasks[i]
        };
        (void)min_heap_insert(&htimebase.scheduled, &aux);
    }

    return TIMEBASE_OK;
}

inline void timebase_set_enable(bool enabled) {
    htimebase.enabled = enabled;
}

TimebaseReturnCode timebase_inc_tick(void) {
    if (!htimebase.enabled)
        return TIMEBASE_DISABLED;

    ++htimebase.t;
    return TIMEBASE_OK;
}

ticks timebase_get_tick(void) {
    return htimebase.t;
}

time timebase_get_time(void) {
    return TIMEBASE_TICKS_TO_TIME(htimebase.t, htimebase.resolution);
}

// TODO: Check delta time between the right time
TimebaseReturnCode timebase_routine(void) {
    if (!htimebase.enabled)
        return TIMEBASE_DISABLED;

    // Execute all the tasks which interval has already elapsed
    TimebaseScheduledTask * aux = (TimebaseScheduledTask *)min_heap_peek(&htimebase.scheduled);
    while (aux != NULL && aux->t <= htimebase.t) {
        // Get and execute current task
        TimebaseScheduledTask task;
        (void)min_heap_remove(&htimebase.scheduled, 0, &task);

        ticks t = htimebase.t; // Copy ticks value to avoid inconsistencies caused by interrupts
        task.t = t + task.task->interval;
        task.task->exec(t, TIMEBASE_TICKS_TO_TIME(t, htimebase.resolution));

        // If the interval is 0 do not insert again the task inside the heap (i.e. runs only once)
        if (task.task->interval > 0U)
            (void)min_heap_insert(&htimebase.scheduled, &task);

        aux = (TimebaseScheduledTask *)min_heap_peek(&htimebase.scheduled);
    }
    return TIMEBASE_OK;
}

#ifdef CONF_TIMEBASE_STRINGS_ENABLE

static char * timebase_module_name = "timebase";

static char * timebase_return_code_name[] = {
    [TIMEBASE_OK] = "ok",
    [TIMEBASE_DISABLED] = "disabled",
    [TIMEBASE_BUSY] = "busy"
};

static char * timebase_return_code_description[] = {
    [TIMEBASE_OK] = "executed successfully",
    [TIMEBASE_DISABLE] = "the timebase is not enabled",
    [TIMEBASE_BUSY] = "the timebase couldn't perform the requested operation"
};

#endif // CONF_TIMEBASE_STRINGS_ENABLE

#endif // CONF_TIMEBASE_MODULE_ENABLE_H
