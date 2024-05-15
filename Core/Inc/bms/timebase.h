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

/**
 * @brief Return code for the timebase module functions
 *
 * @details
 *     - TIMEBASE_OK the function executed successfully
 *     - TIMEBASE_DISABLED the timebase is not running
 *     - TIMEBASE_BUSY the timebase cannot perform the current operation because is busy with other actions
 */
typedef enum {
    TIMEBASE_OK,
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
 *     - TIMEBASE_OK
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
#define timebase_routine() (TIMEBASE_OK)

#endif // CONF_TIMEBASE_MODULE_ENABLE

#endif  // TIMEBASE_H
