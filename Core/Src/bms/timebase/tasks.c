/**
 * \brief tasks.c
 * \date 2024-05-16
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * \brief Tasks that have to be executed at a certain interval
 */

#include "tasks.h"

#include <stdint.h>
#include <string.h>

#include "can-bms.h"
#include "can-bms-api.h"

#include "can-communication.h"
#include "can-communication-api.h"
#include "bal-api.h"
#include "volt-api.h"
#include "temp-api.h"
#include "cellboard-def.h"
#include "identity-api.h"
#include "timebase.h"

#ifdef CONF_TASKS_MODULE_ENABLE

EAGLETRT_STATIC struct TaskHandler htasks;

void prv_tasks_send_balancing_status(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1BALANCING,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2BALANCING,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3BALANCING,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4BALANCING,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5BALANCING,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6BALANCING
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = bal_api_get_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

/** \brief Send the current FSM status via CAN */
void prv_tasks_send_status(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1FSM,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2FSM,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3FSM,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4FSM,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5FSM,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6FSM
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = fsm_get_status_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

/** \brief Send the cells voltages via CAN */
void prv_tasks_send_voltage(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1VOLTAGE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2VOLTAGE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3VOLTAGE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4VOLTAGE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5VOLTAGE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6VOLTAGE
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = volt_api_get_voltage_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

/*! \brief Send the cells temperatures via CAN */
void prv_tasks_send_temperature(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1TEMPERATURE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2TEMPERATURE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3TEMPERATURE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4TEMPERATURE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5TEMPERATURE,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6TEMPERATURE
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = temp_api_get_temperature_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

/** \brief Send the temperature info via CAN */
void prv_tasks_send_temperature_info(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1TEMPERATUREINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2TEMPERATUREINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3TEMPERATUREINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4TEMPERATUREINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5TEMPERATUREINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6TEMPERATUREINFO
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = temp_api_get_temperature_info_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

/** \brief Send the voltage info via CAN */
void prv_tasks_send_voltage_info(void) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_id[] = {
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD1VOLTAGEINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD2VOLTAGEINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD3VOLTAGEINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD4VOLTAGEINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD5VOLTAGEINFO,
        CAN_BMS_MESSAGE_FRAME_ID_TSACCELLBOARD6VOLTAGEINFO
    };

    struct CanCommunicationFrame frame = { 0 };
    frame.id = can_id[cellboard];

    union CanBmsMessages *message = volt_api_get_voltage_info_canlib_payload(NULL);
    int byte_size = can_bms_api_serialize_from_id(
        frame.id,
        message,
        frame.data);

    // TODO: Notify error?
    if (byte_size >= 0) {
        frame.length = byte_size;
        EAGLETRT_API_UNUSED(can_communication_api_add_to_tx(CAN_COMMUNICATION_NETWORK_BMS, &frame));
    }
}

// TODO: update libcan
// /** \brief Send the version info via CAN */
// void _tasks_send_version(void) {
//     size_t byte_size = 0U;
//     const uint8_t *const payload = (const uint8_t *const)identity_api_get_version_canlib_payload(&byte_size);
//     can_comm_tx_add(
//         BMS_CELLBOARD_VERSION_INDEX,
//         CAN_FRAME_TYPE_DATA,
//         payload,
//         byte_size);
// }
//
// /** \brief Send the errors status via CAN if an error occoured*/
// void _tasks_send_errors(void) {
//
//     size_t byte_size = 0U;
//     const uint8_t *const payload = (const uint8_t *const)error_api_get_error_canlib_payload(&byte_size);
//     can_comm_tx_add(
//         BMS_CELLBOARD_ERROR_INDEX,
//         CAN_FRAME_TYPE_DATA,
//         payload,
//         byte_size);
// }
//
// /** \brief Send the discharge resistors temperature via CAN */
// void _tasks_send_discharge_temperatures(void) {
//     size_t byte_size = 0U;
//     const uint8_t *const payload = (const uint8_t *const)temp_api_get_discharge_temp_canlib_payload(&byte_size);
//     can_comm_tx_add(
//         BMS_CELLBOARD_DISCHARGE_TEMPERATURE_INDEX,
//         CAN_FRAME_TYPE_DATA,
//         payload,
//         byte_size);
// }

/** \brief Start the temperatures conversion */
void prv_tasks_read_temperatures(void) {
    temp_api_start_conversion();
}

/** \brief Run the bms manager procedures */
void prv_tasks_run_bms_manager(void) {
    bms_manager_api_routine();
}

enum TasksReturnCode tasks_init(milliseconds_t resolution) {
    if (resolution == 0U) {
        resolution = 1U;
    }
    memset(&htasks, 0U, sizeof(htasks));

    // Initialize the tasks with the X macro
#define TASKS_X(NAME, ENABLED, START, INTERVAL, EXEC)                                               \
    do {                                                                                            \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].id = TASKS_NAME_TO_ID(NAME);                           \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].start = (START);                                       \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].interval = TIMEBASE_MS_TO_TICKS(INTERVAL, resolution); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].exec = (EXEC);                                         \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].enabled = (ENABLED);                                   \
    } while (0U);

    TASKS_X_LIST
#undef TASKS_X

    return TASKS_OK;
}

struct Task *tasks_get_task(const enum TasksId task_id) {
    if (task_id >= TASKS_ID_COUNT) {
        return NULL;
    }
    return &htasks.tasks[task_id];
}

ticks_t tasks_get_start(const enum TasksId task_id) {
    if (task_id >= TASKS_ID_COUNT) {
        return 0U;
    }
    return htasks.tasks[task_id].start;
}

ticks_t tasks_get_interval(const enum TasksId task_id) {
    if (task_id >= TASKS_ID_COUNT) {
        return 0U;
    }
    return htasks.tasks[task_id].interval;
}

tasks_callback tasks_get_callback(const enum TasksId task_id) {
    if (task_id >= TASKS_ID_COUNT) {
        return 0U;
    }
    return htasks.tasks[task_id].exec;
}

enum TasksReturnCode tasks_set_enable(const enum TasksId task_id, const bool enabled) {
    if (task_id >= TASKS_ID_COUNT) {
        return TASKS_INVALID_ID;
    }
    htasks.tasks[task_id].enabled = enabled;
    return TASKS_OK;
}

bool tasks_is_enabled(const enum TasksId task_id) {
    if (task_id >= TASKS_ID_COUNT) {
        return false;
    }
    return htasks.tasks[task_id].enabled;
}

#ifdef CONF_TASKS_STRINGS_ENABLE

EAGLETRT_STATIC char *tasks_module_name = "tasks";

EAGLETRT_STATIC char *tasks_return_code_name[] = {
    [TASKS_OK] = "ok"
};

EAGLETRT_STATIC char *tasks_return_code_descritpion[] = {
    [TASKS_OK] = "executed successfully"
};

#define TASKS_X(NAME, START, INTERVAL, EXEC) [TASKS_NAME_TO_ID(NAME)] = #NAME,
EAGLETRT_STATIC char *tasks_id_name[] = {
    TASKS_X_LIST
};
#undef TASKS_X

#endif // CONF_TASKS_STRINGS_ENALBE

#endif // CONF_TASKS_MODULE_ENABLE
