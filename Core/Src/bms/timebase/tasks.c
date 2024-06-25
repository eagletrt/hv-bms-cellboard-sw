/**
 * @brief tasks.c
 * @date 2024-05-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Tasks that have to be executed at a certain interval
 */

#include "tasks.h"

#include "bms_network.h"
#include "can-comm.h"
#include "fsm.h"
#include "identity.h"
#include "timebase.h"
#include "volt.h"
#include "temp.h"
#include "watchdog.h"
#include "bms-manager.h"
#include "bal.h"

#ifdef CONF_TASKS_MODULE_ENABLE

/** @brief Convert a task name to the corresponding TasksId name */
#define TASKS_NAME_TO_ID(NAME) (TASKS_ID_##NAME)

/**
 * @brief Tasks hanlder struct
 *
 * @param tasks The array of tasks
 */
static struct {
    Task tasks[TASKS_COUNT];
} htasks;

/** @brief Send the current FSM status via CAN */
void _tasks_send_status(void) {
    size_t byte_size;
    uint8_t * payload = (uint8_t *)fsm_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the version info via CAN */
void _tasks_send_version(void) {
    size_t byte_size;
    uint8_t * payload = (uint8_t *)identity_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VERSION_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the cells voltages via CAN */
void _tasks_send_voltages(void) {
    size_t byte_size;
    uint8_t * payload = (uint8_t *)volt_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_CELLS_VOLTAGE_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the cells temperatures via CAN */
void _tasks_send_temperatures(void) {
    size_t byte_size;
    uint8_t * payload = (uint8_t *)temp_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_CELLS_TEMPERATURE_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Send the current balancing status info via CAN */
void _tasks_send_balancing_status(void) {
    size_t byte_size;
    uint8_t * payload = (uint8_t *)bal_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_BALANCING_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}
   
/** @brief Run the watchdog routine */
void _tasks_check_watchdog(void) {
    // Canlib watchdog
    watchdog_routine(timebase_get_time());
}

/** @brief Run the bms manager procedures */
void _tasks_run_bms_manager(void) {
    bms_manager_run();
}

TasksReturnCode tasks_init(milliseconds_t resolution) {
    if (resolution == 0U)
        resolution = 1U;

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

static char * tasks_module_name = "tasks";

static char * tasks_return_code_name[] = {
    [TASKS_OK] = "ok"
};

static char * tasks_return_code_descritpion[] = {
    [TASKS_OK] = "executed successfully"
};

#define TASKS_X(NAME, START, INTERVAL, EXEC) [TASKS_NAME_TO_ID(NAME)] = #NAME,
static char * tasks_id_name[] = {
    TASKS_X_LIST
};
#undef TASKS_X

#endif // CONF_TASKS_STRINGS_ENALBE

#endif // CONF_TASKS_MODULE_ENABLE
