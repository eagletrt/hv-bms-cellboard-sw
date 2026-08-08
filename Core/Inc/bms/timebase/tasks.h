/*!
 * \brief tasks.h
 * \date 2024-05-15
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * \brief Implementations of the tasks that have to be executed by the timebase
 *
 * \attention This file should only be included inside the timebase source
 * The code is separated only for clarity and to avoid having all the code
 * inside a single file
 *
 * \details To use the functions inside this file define the
 * TASKS_IMPLEMENTATION macro
 */
#ifndef TASKS_H
#define TASKS_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms-monitor-fsm.h"

/*!\brief Total number of tasks */
#define TASKS_COUNT (TASKS_ID_COUNT)

/*!
 * \brief List of tasks parameters
 *
 * \attention !!! DO NOT USE THIS MACRO OUTSIDE OF THIS FILE !!!
 *
 * \attention This file uses X macros (https://en.wikipedia.org/wiki/X_macro)
 * to make it easier to add more tasks without to much changes to the code
 *
 * \details To add a new task add a field to this list and give it the right
 * parameters then go to the source file and implement the callback function
 *
 * \param name The name associated with the task (have to be unique)
 * \param start The first moment when the task is executed (in ticks)
 * \param interval How often the task should run
 * \param exec A pointer to the task function callback
 * \param enabled A boolean indicating if the task is enabled or not
 */
#define TASKS_X_LIST                                                                                                             \
    TASKS_X(SEND_BALANCING_STATUS, true, 10U, can_bms_cycle_time_tsaccellboard1balancing, prv_tasks_send_balancing_status)       \
    TASKS_X(SEND_STATUS, true, 10U, can_bms_cycle_time_tsaccellboard1fsm, prv_tasks_send_status)                                 \
    TASKS_X(SEND_VOLTAGE, true, 10U, can_bms_cycle_time_tsaccellboard1voltage, prv_tasks_send_voltage)                           \
    TASKS_X(SEND_TEMPERATURE, true, 10U, can_bms_cycle_time_tsaccellboard1temperature, prv_tasks_send_temperature)               \
    TASKS_X(SEND_TEMPERATURE_INFO, true, 10U, can_bms_cycle_time_tsaccellboard1temperatureinfo, prv_tasks_send_temperature_info) \
    TASKS_X(SEND_VOLTAGE_INFO, true, 10U, can_bms_cycle_time_tsaccellboard1voltageinfo, prv_tasks_send_voltage_info)             \
    TASKS_X(READ_TEMPERATURES, true, 0U, 10U, prv_tasks_read_temperatures)                                                       \
    TASKS_X(RUN_BMS_MANAGER, true, 0U, 2U, prv_tasks_run_bms_manager)

/*! \brief Convert a task name to the corresponding TasksId name */
#define TASKS_NAME_TO_ID(NAME) (TASKS_ID_##NAME)

/*! \brief Type definition for a function that excecutes a single task */
typedef void (*tasks_callback)(void);

/*!
 * \brief Return code for the tasks module functions
 *
 * \details
 *     - TASKS_RC_OK the function executed succesfully
 */
enum TasksReturnCode {
    TASKS_RC_INVALID_ID,
    TASKS_RC_OK
};
/*!
 * \brief Enumeration of tasks
 *
 * \details This enum is mainly used to get the total number of tasks at compile
 * time but can also be used to get a specific tasks given a name in the format
 * TASKS_ID_[NAME]
 */

// clang-format off
#define TASKS_X(NAME, ENABLED, START, INTERVAL, EXEC) TASKS_ID_##NAME,
enum TasksId {
    TASKS_X_LIST
    TASKS_ID_COUNT
};
#undef TASKS_X
// clang-format on

/*!
 * \brief Definition of a single task
 *
 * \details An interval of 0 means that the task is only run once
 *
 * \param id The task identifier
 * \param start The time when the tasks is executed first
 * \param interval The amount of time that must elapsed before the tasks is
 * re-executed
 * \param exec A pointer to the task callback
 * \param enabled A boolean indicating if the task is enabled
 */
struct Task {
    enum TasksId id;
    ticks_t start;
    ticks_t interval;
    tasks_callback exec;
    bool enabled;
};

/*!
 * \brief Type definition for the tasks structure handler
 *
 * \attention This struct should not be used outside of this module
 *
 * \param fsm_event Event used to start an operation of the BMS monitor
 * \param tasks List of tasks
 */
struct TaskHandler {
    bms_monitor_fsm_event_data_t fsm_event;

    struct Task tasks[TASKS_COUNT];
};

#ifdef CONF_TASKS_MODULE_ENABLE

/*!
 * \brief Initialize the tasks module
 *
 * \param resolution The timebase resolution
 *
 * \return TasksReturnCode
 *     - TASKS_RC_OK
 */
enum TasksReturnCode tasks_init(milliseconds_t resolution);

/*!
 * \brief Get a pointer to the tasks
 *
 * \param id The identifier of the task
 *
 * \return Task* The pointer to the tasks or NULL if the id is not valid
 */
struct Task *tasks_get_task(enum TasksId task_id);

/*!
 * \brief Get the start time of the task
 *
 * \param id The identifier of the task
 *
 * \return ticks_t The task start time or 0 if the id is not valid
 */
ticks_t tasks_get_start(enum TasksId task_id);

/*!
 * \brief Get the interval time of the task
 *
 * \param id The identifier of the task
 *
 * \return ticks_t The task interval time or 0 if the id is not valid
 */
ticks_t tasks_get_interval(enum TasksId task_id);

/*!
 * \brief Get a pointer to the task callback
 *
 * \param id The identifier of the task
 *
 * \return tasks_callback The task callback or NULL if the id is not valid
 */
tasks_callback tasks_get_callback(enum TasksId task_id);

/*!
 * \brief Enable or disable a single task
 *
 * \param id The task identifier
 * \param enabled True to enable the tasks, false to disable
 *
 * \return TasksReturnCode
 *     - TASKS_INVALID_ID the given identifier does not exists
 *     - TASKS_RC_OK otherwise
 */
enum TasksReturnCode tasks_set_enable(enum TasksId task_id, bool enabled);

/*!
 * \brief Check if a task is enabled or not
 *
 * \param id The task identifier
 *
 * \return True if the task is enabled, false otherwise
 */
bool tasks_is_enabled(enum TasksId task_id);

#else // CONF_TASKS_MODULE_ENABLE

#define tasks_init(resolution) (TASKS_RC_OK)
#define tasks_get_task(id) (NULL)
#define tasks_get_start(id) (0U)
#define tasks_get_interval(id) (0U)
#define tasks_get_callback(id) (NULL)

#endif // CONF_TASKS_MODULE_ENABLE

#endif // TASKS_H
