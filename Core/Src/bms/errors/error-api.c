/*!
 * \file error.c
 * \date 2024-08-24
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * \brief Simple wrapper for the error handler generated code
 */

#include "error-api.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can-bms.h"
#include "cellboard-def.h"
#include "eagletrt.h"

#include "error.h"
#include "identity-api.h"

#ifdef CONF_ERROR_MODULE_ENABLE

EAGLETRT_STATIC ErrorLibHandler error_handler;

/*! \brief Canlib payload containing the error */
EAGLETRT_STATIC union CanBmsMessages error_libcan_message;

/*! \brief A callback to resets the mainboard */
EAGLETRT_STATIC system_reset_callback system_reset;

/*! \brief Total number of instances for each group */
EAGLETRT_STATIC const size_t error_instances[] = {
    [ERROR_GROUP_POST] = ERROR_GROUP_POST_INSTANCE_COUNT,
    [ERROR_GROUP_UNDER_VOLTAGE] = ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT,
    [ERROR_GROUP_OVER_VOLTAGE] = ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT,
    [ERROR_GROUP_UNDER_TEMPERATURE_CELLS] = ERROR_GROUP_UNDER_TEMPERATURE_CELLS_INSTANCE_COUNT,
    [ERROR_GROUP_OVER_TEMPERATURE_CELLS] = ERROR_GROUP_OVER_TEMPERATURE_CELLS_INSTANCE_COUNT,
    [ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE] = ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT,
    [ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE] = ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT,
    [ERROR_GROUP_CAN_COMMUNICATION] = ERROR_GROUP_CAN_COMMUNICATION_INSTANCE_COUNT,
    [ERROR_GROUP_FLASH] = ERROR_GROUP_FLASH_INSTANCE_COUNT,
    [ERROR_GROUP_BMS_MONITOR_COMMUNICATION] = ERROR_GROUP_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT,
    [ERROR_GROUP_OPEN_WIRE] = ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT
};
/*!
 * \brief Error thresholds for each group
 *
 * \details The values are arbitrary and should not be too much high
 */
EAGLETRT_STATIC const size_t error_thresholds[] = {
    [ERROR_GROUP_POST] = 1U,
    [ERROR_GROUP_UNDER_VOLTAGE] = 3U,
    [ERROR_GROUP_OVER_VOLTAGE] = 3U,
    [ERROR_GROUP_UNDER_TEMPERATURE_CELLS] = 5U,
    [ERROR_GROUP_OVER_TEMPERATURE_CELLS] = 5U,
    [ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE] = 5U,
    [ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE] = 5U,
    [ERROR_GROUP_CAN_COMMUNICATION] = 5U,
    [ERROR_GROUP_FLASH] = 3U,
    [ERROR_GROUP_BMS_MONITOR_COMMUNICATION] = 5U,
    [ERROR_GROUP_OPEN_WIRE] = 3U
};

