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
#include "bsm_network.h"

/** @brief Balancing threshold range in mV */
#define BAL_THRESHOLD_MIN ((millivolt_t)5.f)
#define BAL_THRESHOLD_MAX ((millivolt_t)200.f)

/** @brief Balancing target range in mV */
#define BAL_TARGET_MIN ((millivolt_t)2800.f)
#define BAL_TARGET_MAX ((millivolt_t)4200.f)

/**
 * @brief Return code for the balancing module functions
 *
 * @details
 *     - BAL_OK the function executed succesfully
 *     - BAL_NULL_POINTER a NULL pointer was given to a function
 *     - BAL_BUSY the function cannot be executed because the LTCs are busy
 */
typedef enum {
    BAL_OK,
    BAL_NULL_POINTER,
    BAL_BUSY
} BalReturnCode;

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
void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * payload);

/**
 * @brief Start the balancing mechanism
 *
 * @return BalReturnCode
 *     - BAL_BUSY the LTC is busy performing other operations
 *     - BAL_OK otherwise
 */
BalReturnCode bal_start(void);

/**
 * @brief Stop the balancing mechanism
 *
 * @return BalReturnCode
 *     - BAL_BUSY the LTC is busy performing other operations
 *     - BAL_OK otherwise
 */
BalReturnCode bal_stop(void);

#else  // CONF_BALANCING_MODULE_ENABLE

#define bal_init() (BAL_OK)
#define bal_set_balancing_status_handle(payload) CELLBOARD_NOPE()
#define bal_start() (BAL_OK)
#define bal_stop() (BAL_OK)

#endif // CONF_BALANCING_MODULE_ENABLE

#endif  // BAL_H
