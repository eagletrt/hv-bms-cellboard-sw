/**
 * @file bal.h
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#ifndef BAL_H
#define BAL_H

#include "cellboard-conf.h"

/**
 * @brief Return code for the balancing module functions
 *
 * @details
 *     BAL_OK the function executed succesfully
 */
typedef enum {
    BAL_OK
} BalReturnCode;

#ifdef CONF_BALANCING_MODULE_ENABLE

BalReturnCode bal_init(void);

#else  // CONF_BALANCING_MODULE_ENABLE

#define bal_init() (BAL_OK)

#endif // CONF_BALANCING_MODULE_ENABLE

#endif  // BAL_H
