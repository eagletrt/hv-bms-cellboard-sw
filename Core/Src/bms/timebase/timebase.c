/**
 * @file timebase.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to manage periodic tasks at certain intervals
 */

#include "timebase.h"

#include <string.h>

#include "bms_network.h"
#include "min-heap.h"
#include "tasks.h"

#ifdef CONF_TIMEBASE_MODULE_ENABLE


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
    ticks_t start;
    ticks_t interval;
    tasks_callback exec;
} TimebaseTask;

/**
 * @brief Definition of a scheduled task that has not been called yet
 *
 * @param t The time in which the task should be executed
 * @param task A pointer to the task to run
 */
typedef struct {
    ticks_t t;
    TimebaseTask * task;
} TimebaseScheduledTask;

/**
 * @brief Definition of a running watchdog
 *
 * @param t The time in which the watchdog should timeout
 * @param watchdog A pointer to the watchdog handler structure
 */
typedef struct {
    ticks_t t;
    Watchdog * watchdog;
} TimebaseRunningWatchdog;

/**
 * @brief Timebase handler structure
 *
 * @param enabled True if the timebase is running, false otherwise
 * @param resolution Number of ms that represent one tick
 * @param t The current number of ticks
 * @param tasks The array of tasks to execute
 * @param scheduled The heap of scheduled tasks that has to be executed
 * @param watchdogs The array of watchdogs
 * @param running_wdg The heap of running watchdogs
 */
static struct {
    bool enabled;
    milliseconds_t resolution; // in ms
    ticks_t t;

    // Tasks
    TimebaseTask tasks[TASKS_COUNT];
    MinHeap(TimebaseScheduledTask, TASKS_COUNT) scheduled;

    // Watchdogs
    MinHeap(TimebaseRunningWatchdog, TIMEBASE_RUNNING_WATCHDOG_COUNT) wdg_running;
} htimebase;

int8_t _timebase_scheduled_task_compare(void * a, void * b) {
    TimebaseScheduledTask * f = (TimebaseScheduledTask *)a;
    TimebaseScheduledTask * s = (TimebaseScheduledTask *)b;

    // Compare timestamps
    if (f->t < s->t) return -1;
    if (f->t > s->t) return 1;

    /**************************************************************************
     * For the equality check, in addition to the ticks, the pointers to the
     * task must also be equal, otherwise -1 or 1 may be returned
     * In this case 1 is preferred because it avoid useless swaps between
     * elements that have the same number of ticks
     ***************************************************************************/
    if (f->task == s->task)
        return 0;
    return 1;
}

int8_t _timebase_watchdog_compare(void * a, void * b) {
    TimebaseRunningWatchdog * f = (TimebaseRunningWatchdog *)a;
    TimebaseRunningWatchdog * s = (TimebaseRunningWatchdog *)b;

    /**************************************************************************
     * For the equality check, only the pointers to the watchdogs are checked
     * This can cause problems if multiple instances of the same watchdog are
     * inserted in the heap, but in this case a watchdog can be inserted inside
     * the heap only once
     ***************************************************************************/
    if (f->watchdog == s->watchdog)
        return 0;

    // Compare timestamps
    if (f->t < s->t) return -1;
    return f->t == s->t ? 0 : 1;
}

