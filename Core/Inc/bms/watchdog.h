/**
 * @file watchdog.h
 * @date 2024-04-29
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used to constantly check for the presence of some signals that should
 * always be present
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#ifdef DO_NOT_DEFINE

/**
 * @brief Return code for the watchdog module functions
 *
 * @details
 *      - WATCHDOG_OK the function exeuted succesfully
 *      - WATCHDOG_INVALID_ID a function tried to use an invalid identifier
 *      - WATCHDOG_TIMEOUT a watchdog has timed out
 */
typedef enum {
    WATCHDOG_OK,
    WATCHDOG_INVALID_ID,
    WATCHDOG_TIMEOUT
} WatchdogReturnCode;

#ifdef DO_NOT_DEFINE

/**
 * @brief Initialize the watchdogs
 *
 * @details The watchdog is not started during initialization
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_OK
 */
WatchdogReturnCode watchdog_init(void);

/**
 * @brief Check if the watchdog is active
 *
 * @details If the ID is not a valid identifier false is returned
 *
 * @param index The watched CAN index
 *
 * @return bool True if the watchdog is active, false otherwise
 */
bool watchdog_is_active(can_index index);

/**
 * @brief Check if the watchdog is timed out
 *
 * @details If the ID is not a valid identifier false is returned
 *
 * @param index The watched CAN index
 *
 * @return bool True if the watchdog is timed out, false otherwise
 */
bool watchdog_is_timed_out(can_index index);

/**
 * @brief Start the selected watchdog 
 *
 * @param index The watched CAN index
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_INVALID_ID if the watched ID is not valid
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_start(can_index index);

/**
 * @brief Start all the watchdogs
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_OK
 */
WatchdogReturnCode watchdog_start_all(void);

/**
 * @brief Stop the selected watchdog 
 *
 * @param index The watched CAN index
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_OK
 */
WatchdogReturnCode watchdog_stop(can_index index);

/**
 * @brief Stop all the watchdogs
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_INVALID_ID if a watched ID is not valid
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_stop_all(void);

/**
 * @brief Reset the watchdog if activated to prevent timeout
 *
 * @param index The watched CAN index
 * @param timestamp The current time (in ms)
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_INVALID_ID if the given ID is not valid
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_reset(can_index index, time timestamp);

/**
 * @brief Routine that checks if the watchdogs have timed out
 *
 * @param timestamp The current time (in ms)
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_TIMEOUT if a watchdog has timed out
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_routine(time timestamp);

#else  // CONF_WATCHDOG_MODULE_ENABLE

#define watchdog_init() (WATCHDOG_OK)
#define watchdog_is_active(index) (false)
#define watchdog_is_timed_out(index) (false)
#define watchdog_start(index) (WATCHDOG_OK)
#define watchdog_start_all() (WATCHDOG_OK)
#define watchdog_stop(index) (WATCHDOG_OK)
#define watchdog_stop_all() (WATCHDOG_OK)
#define watchdog_reset(index, timestamp) (WATCHDOG_OK)
#define watchdog_routine(timestamp) (WATCHDOG_OK)

#endif // CONF_WATCHDOG_MODULE_ENABLE

#endif  // WATCHDOG_H

#endif
