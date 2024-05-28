/**
 * @file timebase.h
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to manage periodic tasks at certain intervals
 */

#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>
#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"
#include "watchdog.h"

/**
 * @brief Convert the time in ms to ticks
 *
 * @param T The time to convert
 * @param RES The resolution of a tick
 *
 * @return ticks_t The corresponing amount of ticks
 */
#define TIMEBASE_TIME_TO_TICKS(T, RES) ((ticks_t)((T) / (RES)))

/**
 * @brief Convert the ticks in ms
 *
 * @param T The ticks to convert
 * @param RES The resolution of a tick
 *
 * @return milliseconds_t The corresponing amount of ms
 */
#define TIMEBASE_TICKS_TO_TIME(T, RES) ((milliseconds_t)((T) * (RES)))

/** @brief Maximum number of watchdogs that can be handled simultaneously */
#define TIMEBASE_RUNNING_WATCHDOG_COUNT (16U)

/**
 * @brief Return code for the timebase module functions
 *
 * @details
 *     - TIMEBASE_OK the function executed successfully
 *     - TIMEBASE_NULL_POINTER a NULL pointer was given to a function
 *     - TIMEBASE_DISABLED the timebase is not running
 *     - TIMEBASE_BUSY the timebase cannot perform the current operation because is busy with other actions
 *     - TIMEBASE_WATCHDOG_NOT_REGISTERED the watchdog is not registered inside the timebase
 *     - TIMEBASE_WATCHDOG_UNAVAILABLE the given watchdog can't be registered inside the timebase
 */
typedef enum {
    TIMEBASE_OK,
    TIMEBASE_NULL_POINTER,
    TIMEBASE_DISABLED,
    TIMEBASE_BUSY,
    TIMEBASE_WATCHDOG_NOT_REGISTERED,
    TIMEBASE_WATCHDOG_UNAVAILABLE
} TimebaseReturnCode;

#ifdef CONF_TIMEBASE_MODULE_ENABLE

/**
 * @brief Initialize the timebase handler
 *
 * @param resolution The amount of time that represent one tick (in ms)
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_NULL_POINTER if a tasks is not implemented
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_init(milliseconds_t resolution_ms);

/**
 * @brief Enable or disable the timebase
 *
 * @param enabled True to enable the timebase false to disable it
 */
void timebase_set_enable(bool enabled);

/**
 * @brief Increment the internal timebase by one tick
 * 
 * @return TimebaseReturnCode
 *     - TIMEBASE_DISABLED if the timebase is disabled
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_inc_tick(void);

/**
 * @brief Get the current number of ticks
 *
 * @return ticks_t The number of ticks
 */
ticks_t timebase_get_tick(void);

/**
 * @brief Get the current elapsed time in ms
 *
 * @return milliseconds_t The current elapsed time
 */
milliseconds_t timebase_get_time(void);

/**
 * @brief Get the number of ms that represents a single tick
 *
 * @return milliseconds_t The timebase resolution
 */
milliseconds_t timebase_get_resolution(void);

/**
 * @brief Register a watchdog into the timebase
 *
 * @details If the timebase is not enable the watchdog is started but not updated
 *
 * @param Watchdog A pointer to the watchdog handler structure
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_NULL_POINTER if the watchdog is NULL
 *     - TIMEBASE_BUSY if the watchdog is already running
 *     - TIMEBASE_WATCHDOG_UNAVAILABLE if the timebase can't handle the watchdog
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_register_watchdog(Watchdog * watchdog);

/**
 * @brief Unregister a watchdog from the timebase
 *
 * @details The watchdog is unregistered even if the timebase is not enabled
 *
 * @param watchdog The watchdog handler structure
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_NULL_POINTER if the watchdog pointer is NULL
 *     - TIMEBASE_WATCHDOG_NOT_REGISTERED the watchdog is not registered
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_unregister_watchdog(Watchdog * handler);

/**
 * @brief Check if the watchdog is registered into the timebase
 *
 * @details A NULL pointer is counted as NOT registered
 *
 * @param watchdog A pointer to the watchdog to check
 *
 * @return bool True if the watchdog is registered, false otherwise
 */
bool timebase_is_registered_watchdog(Watchdog * watchdog);

/**
 * @brief Update the registered watchdog
 *
 * @attention During the update process the watchdog is unregistered and than registered again
 * if the function returns TIMEBASE_WATCHDOG_UNAVAILABLE that means that the watchdog was
 * unregistered but cannot be registered back (i.e. is not running anymore)
 *
 * @param watchdog A pointer to the watchdog
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_NULL_POINTER if the watchdog is NULL
 *     - TIMEBASE_WATCHDOG_NOT_REGISTERED the watchdog is not registered
 *     - TIMEBASE_WATCHDOG_UNAVAILABLE if the watchdog can't be registered again
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_update_watchdog(Watchdog * watchdog);

/**
 * @brief Routine that checks which functions shuold run during this
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_DISABLED if the timebase is disabled
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_routine(void);

#else  // CONF_TIMEBASE_MODULE_ENABLE

#define timebase_init(resolution) (TIMEBASE_OK)
#define timebase_set_enable() CELLBOARD_NOPE()
#define timebase_inc_tick() (TIMEBASE_OK)
#define timebase_get_tick() (0U)
#define timebase_get_time() (0U)
#define timebase_get_resolution() (1U) // The default value of 1 is used to avoid 0 division error
#define timebase_regsiter_watchdog(watchdog) (TIMEBASE_OK)
#define timebase_unregsiter_watchdog(watchdog) (TIMEBASE_OK)
#define timebase_update_watchdog(watchdog) (TIMEBASE_OK)
#define timebase_routine() (TIMEBASE_OK)

#endif // CONF_TIMEBASE_MODULE_ENABLE

#endif  // TIMEBASE_H
