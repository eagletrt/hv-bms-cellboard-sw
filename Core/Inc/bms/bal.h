/**
 * @file bal.h
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#ifndef BAL_H
#define BAL_H

#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"

#include "fsm.h"
#include "watchdog.h"

/** @brief Balancing threshold range in V */
#define BAL_THRESHOLD_MIN_V (0.005f)
#define BAL_THRESHOLD_MAX_V (0.200f)

/** @brief Balancing target range in mV */
#define BAL_TARGET_MIN_V (2.8f)
#define BAL_TARGET_MAX_V (4.2f)

/** @brief Balancing timeouts in ms */
#define BAL_TIMEOUT_MS (5000U)

/**
 * @brief Return code for the balancing module functions
 *
 * @details
 *     - BAL_OK the function executed succesfully
 *     - BAL_NULL_POINTER a NULL pointer was given to a function
 *     - BAL_BUSY the function cannot be executed because the LTCs are busy
 *     - BAL_WATCHDOG_ERROR there was an error during a balancing watchdog operation
 */
typedef enum {
    BAL_OK,
    BAL_NULL_POINTER,
    BAL_BUSY,
    BAL_WATCHDOG_ERROR
} BalReturnCode;

/**
 * @brief Balancing status
 *
 * @details
 *     - BAL_STATUS_STOPPED balancing is not active
 *     - BAL_STATUS_PAUSED balancing is active but the cells are not discharging
 *     - BAL_STATUS_DISCHARGING balancing is active and the cells are discharging
 */
typedef enum {
    BAL_STATUS_STOPPED = 0U,
    BAL_STATUS_PAUSED,
    BAL_STATUS_DISCHARCING 
} BalStatus;

/**
 * @brief Definition of the balancing parameters
 *
 * @param target The minimum target voltage that can be reached while discharging in V
 * @param threshold The minimum difference between the maximum and minimum cell voltage
 * value that can be reached in V
 */
typedef struct {
    volt_t target;
    volt_t threshold;
} BalParams;

/**
 * @brief Type definition for the balancing handler structure
 *
 * @attention This structure should not be used outside of this module
 *
 * @details The requested parameters are expected to be equals to the actual parameters
 *
 * @param event The FSM event data
 * @param status_can_payload The canlib payload of the balancing module
 * @param watchdog The watchdog that stops the balancing procedure when timed out
 * @param active True if the balancing is active, false otherwise
 * @param paused True if the balancing is paused, false otherwise
 * @param params The balancing parameters
 */
typedef struct {
    fsm_event_data_t event;
    bms_cellboard_balancing_status_converted_t status_can_payload;
    Watchdog watchdog;

    BalStatus status;
    BalParams params;
} _BalHandler;

#ifdef CONF_BALANCING_MODULE_ENABLE

/**
 * @brief Initialize the internal balancing handler structure
 *
 * @return BalReturnCode
 *     - BAL_OK
 */
BalReturnCode bal_init(void);

/**
 * @brief Handler the information received inside the canlib payload
 *
 * @param payload A pointer to the CAN paylaod data
 */
void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * const payload);

/**
 * @brief Check if the balancing is active
 *
 * @details The balancing is considered active even if it is not discharging
 *
 * @return bool True if the balancing procedure is running, false otherwise
 */
bool bal_is_active(void);

/**
 * @brief Check if the balancing is paused
 *
 * @return bool True if the balancing procedure is paused, false otherwise
 */
bool bal_is_paused(void);

/**
 * @brief Start the balancing mechanism
 *
 * @return BalReturnCode
 *     - BAL_WATCHDOG_ERROR the interal watchdog could not be started
 *     - BAL_OK otherwise
 */
BalReturnCode bal_start(void);

/**
 * @brief Stop the balancing mechanism
 *
 * @return BalReturnCode
 *     - BAL_OK
 */
BalReturnCode bal_stop(void);

/**
 * @brief Pause the discharge
 *
 * @details The balancing procedure can be paused only if active
 * @details The balancing procedure remains active even if it is paused
 *
 * @return BalReturnCode
 *     - BAL_OK
 */
BalReturnCode bal_pause(void);

/**
 * @brief Resume the discharge
 *
 * @details The balancing procedure can be resumed only if active
 *
 * @return BalReturnCode
 *     - BAL_OK
 */
BalReturnCode bal_resume(void);

/**
 * @brief Get a pointer to the CAN payload of the balancing info
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_balancing_status_converted_t* A pointer to the payload
 */
bms_cellboard_balancing_status_converted_t * bal_get_status_canlib_payload(size_t * const byte_size);

#else  // CONF_BALANCING_MODULE_ENABLE

#define bal_init() (BAL_OK)
#define bal_set_balancing_status_handle(payload) (NULL)
#define bal_set_balancing_status_handle (NULL)
#define bal_is_active() (false)
#define bal_is_paused() (false)
#define bal_start() (BAL_OK)
#define bal_stop() (BAL_OK)
#define bal_pause() (BAL_OK)
#define bal_resume() (BAL_OK)
#define bal_get_status_canlib_payload(byte_size) (NULL)

#endif // CONF_BALANCING_MODULE_ENABLE

#endif  // BAL_H
