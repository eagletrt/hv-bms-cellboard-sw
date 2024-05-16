/**
 * @file watchdog.h
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Implementation of generic watchdogs that times-out a certain interval of time 
 */

#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

/**
 * @brief Return code for the watchdog module functions
 *
 * @details
 *     - WATCHDOG_OK the function exeuted succesfully
 *     - WATCHDOG_NULL_POINTER a NULL pointer was given to a function
 */
typedef enum {
    WATCHDOG_OK,
    WATCHDOG_NULL_POINTER
} WatchdogReturnCode;

/** @brief Type definition for a function that is called when the watchdog times-out */
typedef void (* watchdog_timeout_callback)(void);

/**
 * @brief Definiton of the watchdog structure handler
 *
 * @param running True if the watchdog is running, false otherwise
 * @param timeout The number of ticks that should elapse for the watchdog to time-out
 * @param expire The function that is called when the watchdog times-out
 */
typedef struct {
    bool running;
    ticks timeout;
    watchdog_timeout_callback expire;
} Watchdog;

#ifdef CONF_WATCHDOG_MODULE_ENABLE

/**
 * @brief Initialize the watchdog
 *
 * @param watchdog A pointer to the watchdog handler structure
 * @param timeout The number of ticks that should elapse for the watchdog to time-out
 * @param expire The function that is called when the watchdog times-out
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog or the expire pointers are NULL
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_init(
    Watchdog * watchdog,
    ticks timeout,
    watchdog_timeout_callback expire
);

/**
 * @brief De-initialize the watchdog
 *
 * @param watchdog A pointer to the watchdog handler structure 
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog or the internal expire pointers are NULL
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_deinit(Watchdog * watchdog);

/**
 * @brief Start a watchdog
 *
 * @param watchdog A pointer to the watchdog handler structure
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_start(Watchdog * watchdog);

/**
 * @brief Stop a watchdog
 *
 * @param watchdog A pointer to the watchdog handler structure
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_stop(Watchdog * watchdog);

#else  // CONF_WATCHDOG_MODULE_ENABLE

#define watchdog_init(watchdog, timeout, expire) (WATCHDOG_OK)
#define watchdog_deinit(watchdog) (WATCHDOG_OK)
#define watchdog_start(watchdog) (WATCHDOG_OK)
#define watchdog_stop(watchdog) (WATCHDOG_OK)

#endif // CONF_WATCHDOG_MODULE_ENABLE
