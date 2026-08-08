/**
 * @file timebase.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to manage periodic tasks at certain intervals
 */

#include "timebase.h"

#include <string.h>

#include "eagletrt.h"
#include "arena-allocator-api.h"
#include "min-heap-api.h"

#ifdef CONF_TIMEBASE_MODULE_ENABLE

EAGLETRT_STATIC struct TimebaseHandler htimebase;

int8_t prv_timebase_task_compare(void *first, void *second) {
    const struct TimebaseScheduledTask *const p_first = (struct TimebaseScheduledTask *)first;
    const struct TimebaseScheduledTask *const p_second = (struct TimebaseScheduledTask *)second;

    // Compare timestamps
    if (p_first->t < p_second->t) {
        return -1;
    }
    if (p_first->t > p_second->t) {
        return 1;
    }

    /**************************************************************************
     * For the equality check, in addition to the ticks, the pointers to the
     * task must also be equal, otherwise -1 or 1 may be returned
     * In this case 1 is preferred because it avoid useless swaps between
     * elements that have the same number of ticks
     ***************************************************************************/
    if (p_first->task == p_second->task) {
        return 0;
    }
    return 1;
}

int8_t prv_timebase_watchdog_compare(void *first, void *second) {
    const struct TimebaseScheduledWatchdog *p_first = (struct TimebaseScheduledWatchdog *)first;
    const struct TimebaseScheduledWatchdog *p_second = (struct TimebaseScheduledWatchdog *)second;

    /**************************************************************************
     * For the equality check, only the pointers to the watchdogs are checked
     * This can cause problems if multiple instances of the same watchdog are
     * inserted in the heap, but in this case a watchdog can be inserted inside
     * the heap only once
     ***************************************************************************/
    if (p_first->watchdog == p_second->watchdog) {
        return 0;
    }

    // Compare timestamps
    if (p_first->t < p_second->t) {
        return -1;
    }
    return p_first->t == p_second->t ? 0 : 1;
}

enum TimebaseReturnCode timebase_init(const milliseconds_t resolution_ms) {
    // Initialize timebase to 0
    memset(&htimebase, 0U, sizeof(htimebase));

    // Set default parameters
    htimebase.enabled = false;
    // NOLINTNEXTLINE: narrowing conversion is detected with same type
    htimebase.resolution = (resolution_ms == 0U) ? 1U : resolution_ms;

    // Initialize the tasks
    if (tasks_init(resolution_ms) != TASKS_RC_OK) {
        return TIMEBASE_RC_TASK_ERROR;
    }

    // Setup scheduled tasks buffer
    arena_allocator_api_init(&htimebase.arena);
    const enum MinHeapReturnCode tasks_rc = min_heap_api_init(
        &htimebase.scheduled_tasks,
        sizeof(struct TimebaseScheduledTask),
        TASKS_COUNT,
        prv_timebase_task_compare,
        &htimebase.arena);
    if (tasks_rc != MIN_HEAP_RC_OK) {
        return TIMEBASE_RC_BUFFER_ERROR;
    }

    // Schedule tasks
    for (size_t i = 0; i < TASKS_COUNT; ++i) {
        struct TimebaseScheduledTask aux = {
            .t = tasks_get_start(i),
            .task = tasks_get_task(i)
        };
        if (min_heap_api_insert(&htimebase.scheduled_tasks, &aux) != MIN_HEAP_RC_OK) {
            return TIMEBASE_RC_BUFFER_ERROR;
        }
    }

    // Setup scheduled watchdogs buffer
    const enum MinHeapReturnCode watchdogs_rc = min_heap_api_init(
        &htimebase.scheduled_watchdogs,
        sizeof(struct TimebaseScheduledWatchdog),
        TIMEBASE_RUNNING_WATCHDOG_COUNT,
        prv_timebase_watchdog_compare,
        &htimebase.arena);
    if (watchdogs_rc != MIN_HEAP_RC_OK) {
        return TIMEBASE_RC_BUFFER_ERROR;
    }
    return TIMEBASE_RC_OK;
}

void timebase_set_enable(const bool enabled) {
    htimebase.enabled = enabled;
}

enum TimebaseReturnCode timebase_inc_tick(void) {
    if (!htimebase.enabled) {
        return TIMEBASE_RC_DISABLED;
    }
    ++htimebase.t;
    return TIMEBASE_RC_OK;
}

ticks_t timebase_get_tick(void) {
    return htimebase.t;
}

milliseconds_t timebase_get_time(void) {
    return TIMEBASE_TICKS_TO_MS(htimebase.t, htimebase.resolution);
}

milliseconds_t timebase_get_resolution(void) {
    return htimebase.resolution;
}

enum TimebaseReturnCode timebase_register_watchdog(Watchdog *const watchdog) {
    if (watchdog == NULL) {
        return TIMEBASE_RC_NULL_POINTER;
    }

    struct TimebaseScheduledWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    if (min_heap_api_find(&htimebase.scheduled_watchdogs, &aux) >= 0) {
        return TIMEBASE_RC_BUSY;
    }

    aux.t = htimebase.t + TIMEBASE_MS_TO_TICKS(watchdog->timeout, htimebase.resolution);
    if (min_heap_api_insert(&htimebase.scheduled_watchdogs, &aux) == MIN_HEAP_RC_FULL) {
        return TIMEBASE_RC_WATCHDOG_UNAVAILABLE;
    }
    return TIMEBASE_RC_OK;
}

enum TimebaseReturnCode timebase_unregister_watchdog(Watchdog *const watchdog) {
    if (watchdog == NULL) {
        return TIMEBASE_RC_NULL_POINTER;
    }

