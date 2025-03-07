/**
 * @brief tasks.c
 * @date 2024-05-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Tasks that have to be executed at a certain interval
 */

#include "tasks.h"

#include <string.h>

#include "bms_network.h"
#include "can-comm.h"
#include "fsm.h"
#include "identity.h"
#include "timebase.h"
#include "volt.h"
#include "temp.h"
#include "bms-monitor-fsm.h"
#include "bms-manager.h"
#include "bal.h"
#include "error.h"

#ifdef CONF_TASKS_MODULE_ENABLE

_STATIC _TasksHandler htasks;

/** @brief Send the current FSM status via CAN */
void _tasks_send_status(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)fsm_get_status_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_STATUS_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the version info via CAN */
void _tasks_send_version(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)identity_get_version_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_VERSION_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the errors status via CAN if an error occoured*/
void _tasks_send_errors(void) {

    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)error_get_error_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_ERROR_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the cells voltages via CAN */
void _tasks_send_voltages(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)volt_get_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_CELLS_VOLTAGE_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the cells temperatures via CAN */
void _tasks_send_temperatures(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)temp_get_cells_temp_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_CELLS_TEMPERATURE_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the discharge resistors temperature via CAN */
void _tasks_send_discharge_temperatures(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)temp_get_discharge_temp_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_DISCHARGE_TEMPERATURE_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Send the current balancing status info via CAN */
void _tasks_send_balancing_status(void) {
    size_t byte_size = 0U;
    const uint8_t * const payload = (const uint8_t * const)bal_get_status_canlib_payload(&byte_size);
    can_comm_tx_add(
        BMS_CELLBOARD_BALANCING_STATUS_INDEX,
        CAN_FRAME_TYPE_DATA,
        payload,
        byte_size
    );
}

/** @brief Start the temperatures conversion */
void _tasks_read_temperatures(void) {
    temp_start_conversion();
}
   
/** @brief Run the bms manager procedures */
void _tasks_run_bms_manager(void) {
    bms_manager_routine();
}

TasksReturnCode tasks_init(milliseconds_t resolution) {
    if (resolution == 0U)
        resolution = 1U;
    memset(&htasks, 0U, sizeof(htasks));

    // Initialize the tasks with the X macro
#define TASKS_X(NAME, ENABLED, START, INTERVAL, EXEC) \
    do { \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].id = TASKS_NAME_TO_ID(NAME); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].start = (START); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].interval = TIMEBASE_MS_TO_TICKS(INTERVAL, resolution); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].exec = (EXEC); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].enabled = (ENABLED); \
    } while(0U);

    TASKS_X_LIST
#undef TASKS_X

    return TASKS_OK;
}

Task * tasks_get_task(const TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return NULL;
    return &htasks.tasks[id];
}

ticks_t tasks_get_start(const TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].start;
}

ticks_t tasks_get_interval(const TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].interval;
}

tasks_callback tasks_get_callback(const TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].exec;
}

TasksReturnCode tasks_set_enable(const TasksId id, const bool enabled) {
    if (id >= TASKS_ID_COUNT)
        return TASKS_INVALID_ID;
    htasks.tasks[id].enabled = enabled;
    return TASKS_OK; 
}

bool tasks_is_enabled(const TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return false;
    return htasks.tasks[id].enabled;
}

#ifdef CONF_TASKS_STRINGS_ENABLE

_STATIC char * tasks_module_name = "tasks";

_STATIC char * tasks_return_code_name[] = {
    [TASKS_OK] = "ok"
};

_STATIC char * tasks_return_code_descritpion[] = {
    [TASKS_OK] = "executed successfully"
};

#define TASKS_X(NAME, START, INTERVAL, EXEC) [TASKS_NAME_TO_ID(NAME)] = #NAME,
_STATIC char * tasks_id_name[] = {
    TASKS_X_LIST
};
#undef TASKS_X

#endif // CONF_TASKS_STRINGS_ENALBE

#endif // CONF_TASKS_MODULE_ENABLE