/*! \brief List of errors where the data is stored */
EAGLETRT_STATIC int32_t error_post_instances[ERROR_GROUP_POST_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_under_voltage_instances[ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_over_voltage_instances[ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_under_temperature_cells_instances[ERROR_GROUP_UNDER_TEMPERATURE_CELLS_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_over_temperature_cells_instances[ERROR_GROUP_OVER_TEMPERATURE_CELLS_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_under_temperature_discharge_instances[ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_over_temperature_discharge_instances[ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_can_communication_instances[ERROR_GROUP_CAN_COMMUNICATION_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_flash_instances[ERROR_GROUP_FLASH_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_bms_monitor_communication_instances[ERROR_GROUP_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t error_open_wire_instances[ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT];
EAGLETRT_STATIC int32_t *error[] = {
    [ERROR_GROUP_POST] = error_post_instances,
    [ERROR_GROUP_UNDER_VOLTAGE] = error_under_voltage_instances,
    [ERROR_GROUP_OVER_VOLTAGE] = error_over_voltage_instances,
    [ERROR_GROUP_UNDER_TEMPERATURE_CELLS] = error_under_temperature_cells_instances,
    [ERROR_GROUP_OVER_TEMPERATURE_CELLS] = error_over_temperature_cells_instances,
    [ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE] = error_under_temperature_discharge_instances,
    [ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE] = error_over_temperature_discharge_instances,
    [ERROR_GROUP_CAN_COMMUNICATION] = error_can_communication_instances,
    [ERROR_GROUP_FLASH] = error_flash_instances,
    [ERROR_GROUP_BMS_MONITOR_COMMUNICATION] = error_bms_monitor_communication_instances,
    [ERROR_GROUP_OPEN_WIRE] = error_open_wire_instances
};

enum ErrorReturnCode error_api_init(const system_reset_callback reset) {
    system_reset = NULL;
    memset(&error_handler, 0U, sizeof(error_handler));
    // memset(&error_can_payload, 0U, sizeof(error_can_payload));
    if (errorlib_init(&error_handler,
                      error,
                      error_instances,
                      error_thresholds,
                      ERROR_GROUP_COUNT) != ERRORLIB_OK) {
        return ERROR_RC_UNKNOWN;
    }

    if (reset == NULL) {
        return ERROR_RC_NULL_POINTER;
    }
    system_reset = reset;
    return ERROR_RC_OK;
}

enum ErrorReturnCode error_api_set(const enum ErrorGroup group, const error_instance_t instance) {
    ErrorLibReturnCode ret_code = errorlib_error_set(&error_handler, (errorlib_error_group_t)group, instance);

    if (errorlib_get_expired(&error_handler) > 0U) {
        ErrorInfo error = errorlib_get_expired_info(&error_handler);

        if (error.group == ERROR_GROUP_CAN_COMMUNICATION) {
            // Check if the error is from can and in that case reset the cellboard
            system_reset();
        } else {
            // Otherwise init the error payload and start sending it to the mainboard
            // error_can_payload.cellboard_id = (bms_cellboard_error_cellboard_id)identity_api_get_cellboard_id();
            // error_can_payload.group = error.group;
            // error_can_payload.instance = error.instance;

            // tasks_set_enable(TASKS_ID_SEND_ERROR, true);
        }
    }
    return ret_code != ERRORLIB_OK ? ERROR_RC_UNKNOWN : ERROR_RC_OK;
}

enum ErrorReturnCode error_api_reset(const enum ErrorGroup group, const error_instance_t instance) {
    if (errorlib_error_reset(&error_handler, (errorlib_error_group_t)group, instance) != ERRORLIB_OK) {
        return ERROR_RC_UNKNOWN;
    }
    return ERROR_RC_OK;
}

size_t error_api_get_expired(void) {
    return errorlib_get_expired(&error_handler);
}

ErrorInfo error_api_get_expired_info(void) {
    return errorlib_get_expired_info(&error_handler);
}

union CanBmsMessages *error_api_get_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1error,
        can_bms_byte_size_tsaccellboard2error,
        can_bms_byte_size_tsaccellboard3error,
        can_bms_byte_size_tsaccellboard4error,
        can_bms_byte_size_tsaccellboard5error,
        can_bms_byte_size_tsaccellboard6error
    };

    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    uint8_t undervoltage = 0;
    for (size_t i = 0; i < ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT; ++i) {
        undervoltage |= error_under_voltage_instances[i];
    }
    uint8_t overvoltage = 0;
    for (size_t i = 0; i < ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT; ++i) {
        overvoltage |= error_over_voltage_instances[i];
    }
    uint8_t undertemperature = 0;
    for (size_t i = 0; i < ERROR_GROUP_UNDER_TEMPERATURE_CELLS_INSTANCE_COUNT; ++i) {
        undertemperature |= error_under_temperature_cells_instances[i];
    }
    uint8_t overtemperature = 0;
    for (size_t i = 0; i < ERROR_GROUP_OVER_TEMPERATURE_CELLS_INSTANCE_COUNT; ++i) {
        overtemperature |= error_over_temperature_cells_instances[i];
    }
    uint8_t discharge_undertemperature = 0;
    for (size_t i = 0; i < ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT; ++i) {
        discharge_undertemperature |= error_under_temperature_discharge_instances[i];
    }
    uint8_t discharge_overtemperature = 0;
    for (size_t i = 0; i < ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT; ++i) {
        discharge_overtemperature |= error_over_temperature_discharge_instances[i];
    }
    uint8_t bms_monitor_communication = 0;
    for (size_t i = 0; i < ERROR_GROUP_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT; ++i) {
        bms_monitor_communication |= error_bms_monitor_communication_instances[i];
    }

    switch (cellboard) {
        case CELLBOARD_ID_0:
            error_libcan_message.tsaccellboard1error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard1error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard1error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard1error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard1error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard1error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard1error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard1error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard1error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard1error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard1error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard1error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard1error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard1error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard1error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard1error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard1error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard1error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard1error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard1error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard1error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard1error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard1error.openwire13 = error_open_wire_instances[12];
            break;
        case CELLBOARD_ID_1:
            error_libcan_message.tsaccellboard2error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard2error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard2error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard2error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard2error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard2error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard2error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard2error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard2error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard2error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard2error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard2error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard2error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard2error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard2error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard2error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard2error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard2error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard2error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard2error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard2error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard2error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard2error.openwire13 = error_open_wire_instances[12];
            break;
        case CELLBOARD_ID_2:
            error_libcan_message.tsaccellboard3error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard3error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard3error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard3error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard3error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard3error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard3error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard3error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard3error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard3error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard3error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard3error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard3error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard3error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard3error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard3error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard3error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard3error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard3error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard3error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard3error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard3error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard3error.openwire13 = error_open_wire_instances[12];
            break;
        case CELLBOARD_ID_3:
            error_libcan_message.tsaccellboard4error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard4error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard4error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard4error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard4error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard4error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard4error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard4error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard4error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard4error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard4error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard4error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard4error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard4error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard4error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard4error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard4error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard4error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard4error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard4error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard4error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard4error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard4error.openwire13 = error_open_wire_instances[12];
            break;
        case CELLBOARD_ID_4:
            error_libcan_message.tsaccellboard5error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard5error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard5error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard5error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard5error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard5error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard5error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard5error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard5error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard5error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard5error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard5error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard5error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard5error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard5error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard5error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard5error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard5error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard5error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard5error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard5error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard5error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard5error.openwire13 = error_open_wire_instances[12];
            break;
        case CELLBOARD_ID_5:
            error_libcan_message.tsaccellboard6error.post = error_post_instances[0];
            error_libcan_message.tsaccellboard6error.cellundervoltage = undervoltage;
            error_libcan_message.tsaccellboard6error.cellovervoltage = overvoltage;
            error_libcan_message.tsaccellboard6error.cellundertemperature = undertemperature;
            error_libcan_message.tsaccellboard6error.cellovertemperature = overtemperature;
            error_libcan_message.tsaccellboard6error.dischargeresistorundertemperature = discharge_undertemperature;
            error_libcan_message.tsaccellboard6error.dischargeresistorovertemperature = discharge_overtemperature;
            error_libcan_message.tsaccellboard6error.cancommunication = error_can_communication_instances[0];
            error_libcan_message.tsaccellboard6error.flash = error_flash_instances[0];
            error_libcan_message.tsaccellboard6error.bmsmonitorcommunication = bms_monitor_communication;
            error_libcan_message.tsaccellboard6error.openwire1 = error_open_wire_instances[0];
            error_libcan_message.tsaccellboard6error.openwire2 = error_open_wire_instances[1];
            error_libcan_message.tsaccellboard6error.openwire3 = error_open_wire_instances[2];
            error_libcan_message.tsaccellboard6error.openwire4 = error_open_wire_instances[3];
            error_libcan_message.tsaccellboard6error.openwire5 = error_open_wire_instances[4];
            error_libcan_message.tsaccellboard6error.openwire6 = error_open_wire_instances[5];
            error_libcan_message.tsaccellboard6error.openwire7 = error_open_wire_instances[6];
            error_libcan_message.tsaccellboard6error.openwire8 = error_open_wire_instances[7];
            error_libcan_message.tsaccellboard6error.openwire9 = error_open_wire_instances[8];
            error_libcan_message.tsaccellboard6error.openwire10 = error_open_wire_instances[9];
            error_libcan_message.tsaccellboard6error.openwire11 = error_open_wire_instances[10];
            error_libcan_message.tsaccellboard6error.openwire12 = error_open_wire_instances[11];
            error_libcan_message.tsaccellboard6error.openwire13 = error_open_wire_instances[12];
            break;
        default:
            break;
    }
    return &error_libcan_message;
}

#ifdef CONF_ERROR_STRINGS_ENABLE

EAGLETRT_STATIC char *error_module_name = "error";

EAGLETRT_STATIC char *error_return_code_name[] = {
    [ERROR_RC_OK] = "ok",
    [ERROR_RC_NULL_POINTER] = "null pointer",
    [ERROR_RC_UNKNOWN] = "unknown"
};

EAGLETRT_STATIC char *error_return_code_description[] = {
    [ERROR_RC_OK] = "executed succesfully",
    [ERROR_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [ERROR_RC_UNKNOWN] = "unknown error"
};

#endif // CONF_ERROR_STRINGS_ENABLE

#endif // CONF_ERROR_MODULE_ENABLE
