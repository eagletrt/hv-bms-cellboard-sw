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

#include "can-bms.h"

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
#define ERROR_GROUP_OPEN_WIRE_INSTANCE_COUNT (13U)                                               /*! \brief Open wire error instance count (one for the whole module)*/

/*! \brief Type redefinition for an error instance */
typedef errorlib_error_instance_t error_instance_t;

/*!
 * \brief Return code for the error module functions
 */
enum ErrorReturnCode {
    ERROR_RC_OK,           /*!< Function executed successfully */
    ERROR_RC_NULL_POINTER, /*!< A NULL pointer was given to a function */
    ERROR_RC_UNKNOWN       /*!< An unknown error occurred */
};

/*!
 * \brief Type of the error that categorize a group of instances
 */
enum ErrorGroup {
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
};

/*!
 * \brief Error instances for the CAN communication error group
*/
enum ErrorCanInstance {
    ERROR_CAN_INSTANCE_BMS /*<! CAN communication error instance for the BMS*/
};

/*!
 * \brief Error instances for the BMS monitor communication error group
*/
enum ErrorBmsMonitorCommunicationInstance {
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION,         /*!< BMS monitor communication error instance for configuration */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE,               /*!< BMS monitor communication error instance for voltage */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE, /*!< BMS monitor communication error instance for temperature during discharge */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE,             /*!< BMS monitor communication error instance for open-wire detection */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL,                  /*!< BMS monitor communication error instance for polling */
    ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_COUNT                  /*!< Count of BMS monitor communication error instances */
};

#endif // ERROR_H
