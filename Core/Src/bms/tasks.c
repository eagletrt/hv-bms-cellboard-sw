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

#ifdef CONF_TASKS_MODULE_ENABLE

static ticks tasks_interval[TASKS_COUNT] = { 
    [TASKS_ID_SEND_STATUS] = BMS_CELLBOARD_STATUS_CYCLE_TIME_MS,
    [TASKS_ID_SEND_VERSION] = BMS_CELLBOARD_VERSION_CYCLE_TIME_MS,
    [TASKS_ID_SEND_VOLTAGES] = BMS_CELLBOARD_VOLTAGES_CYCLE_TIME_MS,
    [TASKS_ID_SEND_TEMPERATURES] = BMS_CELLBOARD_TEMPERATURES_CYCLE_TIME_MS, 
    [TASKS_ID_CHECK_WATCHDOG] = 200U
};

TasksReturnCode tasks_init(time resolution) {
    if (resolution == 0U)
        resolution = 1U;
    tasks_interval[TASKS_ID_SEND_STATUS] = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_STATUS_CYCLE_TIME_MS, resolution);
    tasks_interval[TASKS_ID_SEND_VERSION] = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_VERSION_CYCLE_TIME_MS, resolution);
    tasks_interval[TASKS_ID_SEND_VOLTAGES] = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_VOLTAGES_CYCLE_TIME_MS, resolution);
    tasks_interval[TASKS_ID_SEND_TEMPERATURES] = TIMEBASE_TIME_TO_TICKS(BMS_CELLBOARD_TEMPERATURES_CYCLE_TIME_MS, resolution);

    return TASKS_OK;
}

ticks tasks_get_interval_from_id(TasksId id) {
    if (id < 0U && id >= TASKS_ID_COUNT)
        return 0U;
    return tasks_interval[id];
}
tasks_callback tasks_get_callback_from_id(TasksId id) {
    switch(id) {
        case TASKS_ID_SEND_STATUS:
            return tasks_send_status;
        case TASKS_ID_SEND_VERSION:
            return tasks_send_version;
        case TASKS_ID_SEND_VOLTAGES:
            return tasks_send_voltages;
        case TASKS_ID_SEND_TEMPERATURES:
            return tasks_send_temperatures;
        case TASKS_ID_CHECK_WATCHDOG:
            return tasks_check_watchdog;

        default:
            return NULL;
    }
}

void tasks_send_status(void) {
    size_t byte_size;
    void * payload = fsm_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

void tasks_send_version(void) {
    size_t byte_size;
    void * payload = identity_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VERSION_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

void tasks_send_voltages(void) {
    size_t byte_size;
    void * payload = volt_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VOLTAGES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

void tasks_send_temperatures(void) {
    size_t byte_size;
    void * payload = temp_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_TEMPERATURES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

void tasks_check_watchdog(void) {
    watchdog_routine(timebase_get_time());
}

#ifdef CONF_TASKS_STRINGS_ENALBE

static char * tasks_module_name = "tasks";

static char * tasks_return_code_name[] = {
    [TASKS_OK] = "ok"
};

static char * tasks_return_code_descritpion[] = {
    [TASKS_OK] = "executed successfully"
};

#endif // CONF_TASKS_STRINGS_ENALBE

#endif // CONF_TASKS_MODULE_ENABLE
