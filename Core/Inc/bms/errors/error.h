/*!
 * \file error.h
 * \date 2024-08-24
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Simple wrapper for the error handler generated code
 */

#ifndef ERROR_H
#define ERROR_H

#include "cellboard-def.h"
#include "cellboard-conf.h"

#include "bms_network.h"

#include "errorlib.h"

/*! \brief Error instances count for each group */
#define ERROR_GROUP_POST_INSTANCE_COUNT (1U)                                                     /*! \brief Post error instance count */
#define ERROR_GROUP_UNDER_VOLTAGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_CELLS_COUNT)                 /*! \brief Under voltage error instance count (one per cell) */
#define ERROR_GROUP_OVER_VOLTAGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_CELLS_COUNT)                  /*! \brief Over voltage error instance count (one per cell) */
#define ERROR_GROUP_UNDER_TEMPERATURE_CELLS_INSTANCE_COUNT (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) /*! \brief Under temperature cells error instance count (one per segment)*/
#define ERROR_GROUP_OVER_TEMPERATURE_CELLS_INSTANCE_COUNT (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)  /*! \brief Over temperature cells error instance count (one per segment)*/
#define ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_SERIES_COUNT)  /*! \brief Under temperature discharge error instance count (one per series) unused*/
#define ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE_INSTANCE_COUNT (CELLBOARD_SEGMENT_SERIES_COUNT)   /*! \brief Over temperature discharge error instance count (one per series) unused*/
#define ERROR_GROUP_CAN_COMMUNICATION_INSTANCE_COUNT (1U)                                        /*! \brief CAN communication error instance count (one for the whole module)*/
#define ERROR_GROUP_FLASH_INSTANCE_COUNT (1U)                                                    /*! \brief Flash error instance count (one for the whole module)*/
#define ERROR_GROUP_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT (5U)                                /*! \brief BMS monitor communication error instance count (one for configuration, one per voltage and temperature sensor, one for open-wire detection and one for poll)*/
#define ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT (1U)                                                /*! \brief Open wire error instance count (one for the whole module)*/

/*! \brief Type redefinition for an error instance */
typedef errorlib_error_instance_t error_instance_t;

/*!
 * \brief Return code for the error module functions
 */
typedef enum {
    ERROR_RC_OK,           /*!< Function executed successfully */
    ERROR_RC_NULL_POINTER, /*!< A NULL pointer was given to a function */
    ERROR_RC_UNKNOWN       /*!< An unknown error occurred */
} ErrorReturnCode;

/*!
 * \brief Type of the error that categorize a group of instances
 */
typedef enum {
    ERROR_GROUP_POST,                        /*!< Power On Self Test procedure failed */
    ERROR_GROUP_UNDER_VOLTAGE,               /*!< Voltage value is lower than a certain threshold */
    ERROR_GROUP_OVER_VOLTAGE,                /*!< Voltage value is higher than a certain threshold */
    ERROR_GROUP_UNDER_TEMPERATURE_CELLS,     /*!< Temperature value is lower than a certain threshold */
    ERROR_GROUP_OVER_TEMPERATURE_CELLS,      /*!< Temperature value is higher than a certain threshold */
    ERROR_GROUP_UNDER_TEMPERATURE_DISCHARGE, /*!< Temperature value is lower than a certain threshold */
    ERROR_GROUP_OVER_TEMPERATURE_DISCHARGE,  /*!< Temperature value is higher than a certain threshold */
    ERROR_GROUP_CAN_COMMUNICATION,           /*!< CAN bus communication is not working */
    ERROR_GROUP_FLASH,                       /*!< The flash procedure could not be completed safely */
    ERROR_GROUP_BMS_MONITOR_COMMUNICATION,   /*!< BMS monitor communication is not working */
    ERROR_GROUP_OPEN_WIRE,                   /*!< The BMS monitor detected an open-wire */
    ERROR_GROUP_COUNT                        /*<! Count of error groups, must be the last element */
} ErrorGroup;

/*!
 * \brief Error instances for the CAN communication error group
*/
typedef enum {
    ERROR_CAN_INSTANCE_BMS /*<! CAN communication error instance for the BMS*/
} ErrorCanInstance;

/*!
 * \brief Error instances for the BMS monitor communication error group
*/
typedef enum {
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION,         /*!< BMS monitor communication error instance for configuration */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE,               /*!< BMS monitor communication error instance for voltage */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE, /*!< BMS monitor communication error instance for temperature during discharge */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE,             /*!< BMS monitor communication error instance for open-wire detection */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL,                  /*!< BMS monitor communication error instance for polling */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT                  /*!< Count of BMS monitor communication error instances */
} ErrorBmsMonitorCommunicationInstance;

#ifdef CONF_ERROR_MODULE_ENABLE

/*!
 * \brief Initialization of the internal error handler structure
 * \param reset A callback function that performs a system reset, used for critical errors. It is used for can communication errors to reset the cellboard and try to recover from the error.
 *
 * \retval ERROR_RC_OK if the initialization function correctly executed
 * \retval ERROR_RC_UNKNOWN if the errorlib returned error
 * \retval ERROR_RC_NULL_POINTER if reset is not a valid function pointer
 */
ErrorReturnCode error_init(const system_reset_callback reset);

/*!
 * \brief Increments the error counter on a specific instance of an error group.
 * If an error expires the function fills the payload with the given error and enables the error-sending task.
 * 
 * \bug Consecutive errors overwrite the payload, queue implementation has to be taken into consideration.
 * 
 * \attention If the can error group is given as input to this function it will reset the micro.
 * 
 * \param group The group of the error to be incremented
 * \param instance The instance of the error to be incremented
 * 
 * \retval ERROR_RC_UNKNOWN The errorlib returned error
 * \retval ERROR_RC_OK The function executed correctly
 */
ErrorReturnCode error_set(const ErrorGroup group, const error_instance_t instance);

/*!
 * \brief Resets the error counter for a specific instance of an error group.
 *
 * \param group The group of the error to be reset
 * \param instance The instance of the error to be reset
 *
 * \retval ERROR_RC_UNKNOWN The errorlib returned error
 * \retval ERROR_RC_OK The function executed correctly
 */
ErrorReturnCode error_reset(const ErrorGroup group, const error_instance_t instance);

/*!
 * \brief Get the number of expired errors
 *
 * \return size_t The number of expired errors
 */
size_t error_get_expired(void);

/*!
 * \brief Get the information of the first expired error
 *
 * \return ErrorInfo The information of the first expired error
 */
ErrorInfo error_get_expired_info(void);

/*!
 * \brief Get a pointer to the CAN payload of cellboard error
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \return bms_cellboard_error_converted_t*  A pointer to the payload
 */
bms_cellboard_error_converted_t *error_get_error_canlib_payload(size_t *const byte_size);

#else // CONF_ERROR_MODULE_ENABLE

#define error_init() (ERROR_RC_OK)
#define error_set(group, instance) (ERROR_RC_OK)
#define error_reset(group, instance) (ERROR_RC_OK)
#define error_get_expired() (0U)
#define error_get_error_canlib_payload(byte_size) (NULL)
#define error_get_expired_info() ((ErrorInfo){ 0U })

#endif // CONF_ERROR_MODULE_ENABLE

#endif // ERROR_H
