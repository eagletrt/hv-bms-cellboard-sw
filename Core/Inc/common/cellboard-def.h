/*!
 * \file cellboard-def.h
 * \date 2024-04-16
 * \author Antonio Gelain [antonio.gelain2\gmail.com]
 * \author Alessandro Giustina [giustinalessandro\gmail.com]
 *
 * \brief Custom definitions used inside the project
 */

#ifndef CELLBOARD_DEF_H
#define CELLBOARD_DEF_H

#include <stdint.h>

#include "cellboard-conf.h"

/*!* ######################### CONSTANTS ################################# *!*/

/*!
 * \defgroups constants
 * \brief Constant used throughout the project
 * {\
 */

/*! \brief Total number of cellboards */
#define CELLBOARD_COUNT (CELLBOARD_ID_COUNT)

/*! \brief Total number of LTC chips of the cellboards */
#define CELLBOARD_SEGMENT_LTC_COUNT (2U)
#define CELLBOARD_LTC_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_LTC_COUNT))

/*! \brief Number of cells handled by the LTCs */
#define CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT (12U)

/*! \brief Number of cells series of a single segment */
#define CELLBOARD_SEGMENT_SERIES_COUNT ((CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) * (CELLBOARD_SEGMENT_LTC_COUNT))
/*! \brief Total number of cells series */
#define CELLBOARD_SERIES_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_SERIES_COUNT))

/*! \brief Number of cells parallels of a single segment */
#define CELLBOARD_SEGMENT_PARALLELS_COUNT (3U)
/*! \brief Total number of cells parallels */
#define CELLBOARD_PARALLELS_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_PARALLELS_COUNT))

/*! \brief Number of cells of a single segment */
#define CELLBOARD_SEGMENT_CELLS_COUNT ((CELLBOARD_SEGMENT_SERIES_COUNT) * (CELLBOARD_SEGMENT_PARALLELS_COUNT))
/*! \brief Total number of cells series */
#define CELLBOARD_CELLS_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_CELLS_COUNT))

/*! \brief Number of temperatures that can be read at the same time from a single segment */
#define CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT (3U)
/*! \brief Total number of temperatures that can be read at the same time */
#define CELLBOARD_TEMP_CHANNEL_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT))

/*! \brief Number of temperatures sensors per channel of a single segment */
#define CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT (16U)
/*! \brief Total number of temperatures sensors per channel */
#define CELLBOARD_TEMP_SENSOR_PER_CHANNEL_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT))

/*! \brief Number of temperatures sensors per channel of a single segment */
#define CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT ((CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT))
/*! \brief Total number of temperatures sensors per channel */
#define CELLBOARD_TEMP_SENSOR_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT))

/*! \brief Number of temperatures of the discharge resitors handled by the LTCs */
#define CELLBOARD_SEGMENT_DISCHARGE_TEMP_PER_LTC_COUNT (5U)

/*! \brief Number of temperatures of the discharge resitors of a single segment */
#define CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT (CELLBOARD_SEGMENT_DISCHARGE_TEMP_PER_LTC_COUNT)
/*! \brief Total number of temperatures of the discharge resistance that can be read at the same time from a single segment */
#define CELLBOARD_DISCHARGE_TEMP_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT))

/*! \} */

/*!* ######################### MACROS #################################### *!*/

/*!
 * \defgroups macros
 * \brief Macros used throughout the project
 * {\
 */

/*!
 * \brief Convert a value gathered from an ADC to a voltage in V
 *
 * \param VALUE The raw value to convert
 * \param VREF The voltage reference in V
 * \param RES The resolution of the ADC in bits
 */
#define CELLBOARD_ADC_RAW_VALUE_TO_VOLT(VALUE, VREF, RES) ((float)(VALUE) / ((1U << (RES)) - 1U) * (VREF))

/*! \} */

/*!* ######################### TYPE DEFINITIONS ########################## *!*/

/*!
 * \defgroups types
 * \brief Type definition for various unit of measurements
 * {\
 */

/*! \brief Type definition for bit flags */
typedef uint8_t bit_flag8_t;
typedef uint16_t bit_flag16_t;
typedef uint32_t bit_flag32;

/*! \brief Type definition for the standard CAN 2.0a and CAN 2.0b (extended) identifiers */
typedef uint16_t can_id_t;
typedef uint32_t can_ext_id_t;

/*!
 * \brief Type definition for a CAN index
 *
 * \details Used to map the can identifiers
 */
typedef int32_t can_index_t;

/*! \brief Type definition for a custom amount of elapsed time */
typedef uint32_t ticks_t;

/*! \brief Type definition for the time for backwards compatibility*/
typedef uint32_t seconds_t;
typedef uint32_t milliseconds_t;
typedef uint32_t microseconds_t;

/*! \brief Type definition for the time*/
typedef uint32_t seconds;
typedef uint32_t milliseconds;
typedef uint32_t microseconds;

/*!
 * \brief Raw temperature value
 * \details This type depends on the mechanism of acquisition of the temperatures
 * \details If and ADC is used the number of bits depends on its resolution
 */
typedef uint16_t raw_temp_t;

/*! \brief Temperature value in °C */
typedef float celsius;

/*!
 * \brief Raw voltage value
 * \details This type depends on the mechanism of acquisition of the voltages
 * \details If and ADC is used the number of bits depends on its resolution
 */
typedef uint16_t raw_volt;

/*! \brief Actual voltages */
typedef float volt;
typedef float millivolt;

/*! \brief Function callback that resets the microcontroller */
typedef void (*system_reset_callback)(void);

/*! \brief Function callback used to enter a critical section */
typedef void (*interrupt_critical_section_enter_t)(void);

/*! \brief Function callback used to exit a critical section */
typedef void (*interrupt_critical_section_exit_t)(void);

/*! \} */

/*!* ######################### ENUMS DEFINITIONS ######################### *!*/

/*!
 * \defgroups enums
 * \brief Definition of different enumerations used to give meaning to integer values
 * {\
 */

/*!
 * \brief Definition of the cellboard indices
 *
 * \details Each cellboard is numbered from 0 to n where n is the last cellboard
 * The real cellboard order is not guaranteed to match this order
 *
 * \details A mainboard identifier is added for utility purposes but it is not included
 * in the cellboard id count
 */
enum CellboardId {
    CELLBOARD_ID_0 = 0U,
    CELLBOARD_ID_1,
    CELLBOARD_ID_2,
    CELLBOARD_ID_3,
    CELLBOARD_ID_4,
    CELLBOARD_ID_5,
    CELLBOARD_ID_COUNT,
    MAINBOARD_ID
};

/*! \} */

#endif // CELLBOARD_DEF_H
