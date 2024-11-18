/**
 * @file error.h
 * @date 2024-08-24
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Simple wrapper for the error handler generated code
 */

#ifndef ERROR_H
#define ERROR_H

#include "cellboard-def.h"
#include "cellboard-conf.h"

#include "bms_network.h"

#include "errorlib.h"
 
/** @brief Error instances count for each group */
#define ERROR_GROUP_POST_INSTANCE_COUNT (1U)
#define ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_CELLS_COUNT)
#define ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_CELLS_COUNT)
#define ERROR_GROUP_UNDER_TEMPERATURE_CELLS_INSTANCE_COUNT (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
#define ERROR_GROUP_OVER_TEMPERATURE_CELLS_INSTANCE_COUNT (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
#define ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_SERIES_COUNT)
#define ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_SERIES_COUNT)
#define ERROR_GROUP_CAN_COMMUNICATION_INSTANCE_COUNT (1U)
#define ERROR_GROUP_FLASH_INSTANCE_COUNT (1U)
#define ERROR_GROUP_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT (5U)
#define ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT (1U)

/** @brief Type redefinition for an error instance */
typedef errorlib_error_instance_t error_instance_t;

/**
 * @brief Return code for the error module functions
 *
 * @details
 *     - ERROR_OK the function executed succesfully
 *     - ERROR_NULL_POINTER a NULL pointer was given to a function
 *     - ERROR_UNKNOWN unknown error
 */
typedef enum {
    ERROR_OK,
    ERROR_NULL_POINTER,
    ERROR_UNKNOWN
} ErrorReturnCode;

/**
 * @brief Type of the error that categorize a group of instances
 *
 * @details
 *     - ERROR_GROUP_POST the Power On Self Test procedure failed
 *     - ERROR_GROUP_UNDER_VOLTAGE a voltage value is lower than a certain threshold
 *     - ERROR_GROUP_OVER_VOLTAGE a voltage value is higher than a certain threshold
 *     - ERROR_GROUP_UNDER_TEMPERATURE_CELLS a temperature value is lower than a certain threshold
 *     - ERROR_GROUP_OVER_TEMPERATURE_CELLS a temperature value is higher than a certain threshold
 *     - ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE a temperature value is lower than a certain threshold
 *     - ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE a temperature value is higher than a certain threshold
 *     - ERROR_GROUP_CAN CAN bus communication is not working
 *     - ERROR_GROUP_FLASH The flash procedure could not be completed safely
 *     - ERROR_GROUP_BMS_MONITOR BMS monitor communication is not working
 *     - ERROR_GROUP_OPEN_WIRE The BMS monitor detected an open-wire
 */
typedef enum {
    ERROR_GROUP_POST,
    ERROR_GROUP_UNDER_VOLTAGE,
    ERROR_GROUP_OVER_VOLTAGE,
    ERROR_GROUP_UNDER_TEMPERATURE_CELLS,
    ERROR_GROUP_OVER_TEMPERATURE_CELLS,
    ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE,
    ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE,
    ERROR_GROUP_CAN_COMMUNICATION,
    ERROR_GROUP_FLASH,
    ERROR_GROUP_BMS_MONITOR_COMMUNICATION,
    ERROR_GROUP_OPEN_WIRE,
    ERROR_GROUP_COUNT
} ErrorGroup;

typedef enum {
    ERROR_CAN_INSTANCE_BMS
} ErrorCanInstance;

typedef enum {
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION,
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE,
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE,
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE,
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL,
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT
} ErrorBmsMonitorCommunicationInstance;

#ifdef CONF_ERROR_MODULE_ENABLE

/**
 * @brief Initialization of the internal error handler structure
 */
ErrorReturnCode error_init(void);
ErrorReturnCode error_set(const ErrorGroup group, const error_instance_t instance);
ErrorReturnCode error_reset(const ErrorGroup group, const error_instance_t instance);
size_t error_get_expired(void);
ErrorInfo error_get_expired_info(void);

/**
 * @brief Get a pointer to the CAN payload of cellboard error
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_errors_converted_t*  A pointer to the payload
 */
bms_cellboard_errors_converted_t * error_get_errors_canlib_payload(size_t * const byte_size);

#else  // CONF_ERROR_MODULE_ENABLE

#define error_init() (ERROR_OK)
#define error_set(group, instance) (ERROR_OK)
#define error_reset(group, instance) (ERROR_OK)
#define error_get_expired() (0U)
#define error_get_errors_canlib_payload(byte_size) (NULL)
#define error_get_expired_info() ((ErrorInfo){ 0U })

#endif // CONF_ERROR_MODULE_ENABLE

#endif  // ERROR_H

