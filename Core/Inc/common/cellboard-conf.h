/**
 * @file cellboard-conf.h
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Main configuration for the cellboard project
 *
 * @attention Do not touch unless you known whay you are doing
 */

#ifndef CELLBOARD_CONF_H
#define CELLBOARD_CONF_H

/*** ######################### LIBRARIES MACROS ########################## ***/

/**
 * @defgroup libmacros
 * @brief Macros used by the libraries included inside the project
 */
#define bms_NETWORK_IMPLEMENTATION

/** @} */

/*** ######################### MODULE SELECTION ########################## ***/

/**
 * @defgroup modules
 * @brief Enable or disable the internal modules of the project
 * {@
 */
#define CONF_POST_MODULE_ENABLE
#define CONF_IDENTITY_MODULE_ENABLE
#define CONF_TIMEBASE_MODULE_ENABLE
#define CONF_TASKS_MODULE_ENABLE
#define CONF_VOLTAGE_MODULE_ENABLE
#define CONF_TEMPERATURE_MODULE_ENABLE
#define CONF_CAN_COMM_MODULE_ENABLE
#define CONF_WATCHDOG_MODULE_ENABLE
#define CONF_BALANCING_MODULE_ENABLE
#define CONF_BMS_MANAGER_MODULE_ENABLE
#define CONF_LED_MODULE_ENABLE
#define CONF_ERROR_MODULE_ENABLE

/** @} */

/*** ######################### STRINGS INFORMATION ####################### ***/

/**
 * @defgroup strings
 * @brief Add useful strings that can be printed to get more info
 * {@
 */
// #define CONF_POST_STRINGS_ENABLE
// #define CONF_IDENTITY_STRINGS_ENABLE
// #define CONF_TIMEBASE_STRINGS_ENABLE
// #define CONF_TASKS_STRINGS_ENABLE
// #define CONF_VOLTAGE_STRINGS_ENABLE
// #define CONF_TEMPERATURE_STRINGS_ENABLE
// #define CONF_CAN_COMM_STRINGS_ENABLE
// #define CONF_WATCHDOG_STRINGS_ENABLE
// #define CONF_BALANCING_STRINGS_ENABLE
// #define CONF_BMS_MANAGER_STRINGS_ENABLE
// #define CONF_LED_STRINGS_ENABLE
// #define CONF_ERROR_STRINGS_ENABLE

/** @} */

/*** ######################### DEBUG INFORMATION ####################### ***/

/**
 * @defgroup debug
 * @brief Add useful debug utilities
 * {@
 */

// Enable small demo to test all cellboard functionality
#define CONF_DEMO_ENABLE

// Enable custom asserts
#define CONF_FULL_ASSERT_ENABLE

// Enable manual discharge
// #define CONF_MANUAL_DISCHARGE_ENABLE

/** @} */

#endif  // CELLBOARD_CONF_H
