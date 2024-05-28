/**
 * @file watchdog.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Implementation of generic watchdogs that times-out a certain interval of time 
 */

#include "watchdog.h"

#include <stddef.h>
#include <string.h>

#include "timebase.h"

#ifdef CONF_WATCHDOG_MODULE_ENABLE

/** @brief Dummy function to avoid NULL dereferencing */
void _watchdog_timeout_dummy(void) { }

WatchdogReturnCode watchdog_init(
    Watchdog * watchdog,
    ticks_t timeout,
    watchdog_timeout_callback expire)
{
    if (watchdog == NULL || expire == NULL)
        return WATCHDOG_NULL_POINTER;
    if (watchdog->running)
        return WATCHDOG_BUSY;

    memset(watchdog, 0U, sizeof(Watchdog));

    // Init watchdog
    watchdog->timeout = timeout;
    watchdog->expire = expire;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_deinit(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;

    // Unregister before deinit
    (void)timebase_unregister_watchdog(watchdog);
    
    memset(watchdog, 0U, sizeof(Watchdog));

    // Deinit watchdog
    watchdog->expire = _watchdog_timeout_dummy;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_start(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (watchdog->running)
        return WATCHDOG_BUSY; 
    if (watchdog->timed_out)
        return WATCHDOG_TIMED_OUT;
    
    // Start and register the watchdog to the timebase
    if (timebase_register_watchdog(watchdog) == TIMEBASE_WATCHDOG_UNAVAILABLE)
        return WATCHDOG_UNAVAILABLE;
    watchdog->running = true;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_stop(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (!watchdog->running)
        return WATCHDOG_NOT_RUNNING;
    if (watchdog->timed_out)
        return WATCHDOG_TIMED_OUT;
    
    // Stop and unregister the watchdog to the timebase
    (void)timebase_unregister_watchdog(watchdog);
    watchdog->running = false;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_restart(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    // Stop watchdog if its running
    if (watchdog->running) {
        (void)timebase_unregister_watchdog(watchdog);
        watchdog->running = false;
    }
    
    // Start the watchdog
    if (timebase_register_watchdog(watchdog) == TIMEBASE_WATCHDOG_UNAVAILABLE)
        return WATCHDOG_UNAVAILABLE;
    watchdog->timed_out = false;
    watchdog->running = true;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_reset(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (watchdog->timed_out)
        return WATCHDOG_TIMED_OUT;
    if (!watchdog->running)
        return WATCHDOG_NOT_RUNNING;
    
    // Update the watchdog registered in the timebase
    if (timebase_update_watchdog(watchdog) == TIMEBASE_WATCHDOG_UNAVAILABLE)
        return WATCHDOG_UNAVAILABLE;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_timeout(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (!watchdog->running)
        return WATCHDOG_NOT_RUNNING;
    if (watchdog->timed_out)
        return WATCHDOG_TIMED_OUT;
    // Update data
    watchdog->timed_out = true;
    watchdog->running = false;
    watchdog->expire();
    return WATCHDOG_OK;
}

bool watchdog_is_timed_out(Watchdog * watchdog) {
    if (watchdog == NULL)
        return false;
    return watchdog->timed_out;
}

#else  // CONF_WATCHDOG_MODULE_ENABLE

#ifdef CONF_WATCHDOG_STRINGS_ENABLE

static char * watchdog_module_name = "watchdog";

static char * watchdog_return_code_name[] = {
    [WATCHDOG_OK] = "ok",
    [WATCHDOG_NULL_POINTER] = "null pointer",
    [WATCHDOG_BUSY] = "busy",
    [WATCHDOG_TIMED_OUT] = "timeout",
    [WATCHDOG_NOT_RUNNING] = "not running",
    [WATCHDOG_UNAVAILABLE] = "unavailable"
};

static char * watchdog_return_code_name[] = {
    [WATCHDOG_OK] = "executed sucessfully",
    [WATCHDOG_NULL_POINTER] = "attempt to dereference a null pointer"
    [WATCHDOG_BUSY] = "the watchdog is already running",
    [WATCHDOG_TIMED_OUT] = "the watchdog has timed-out",
    [WATCHDOG_NOT_RUNNING] = "the watchdog is not running",
    [WATCHDOG_UNAVAILABLE] = "the watchdog can't be registered"
};

#endif // CONF_WATCHDOG_STRINGS_ENABLE

#endif // CONF_WATCHDOG_MODULE_ENABLE