    // Get and remove the running watchdog from the heap
    struct TimebaseScheduledWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    const int16_t index = min_heap_api_find(&htimebase.scheduled_watchdogs, &aux);
    if (index < 0) {
        return TIMEBASE_RC_WATCHDOG_NOT_REGISTERED;
    }
    if (min_heap_api_remove(&htimebase.scheduled_watchdogs, index, NULL) != MIN_HEAP_RC_OK) {
        return TIMEBASE_RC_BUFFER_ERROR;
    }
    return TIMEBASE_RC_OK;
}

bool timebase_is_registered_watchdog(Watchdog *const watchdog) {
    if (watchdog == NULL) {
        return false;
    }

    // Get the running watchdog
    struct TimebaseScheduledWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    return min_heap_api_find(&htimebase.scheduled_watchdogs, &aux) >= 0;
}

enum TimebaseReturnCode timebase_update_watchdog(Watchdog *const watchdog) {
    if (watchdog == NULL) {
        return TIMEBASE_RC_NULL_POINTER;
    }

    // Get the running watchdog
    struct TimebaseScheduledWatchdog aux = {
        .t = 0U,
        .watchdog = watchdog
    };
    const int16_t index = min_heap_api_find(&htimebase.scheduled_watchdogs, &aux);
    // Remove, update and re-insert the item in the heap
    if (index < 0) {
        return TIMEBASE_RC_WATCHDOG_NOT_REGISTERED;
    }

    if (min_heap_api_remove(&htimebase.scheduled_watchdogs, index, NULL) != MIN_HEAP_RC_OK) {
        return TIMEBASE_RC_BUFFER_ERROR;
    }
    aux.t = htimebase.t + watchdog->timeout;
    if (min_heap_api_insert(&htimebase.scheduled_watchdogs, &aux) == MIN_HEAP_RC_FULL) {
        return TIMEBASE_RC_WATCHDOG_UNAVAILABLE;
    }
    return TIMEBASE_RC_OK;
}

// TODO: Check delta time between the right time?
enum TimebaseReturnCode timebase_routine(void) {
    if (!htimebase.enabled) {
        return TIMEBASE_RC_DISABLED;
    }

    enum TimebaseReturnCode result = TIMEBASE_RC_OK;

    // Execute all the tasks which interval has already elapsed
    const struct TimebaseScheduledTask *task_p = (struct TimebaseScheduledTask *)min_heap_api_peek(&htimebase.scheduled_tasks);
    while (task_p != NULL && task_p->t <= htimebase.t) {
        // Get and execute current task
        struct TimebaseScheduledTask task = { 0 };
        if (min_heap_api_remove(&htimebase.scheduled_tasks, 0U, &task) != MIN_HEAP_RC_OK) {
            result = TIMEBASE_RC_BUFFER_ERROR;
            break;
        }

        // Copy ticks value to avoid inconsistencies caused by interrupts
        const ticks_t tick = htimebase.t;
        task.t = tick + task.task->interval;
        // TODO: Remove task from heap when disabled, and re-insert when re-enabled
        if (task.task->enabled) {
            task.task->exec();
        }

        // If the interval is 0 do not insert again the task inside the heap (i.e. runs only once)
        if (task.task->interval > 0U) {
            if (min_heap_api_insert(&htimebase.scheduled_tasks, &task) != MIN_HEAP_RC_OK) {
                result = TIMEBASE_RC_BUFFER_ERROR;
                break;
            }
        }

        task_p = (struct TimebaseScheduledTask *)min_heap_api_peek(&htimebase.scheduled_tasks);
    }

    // Check if the watchdogs has already timed-out
    const struct TimebaseScheduledWatchdog *wdg_p = (struct TimebaseScheduledWatchdog *)min_heap_api_peek(&htimebase.scheduled_watchdogs);
    while (wdg_p != NULL && wdg_p->t <= htimebase.t) {
        // Get the watchdog
        struct TimebaseScheduledWatchdog wdg = { 0 };
        if (min_heap_api_remove(&htimebase.scheduled_watchdogs, 0U, &wdg) != MIN_HEAP_RC_OK) {
            result = TIMEBASE_RC_BUFFER_ERROR;
            break;
        }

        // Disable and execute the watchdog timeout callback
        watchdog_timeout(wdg.watchdog);
        wdg_p = (struct TimebaseScheduledWatchdog *)min_heap_api_peek(&htimebase.scheduled_watchdogs);
    }
    return result;
}

#ifdef CONF_TIMEBASE_STRINGS_ENABLE

EAGLETRT_STATIC char *timebase_module_name = "timebase";

EAGLETRT_STATIC char *timebase_return_code_name[] = {
    [TIMEBASE_OK] = "ok",
    [TIMEBASE_NULL_POINTER] = "null pointer",
    [TIMEBASE_DISABLED] = "disabled",
    [TIMEBASE_BUSY] = "busy",
    [TIMEBASE_WATCHDOG_NOT_REGISTERED] = "watchdog not registered",
    [TIMEBASE_WATCHDOG_UNAVAILABLE] = "watchdog unavailable"
};

EAGLETRT_STATIC char *timebase_return_code_description[] = {
    [TIMEBASE_OK] = "executed successfully",
    [TIMEBASE_NULL_POINTER] = "attempt to dereference a null pointer",
    [TIMEBASE_DISABLE] = "the timebase is not enabled",
    [TIMEBASE_BUSY] = "the timebase couldn't perform the requested operation",
    [TIMEBASE_WATCHDOG_NOT_REGISTERED] = "the watchdog is not registered",
    [TIMEBASE_WATCHDOG_UNAVAILABLE] = "the watchdog can't be registered inside the timebase"
};

#endif // CONF_TIMEBASE_STRINGS_ENABLE

#endif // CONF_TIMEBASE_MODULE_ENABLE_H
