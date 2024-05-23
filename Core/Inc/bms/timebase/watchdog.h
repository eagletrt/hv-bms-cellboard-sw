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
 *     - WATCHDOG_BUSY the watchdog is already running
 *     - WATCHDOG_TIMED_OUT the watchdog has timed out
 *     - WATCHDOG_NOT_RUNNING the watchdog is not running
 *     - WATCHDOG_UNAVAILABLE the watchdog is not registered inside the timebase
 */
typedef enum {
    WATCHDOG_OK,
    WATCHDOG_NULL_POINTER,
    WATCHDOG_BUSY,
    WATCHDOG_TIMED_OUT,
    WATCHDOG_NOT_RUNNING,
    WATCHDOG_UNAVAILABLE
} WatchdogReturnCode;

/**
 * @brief Type definition for a function that is called when the watchdog times-out
 * 
 * @details When the watchdog times-out it unregister itself from the timebase automatically
 */
typedef void (* watchdog_timeout_callback)(void);

/**
 * @brief Definiton of the watchdog structure handler
 *
 * @param running True if the watchdog is running, false otherwise
 * @param timed_out True if the watchdog is running, false otherwise
 * @param timeout The number of ticks that should elapse for the watchdog to time-out
 * @param expire The function that is called when the watchdog times-out
 */
typedef struct {
    bool running;
    bool timed_out;
    ticks_t timeout;
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
 *     - WATCHDOG_BUSY if the watchdog is already running
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_init(
    Watchdog * watchdog,
    ticks_t timeout,
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
 * @details A timed out watchdog cannot be started
 *
 * @param watchdog A pointer to the watchdog handler structure
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_BUSY if the watchdog is already running
 *     - WATCHDOG_TIMED_OUT if the watchdog has already timed out
 *     - WATCHDOG_UNAVAILABLE if the watchdog can't be registered
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_start(Watchdog * watchdog);

/**
 * @brief Stop a watchdog
 *
 * @details A timed out watchdog cannot be stopped
 *
 * @param watchdog A pointer to the watchdog handler structure
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_NOT_RUNNING if the watchdog is not running
 *     - WATCHDOG_TIMED_OUT if the watchdog has already timed out
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_stop(Watchdog * watchdog);

/**
 * @brief Start a watchdog even if it has timed out
 *
 * @details If the watchdog is not running it is started
 * as the watchdog start function
 *
 * @param A pointer to the watchdog handler structure
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_UNAVAILABLE if the watchdog can't be registered
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_restart(Watchdog * watchdog);

/**
 * @brief Reset the watchdog internal time to 0
 *
 * @details The watchdog is not stopped after the reset
 *
 * @param watchdog A pointer to the watchdog
 *
 * @return WatchogReturnCode
 *     - WATCHDOG_NULL_POINTER if the watchdog is NULL
 *     - WATCHDOG_NOT_RUNNING if the watchdog is not running
 *     - WATCHDOG_TIMED_OUT if the watchdog has already timed out
 *     - WATCHDOG_UNAVAILABLE if the watchdog can't be registered inside the timebase
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_reset(Watchdog * watchdog);

/**
 * @brief Set the watchdog status as timed out
 *
 * @param watchdog A pointer to the watchdog
 *
 * @return WatchdogReturnCode
 *     - WATCHDOG_NULL_POINTER A NULL pointer was given as parameter
 *     - WATCHDOG_NOT_RUNNING if the watchdog is not running
 *     - WATCHDOG_TIMED_OUT if the watchdog has already timed out
 *     - WATCHDOG_OK otherwise
 */
WatchdogReturnCode watchdog_timeout(Watchdog * watchdog);

/**
 * @param Check if the watchdog has timed out
 *
 * @param watchdog A pointer to the watchdog
 *
 * @return bool True if the watchdog has timed out, false otherwise
 */
bool watchdog_is_timed_out(Watchdog * watchdog);

#else  // CONF_WATCHDOG_MODULE_ENABLE

#define watchdog_init(watchdog, timeout, expire) (WATCHDOG_OK)
#define watchdog_deinit(watchdog) (WATCHDOG_OK)
#define watchdog_start(watchdog) (WATCHDOG_OK)
#define watchdog_stop(watchdog) (WATCHDOG_OK)
#define watchdog_restart(watchdog) (WATCHDOG_OK)
#define watchdog_reset(watchdog) (WATCHDOG_OK)
#define watchdog_timeout(watchdog) (WATCHDOG_OK)
#define watchdog_is_timed_out(watchdog) (false)

#endif // CONF_WATCHDOG_MODULE_ENABLE
