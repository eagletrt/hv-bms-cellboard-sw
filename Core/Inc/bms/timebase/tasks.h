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

/** @brief Total number of tasks */
#define TASKS_COUNT (TASKS_ID_COUNT)

/** @brief Type definition for a function that excecutes a single task */
typedef void (* tasks_callback)(void);

/**
 * @brief Tasks identifiers
 *
 * @details
 *     - TASKS_ID_SEND_STATUS send the cellboard status via the CAN bus
 *     - TASKS_ID_SEND_VERSION send the cellboard version via the CAN bus
 *     - TASKS_ID_SEND_VOLTAGES send the cells voltages via the CAN bus
 *     - TASKS_ID_SEND_TEMPERATURES send the cells temperatures via the CAN bus
 *     - TASKS_ID_CHECK_WATCHDOG check the status of the watchogs
 *     - TASKS_ID_RUN_BMS_MANAGER run the BMS manager operations
 */
typedef enum {
    TASKS_ID_SEND_STATUS,
    TASKS_ID_SEND_VERSION,
    TASKS_ID_SEND_VOLTAGES,
    TASKS_ID_SEND_TEMPERATURES,
    TASKS_ID_CHECK_WATCHDOG,
    TASKS_ID_RUN_BMS_MANAGER,
    TASKS_ID_COUNT
} TasksId;

/**
 * @brief Return code for the tasks module functions
 *
 * @details
 *     - TASKS_OK the function executed succesfully
 */
typedef enum {
    TASKS_OK
} TasksReturnCode;

#ifdef CONF_TASKS_MODULE_ENABLE

/**
 * @brief Initialize the tasks module
 *
 * @param resolution The timebase resolution
 *
 * @return TasksReturnCode
 *     - TASKS_OK
 */
TasksReturnCode tasks_init(time_t resolution);

/**
 * @brief Get the task interval from its identifier
 *
 * @param id The task identifier
 *
 * @return ticks_t The task interval or 0 if not found
 */
ticks_t tasks_get_interval_from_id(TasksId id);

/**
 * @brief Get the task callback from its identifier
 *
 * @param id The task identifier
 *
 * @return tasks_callback A pointer to the task callaback or NULL if not found
 */
tasks_callback tasks_get_callback_from_id(TasksId id);

#else  // CONF_TASKS_MODULE_ENABLE

#define tasks_init(resolution) (TASKS_OK)
#define tasks_get_interval_from_id(id) (0U)
#define tasks_get_callback_from_id(id) (NULL)

#endif // CONF_TASKS_MODULE_ENABLE
