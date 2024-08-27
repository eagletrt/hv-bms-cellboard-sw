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

/** @brief Convert a task name to the corresponding TasksId name */
#define TASKS_NAME_TO_ID(NAME) (TASKS_ID_##NAME)

_STATIC _TasksHandler htasks;

/** @brief Send the current FSM status via CAN */
void _tasks_send_status(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)fsm_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the version info via CAN */
void _tasks_send_version(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)identity_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VERSION_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the errors status via CAN */
void _tasks_send_errors(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)error_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_ERRORS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the cells voltages via CAN */
void _tasks_send_voltages(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)volt_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_CELLS_VOLTAGE_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the cells temperatures via CAN */
void _tasks_send_temperatures(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)temp_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_CELLS_TEMPERATURE_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the current balancing status info via CAN */
void _tasks_send_balancing_status(void) {
    size_t byte_size = 0U;
    uint8_t * payload = (uint8_t *)bal_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_BALANCING_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

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
#define TASKS_X(NAME, START, INTERVAL, EXEC) \
    do { \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].id = TASKS_NAME_TO_ID(NAME); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].start = (START); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].interval = TIMEBASE_TIME_TO_TICKS(INTERVAL, resolution); \
        htasks.tasks[TASKS_NAME_TO_ID(NAME)].exec = (EXEC); \
    } while(0U);

    TASKS_X_LIST
#undef TASKS_X

    return TASKS_OK;
}

Task * tasks_get_task(TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return NULL;
    return &htasks.tasks[id];
}

ticks_t tasks_get_start(TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].start;
}

ticks_t tasks_get_interval(TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].interval;
}

tasks_callback tasks_get_callback(TasksId id) {
    if (id >= TASKS_ID_COUNT)
        return 0U;
    return htasks.tasks[id].exec;
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
