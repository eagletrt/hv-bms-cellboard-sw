/**
 * @file error.c
 * @date 2024-08-24
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Simple wrapper for the error handler generated code
 */

#include "error.h"

#include <string.h>

#include "bms_network.h"

#ifdef CONF_ERROR_MODULE_ENABLE

_STATIC ErrorLibHandler herror;

bms_cellboard_errors_converted_t can_payload;

/** @brief Total number of instances for each group */
const size_t instances[] = {
    [ERROR_GROUP_POST] = ERROR_GROUP_POST_INSTANCE_COUNT,
    [ERROR_GROUP_UNDER_VOLTAGE] = ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT,
    [ERROR_GROUP_OVER_VOLTAGE] = ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT,
    [ERROR_GROUP_UNDER_TEMPERATURE] = ERROR_GROUP_UNDER_TEMPERATURE_INSTANCE_COUNT,
    [ERROR_GROUP_OVER_TEMPERATURE] = ERROR_GROUP_OVER_TEMPERATURE_INSTANCE_COUNT,
    [ERROR_GROUP_CAN] = ERROR_GROUP_CAN_INSTANCE_COUNT,
    [ERROR_GROUP_FLASH] = ERROR_GROUP_FLASH_INSTANCE_COUNT,
    [ERROR_GROUP_BMS_MONITOR] = ERROR_GROUP_BMS_MONITOR_INSTANCE_COUNT,
    [ERROR_GROUP_OPEN_WIRE] = ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT
};
/**
 * @brief Error thresholds for each group
 *
 * @details The values are arbitrary and should not be too much high
 */
const size_t thresholds[] = {
    [ERROR_GROUP_POST] = 1U,
    [ERROR_GROUP_UNDER_VOLTAGE] = 5U,
    [ERROR_GROUP_OVER_VOLTAGE] = 5U,
    [ERROR_GROUP_UNDER_TEMPERATURE] = 10U,
    [ERROR_GROUP_OVER_TEMPERATURE] = 10U,
    [ERROR_GROUP_CAN] = 5U,
    [ERROR_GROUP_FLASH] = 3U,
    [ERROR_GROUP_BMS_MONITOR] = 5U,
    [ERROR_GROUP_OPEN_WIRE] = 3U
};

/** @brief List of errors where the data is stored */
int32_t error_post_instances[ERROR_GROUP_POST_INSTANCE_COUNT];
int32_t error_under_voltage_instances[ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT];
int32_t error_over_voltage_instances[ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT];
int32_t error_under_temperature_instances[ERROR_GROUP_UNDER_TEMPERATURE_INSTANCE_COUNT];
int32_t error_over_temperature_instances[ERROR_GROUP_OVER_TEMPERATURE_INSTANCE_COUNT];
int32_t error_can_instances[ERROR_GROUP_CAN_INSTANCE_COUNT];
int32_t error_flash_instances[ERROR_GROUP_FLASH_INSTANCE_COUNT];
int32_t error_bms_monitor_instances[ERROR_GROUP_BMS_MONITOR_INSTANCE_COUNT];
int32_t error_open_wire_instances[ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT];
int32_t * errors[] = {
    [ERROR_GROUP_POST] = error_post_instances,
    [ERROR_GROUP_UNDER_VOLTAGE] = error_under_voltage_instances,
    [ERROR_GROUP_OVER_VOLTAGE] = error_over_voltage_instances,
    [ERROR_GROUP_UNDER_TEMPERATURE] = error_under_temperature_instances,
    [ERROR_GROUP_OVER_TEMPERATURE] = error_over_temperature_instances,
    [ERROR_GROUP_CAN] = error_can_instances,
    [ERROR_GROUP_FLASH] = error_flash_instances,
    [ERROR_GROUP_BMS_MONITOR] = error_bms_monitor_instances,
    [ERROR_GROUP_OPEN_WIRE] = error_open_wire_instances
};

ErrorReturnCode error_init(void) {
    if (errorlib_init(&herror,
        errors,
        instances,
        thresholds,
        ERROR_GROUP_COUNT
    ) != ERRORLIB_OK)
        return ERROR_UNKNOWN;
    memset(&can_payload, 0U, sizeof(can_payload));
    return ERROR_OK;
}

ErrorReturnCode error_set(ErrorGroup group, error_instance_t instance) {
    if (errorlib_error_set(&herror, (errorlib_error_group_t)group, instance) != ERRORLIB_OK)
        return ERROR_UNKNOWN;
    return ERROR_OK;
}

ErrorReturnCode error_reset(ErrorGroup group, error_instance_t instance) {
    if (errorlib_error_reset(&herror, (errorlib_error_group_t)group, instance) != ERRORLIB_OK)
        return ERROR_UNKNOWN;
    return ERROR_OK;
}

size_t error_get_expired(void) {
    return errorlib_get_expired(&herror);
}

bms_cellboard_errors_converted_t * error_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(can_payload);
    // TODO: Set canlib payload data
    // can_payload.can = errorlib_error_get_status(&herror, ERROR_GROUP_CAN);
    return &can_payload;
}

#ifdef CONF_ERROR_STRINGS_ENABLE

_STATIC char * error_module_name = "error";

_STATIC char * error_return_code_name[] = {
    [ERROR_OK] = "ok",
    [ERROR_NULL_POINTER] = "null pointer"
    [ERROR_UNKNOWN] = "unknown"
}

_STATIC char * error_return_code_description[] = {
    [ERROR_OK] = "executed succesfully",
    [ERROR_NULL_POINTER] = "attempt to dereference a null pointer",
    [ERROR_UNKNOWN] = "unknown error"
}

#endif // CONF_ERROR_STRINGS_ENABLE

#endif // CONF_ERROR_MODULE_ENABLE
