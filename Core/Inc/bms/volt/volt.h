/*!
 * \file volt.h
 * \date 2024-04-20
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Voltage measurment and control
 */

#ifndef VOLT_H
#define VOLT_H

#include <stddef.h>

#include "cellboard-def.h"

#include "can-bms.h"

/*!
 * \defgroup VOLT_LIMIT_V Minimum and maximum allowed cell voltage in V
 * @{
 */
/* Minimum cell voltage in V */
#define VOLT_MIN_V (2.8f)
/* Maximum cell voltage in V */
#define VOLT_MAX_V (4.2f)
/*! @} */

/*!
 * \brief Type definition for the array of cells voltages
 *
 * \details This is a type definition for an array of CELLBOARD_SEGMENT_SERIES_COUNT
 * voltages, it is mainly used to force pointers to keep the information about
 * the array length
 */
typedef volt cells_volt[CELLBOARD_SEGMENT_SERIES_COUNT];

/*!
 * \brief Return code for the voltage module functions
 */
enum VoltReturnCode {
    VOLT_RC_OK,           /*!< Function executed successfully */
    VOLT_RC_NULL_POINTER, /*!< A NULL pointer is given as parameter or used inside the function */
    VOLT_RC_OUT_OF_BOUNDS /*!< An index (or pointer) value is greater/lower than the maximum/minimum allowed value */
};

/*!
 * \brief Type definition for the voltages handler structure
 */
struct VoltHandler {
    cells_volt voltages; /*<! The array of cells voltages in V */

    union CanBmsMessages libcan_message_voltage;      /*<! The canlib payload of the cells voltages */
    union CanBmsMessages libcan_message_voltage_info; /*<! The canlib payload of the cells voltage info */
};

#endif // VOLT_H
