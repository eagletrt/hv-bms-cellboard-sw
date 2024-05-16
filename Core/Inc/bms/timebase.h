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

/** @brief Maximum number of watchdogs available */
#define TIMEBASE_WATCHDOG_COUNT (16U)

/** @brief Type definition for a function that is called if a watchdog times-out */
typedef void (* timebase_watchdog_timeout_callback)(void);

/**
 * @brief Return code for the timebase module functions
 *
 * @details
 *     - TIMEBASE_OK the function executed successfully
 *     - TIMEBASE_NULL_POINTER a NULL pointer was given to a function
 *     - TIMEBASE_DISABLED the timebase is not running
 *     - TIMEBASE_BUSY the timebase cannot perform the current operation because is busy with other actions
 */
typedef enum {
    TIMEBASE_OK,
    TIMEBASE_NULL_POINTER,
    TIMEBASE_DISABLED,
    TIMEBASE_BUSY
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
TimebaseReturnCode timebase_init(time resolution_ms);

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
 * @return ticks The number of ticks
 */
ticks timebase_get_tick(void);

/**
 * @brief Get the current elapsed time in ms
 *
 * @return time The current elapsed time
 */
time timebase_get_time(void);

/**
 * @brief Get the number of ms that represents a single tick
 *
 * @return time The timebase resolution
 */
time timebase_get_resolution(void);

/**
 * @brief Start a watchdog
 *
 * @details The watchdog is started even if the timebase is not enabled
 * but it will not run until the timebase is enabled again
 *
 * @param timeout The duration of the watchdog in ticks
 * @param expire A pointer to the function that is called when the watchdog times-out
 *
 * @return TimebaseReturnCode
 *     - TIMEBASE_NULL_POINTER if the function callback pointer is NULL
 *     - TIMEBASE_BUSY if there are no available watchdogs
 *     - TIMEBASE_OK otherwise
 */
TimebaseReturnCode timebase_watchdog_start(ticks timeout, timebase_watchdog_timeout_callback expired);

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
#define timebase_watchdog_start(timeout, expire) (TIMEBASE_OK)
#define timebase_routine() (TIMEBASE_OK)

#endif // CONF_TIMEBASE_MODULE_ENABLE

#endif  // TIMEBASE_H
