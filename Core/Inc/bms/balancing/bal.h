/*!
 * \file bal.h
 * \date 2024-04-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Balancing handling functions
 */

#ifndef BAL_H
#define BAL_H

#include <stdbool.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"

#include "fsm.h"
#include "watchdog.h"

/*! \brief Balancing threshold range in V */
#define BAL_THRESHOLD_MIN_V (0.005f)
#define BAL_THRESHOLD_MAX_V (0.200f)

/*! \brief Balancing target range in mV */
#define BAL_TARGET_MIN_V (2.8f)
#define BAL_TARGET_MAX_V (4.2f)

/*! \brief Balancing timeouts in ms */
#define BAL_TIMEOUT_MS (5000U)

/*!
 * \brief Return code for the balancing module functions
 */
enum BalReturnCode {
    BAL_OK,            /*<! The function executed succesfully */
    BAL_NULL_POINTER,  /*<! A NULL pointer was given to a function */
    BAL_BUSY,          /*<! The function cannot be executed because the LTCs are busy */
    BAL_WATCHDOG_ERROR /*<! There was an error during a balancing watchdog operation */
};

/*!
 * \brief Balancing status
 */
enum BalStatus {
    BAL_STATUS_STOPPED = 0U, /*<! The balancing procedure is stopped */
    BAL_STATUS_PAUSED,       /*<! The balancing procedure is paused, the cells are not discharging */
    BAL_STATUS_DISCHARCING   /*<! The balancing procedure is active and the cells are discharging */
};

/*!
 * \brief Definition of the balancing parameters
 */
struct BalParams {
    volt target;    /*!< The minimum target voltage that can be reached while discharging in V */
    volt threshold; /*!< The minimum difference between the maximum and minimum cell voltage value that can be reached in V */
};

/*!
 * \brief Type definition for the balancing handler structure
 *
 * \attention This structure should not be used outside of this module
 *
 * \details The requested parameters are expected to be equals to the actual parameters
 */
struct BalHandler {
    fsm_event_data_t event;                                        /*!< The FSM event data */
    bms_cellboard_balancing_status_converted_t status_can_payload; /*!< The canlib payload of the balancing module */
    Watchdog watchdog;                                             /*!< The watchdog that stops the balancing procedure when timed out */

    enum BalStatus status;   /*!< The actual balancing status */
    struct BalParams params; /*!< The balancing parameters */
};

#endif // BAL_H