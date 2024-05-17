/**
 * @file watchdog.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Implementation of generic watchdogs that times-out a certain interval of time 
 */

#include "watchdog.h"

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

    watchdog->running = false;
    watchdog->timeout = timeout;
    watchdog->expire = expire;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_deinit(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    
    watchdog->running = false;
    watchdog->timeout = 0U;
    watchdog->expire = _watchdog_timeout_dummy;
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_start(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (watchdog->running)
        return WATCHDOG_OK;
    
    watchdog->running = true;
    timebase_register_watchdog(watchdog);
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_stop(Watchdog * watchdog) {
    if (watchdog == NULL)
        return WATCHDOG_NULL_POINTER;
    if (!watchdog->running)
        return WATCHDOG_OK;
    
    timebase_unregister_watchdog(watchdog);
    watchdog->running = false;
    return WATCHDOG_OK;
}

#else  // CONF_WATCHDOG_MODULE_ENABLE

#ifdef CONF_WATCHDOG_STRINGS_ENABLE

static char * watchdog_module_name = "watchdog";

static char * watchdog_return_code_name[] = {
    [WATCHDOG_OK] = "ok",
    [WATCHDOG_NULL_POINTER] = "null pointer"
};

static char * watchdog_return_code_name[] = {
    [WATCHDOG_OK] = "executed sucessfully",
    [WATCHDOG_NULL_POINTER] = "attempt to dereference a null pointer"
};

#endif // CONF_WATCHDOG_STRINGS_ENABLE

#endif // CONF_WATCHDOG_MODULE_ENABLE