TimebaseReturnCode timebase_init(milliseconds_t resolution_ms) {
    // Initialize timebase to 0
    memset(&htimebase, 0U, sizeof(htimebase));

    // Set default parameters
    htimebase.enabled = false;
    htimebase.resolution = (resolution_ms == 0U) ? 1U : resolution_ms;

    // Initialize the tasks
    (void)tasks_init(resolution_ms);
    for (TasksId id = 0; id < TASKS_ID_COUNT; ++id) {
        ticks_t interval = tasks_get_interval_from_id(id);
        tasks_callback exec = tasks_get_callback_from_id(id);
        if (exec == NULL)
            return TIMEBASE_NULL_POINTER;

        htimebase.tasks[id].start = id;
        htimebase.tasks[id].interval = interval;
        htimebase.tasks[id].exec = exec;
    }

    // Initialize the tasks heap
    (void)min_heap_init(&htimebase.scheduled, TimebaseScheduledTask, TASKS_COUNT, _timebase_scheduled_task_compare);
    for (size_t i = 0; i < TASKS_COUNT; ++i) {
        TimebaseScheduledTask aux = {
            .t = htimebase.tasks[i].start,
            .task = &htimebase.tasks[i]
        };
        (void)min_heap_insert(&htimebase.scheduled, &aux);
    }

    // Initialize the watchdogs heap
    (void)min_heap_init(&htimebase.wdg_running, TimebaseRunningWatchdog, TIMEBASE_RUNNING_WATCHDOG_COUNT, _timebase_watchdog_compare);
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

ticks_t timebase_get_tick(void) {
    return htimebase.t;
}

milliseconds_t timebase_get_time(void) {
    return TIMEBASE_TICKS_TO_TIME(htimebase.t, htimebase.resolution);
}

milliseconds_t timebase_get_resolution(void) {
    return htimebase.resolution;
}

TimebaseReturnCode timebase_register_watchdog(Watchdog * watchdog) {
    if (watchdog == NULL)
        return TIMEBASE_NULL_POINTER;

    TimebaseRunningWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    if (min_heap_find(&htimebase.wdg_running, &aux) >= 0)
        return TIMEBASE_BUSY;

    aux.t = htimebase.t + watchdog->timeout;
    if (min_heap_insert(&htimebase.wdg_running, &aux) == MIN_HEAP_FULL)
        return TIMEBASE_WATCHDOG_UNAVAILABLE;
    return TIMEBASE_OK;
}

TimebaseReturnCode timebase_unregister_watchdog(Watchdog * watchdog) {
    if (watchdog == NULL)
        return TIMEBASE_NULL_POINTER;
    
    // Get and remove the running watchdog from the heap
    TimebaseRunningWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    signed_size_t i = min_heap_find(&htimebase.wdg_running, &aux);
    if (i < 0)
        return TIMEBASE_WATCHDOG_NOT_REGISTERED;
    (void)min_heap_remove(&htimebase.wdg_running, i, NULL);
    return TIMEBASE_OK;
}

bool timebase_is_registered_watchdog(Watchdog * watchdog) {
    if (watchdog == NULL)
        return false;

    // Get the running watchdog
    TimebaseRunningWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    return min_heap_find(&htimebase.wdg_running, &aux) >= 0;
}

TimebaseReturnCode timebase_update_watchdog(Watchdog * watchdog) {
    if (watchdog == NULL)
        return TIMEBASE_NULL_POINTER;

    // Get the running watchdog
    TimebaseRunningWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    signed_size_t i = min_heap_find(&htimebase.wdg_running, &aux);

    // Remove, update and re-insert the item in the heap
    if (i < 0)
        return TIMEBASE_WATCHDOG_NOT_REGISTERED;

    (void)min_heap_remove(&htimebase.wdg_running, i, NULL);
    
    aux.t = htimebase.t + watchdog->timeout;
    if (min_heap_insert(&htimebase.wdg_running, &aux) == MIN_HEAP_FULL)
        return TIMEBASE_WATCHDOG_UNAVAILABLE;
    return TIMEBASE_OK;

}

// TODO: Check delta time between the right time?
TimebaseReturnCode timebase_routine(void) {
    if (!htimebase.enabled)
        return TIMEBASE_DISABLED;

    // Execute all the tasks which interval has already elapsed
    TimebaseScheduledTask * task_p = (TimebaseScheduledTask *)min_heap_peek(&htimebase.scheduled);
    while (task_p != NULL && task_p->t <= htimebase.t) {
        // Get and execute current task
        TimebaseScheduledTask task = { 0 };
        (void)min_heap_remove(&htimebase.scheduled, 0U, &task);

        ticks_t t = htimebase.t; // Copy ticks value to avoid inconsistencies caused by interrupts
        task.t = t + task.task->interval;
        task.task->exec();

        // If the interval is 0 do not insert again the task inside the heap (i.e. runs only once)
        if (task.task->interval > 0U)
            (void)min_heap_insert(&htimebase.scheduled, &task);

        task_p = (TimebaseScheduledTask *)min_heap_peek(&htimebase.scheduled);
    }

    // Check if the watchdogs has already timed-out
    TimebaseRunningWatchdog * wdg_p = (TimebaseRunningWatchdog *)min_heap_peek(&htimebase.wdg_running);
    while (wdg_p != NULL && wdg_p->t <= htimebase.t) {
        // Get the watchdog
        TimebaseRunningWatchdog wdg = { 0 };
        (void)min_heap_remove(&htimebase.wdg_running, 0U, &wdg);

        // Disable and execute the watchdog timeout callback
        watchdog_timeout(wdg.watchdog);
        wdg_p = (TimebaseRunningWatchdog *)min_heap_peek(&htimebase.wdg_running);
    }
    return TIMEBASE_OK;
}

#ifdef CONF_TIMEBASE_STRINGS_ENABLE

static char * timebase_module_name = "timebase";

static char * timebase_return_code_name[] = {
    [TIMEBASE_OK] = "ok",
    [TIMEBASE_NULL_POINTER] = "null pointer",
    [TIMEBASE_DISABLED] = "disabled",
    [TIMEBASE_BUSY] = "busy",
    [TIMEBASE_WATCHDOG_NOT_REGISTERED] = "watchdog not registered",
    [TIMEBASE_WATCHDOG_UNAVAILABLE] = "watchdog unavailable"
};

static char * timebase_return_code_description[] = {
    [TIMEBASE_OK] = "executed successfully",
    [TIMEBASE_NULL_POINTER] = "attempt to dereference a null pointer",
    [TIMEBASE_DISABLE] = "the timebase is not enabled",
    [TIMEBASE_BUSY] = "the timebase couldn't perform the requested operation",
    [TIMEBASE_WATCHDOG_NOT_REGISTERED] = "the watchdog is not registered",
    [TIMEBASE_WATCHDOG_UNAVAILABLE] = "the watchdog can't be registered inside the timebase"
};

#endif // CONF_TIMEBASE_STRINGS_ENABLE

#endif // CONF_TIMEBASE_MODULE_ENABLE_H
