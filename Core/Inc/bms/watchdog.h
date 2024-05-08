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


/**
 * @brief Return code for the watchdog module functions
 *
 * @details
 *      WATCHDOG_OK the function exeuted succesfully
 *      WATCHDOG_INVALID_ID a function tried to use an invalid identifier
 *      WATCHDOG_TIMEOUT a watchdog has timed out
 */
typedef enum {
    WATCHDOG_OK,
    WATCHDOG_INVALID_ID,
    WATCHDOG_TIMEOUT
} WatchdogReturnCode;

#ifdef CONF_WATCHDOG_MODULE_ENABLE

/**
 * @brief Initialize the watchdogs
 *
 * @details The watchdog is not started during initialization
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if the watched ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_init(void);

/**
 * @brief Check if the watchdog is active
 *
 * @details If the ID is not a valid identifier false is returned
 *
 * @param id The watched CAN identifier
 *
 * @return bool True if the watchdog is active, false otherwise
 */
bool watchdog_is_active(can_id id);

/**
 * @brief Check if the watchdog is timed out
 *
 * @details If the ID is not a valid identifier false is returned
 *
 * @param id The watched CAN identifier
 *
 * @return bool True if the watchdog is timed out, false otherwise
 */
bool watchdog_is_timed_out(can_id id);

/**
 * @brief Start the selected watchdog 
 *
 * @param id The watched CAN identifier
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if the watched ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_start(can_id id);

/**
 * @brief Start all the watchdogs
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if a watched ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_start_all(void);

/**
 * @brief Stop the selected watchdog 
 *
 * @param id The watched CAN identifier
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if the watched ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_stop(can_id id);

/**
 * @brief Stop all the watchdogs
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if a watched ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_stop_all(void);

/**
 * @brief Reset the watchdog if activated to prevent timeout
 *
 * @param id The watched CAN identifier
 * @param timestamp The current time (in ms)
 *
 * @return WatchdogReturnCode
 *     WATCHDOG_INVALID_ID if the given ID is not valid
 *     WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_reset(can_id id, time timestamp);

/**
 * @brief Routine that checks if the watchdogs have timed out
 *
 * @param timestamp The current time (in ms)
 */
WatchdogReturnCode watchdog_routine(time timestamp);

#else  // CONF_WATCHDOG_MODULE_ENABLE

#define watchdog_init() (WATCHDOG_OK)
#define watchdog_is_active(id) (false)
#define watchdog_is_timed_out(id) (false)
#define watchdog_start(id) (WATCHDOG_OK)
#define watchdog_start_all() (WATCHDOG_OK)
#define watchdog_stop(id) (WATCHDOG_OK)
#define watchdog_stop_all() (WATCHDOG_OK)
#define watchdog_reset(id, timestamp) (WATCHDOG_OK)
#define watchdog_routine(timestamp) (WATCHDOG_OK)

#endif // CONF_WATCHDOG_MODULE_ENABLE

#endif  // WATCHDOG_H
