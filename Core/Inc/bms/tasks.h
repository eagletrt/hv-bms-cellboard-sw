/**
 * @brief tasks.h
 * @date 2024-05-15
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Implementations of the tasks that have to be executed by the timebase
 *
 * @attention This file should only be included inside the timebase source
 * The code is separated only for clarity and to avoid having all the code inside a single
 * file
 *
 * @details To use the functions inside this file define the TASKS_IMPLEMENTATION macro
 */

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"
#include "can-comm.h"
#include "fsm.h"
#include "identity.h"

#if defined(CONF_TIMEBASE_MODULE_ENABLE) && defined(TASKS_IMPLEMENTATION)

/** @brief Total number of tasks */
#define TASKS_COUNT (TASKS_ID_COUNT)

/**
 * @brief Tasks identifiers
 *
 * @details
 *     - TASKS_ID_SEND_STATUS send the cellboard status via the CAN bus
 *     - TASKS_ID_SEND_VERSION send the cellboard version via the CAN bus
 *     - TASKS_ID_SEND_VOLTAGES send the cells voltages via the CAN bus
 *     - TASKS_ID_SEND_TEMPERATURES send the cells temperatures via the CAN bus
 *     - TASKS_ID_CHECK_WATCHDOG check the status of the watchogs
 */
typedef enum {
    TASKS_ID_SEND_STATUS,
    TASKS_ID_SEND_VERSION,
    TASKS_ID_SEND_VOLTAGES,
    TASKS_ID_SEND_TEMPERATURES,
    TASKS_ID_CHECK_WATCHDOG,
    TASKS_ID_COUNT
} TasksId;

// TODO: Handle return codes

/** @brief Get and send the status of the FSM */
void tasks_send_status(ticks tick, time ms) {
    CELLBOARD_UNUSED(tick);
    CELLBOARD_UNUSED(ms);

    size_t byte_size;
    void * payload = fsm_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Get and send info about the versions */
void tasks_send_version(ticks tick, time ms) {
    CELLBOARD_UNUSED(tick);
    CELLBOARD_UNUSED(ms);

    size_t byte_size;
    void * payload = identity_get_can_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VERSION_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Get and send the cells voltages */
void tasks_send_voltages(ticks tick, time ms) {
    CELLBOARD_UNUSED(tick);
    CELLBOARD_UNUSED(ms);

    size_t byte_size;
    void * payload = volt_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_VOLTAGES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Get and send the cells temperatures */
void tasks_send_temperatures(ticks tick, time ms) {
    CELLBOARD_UNUSED(tick);
    CELLBOARD_UNUSED(ms);

    size_t byte_size;
    void * payload = temp_get_canlib_payload(&byte_size);
    can_comm_tx_add(BMS_CELLBOARD_TEMPERATURES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
}

/** @brief Check the watchdog status */
void tasks_check_watchdog(ticks tick, time ms) {
    CELLBOARD_UNUSED(tick);

    watchog_routine(ms);
}

#endif // CONF_TIMEBASE_MODULE_ENABLE
