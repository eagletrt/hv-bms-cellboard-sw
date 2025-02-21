/**
 * @file cellboard-def.h
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Custom definitions used inside the project
 */

#ifndef CELLBOARD_DEF_H
#define CELLBOARD_DEF_H

#include <stdint.h>

#include "cellboard-conf.h"

/**
 * @brief No operation
 *
 * @details Does nothing
 */
#define CELLBOARD_NOPE() ((void)(0U))

/**
 * @brief Ignore the return value
 *
 * @details Used to avoid compiler warnings
 *
 * @param _ Anything that can return a value (even variables)
 */
#define CELLBOARD_UNUSED(_) ((void)(_))

/**
 * @brief Type definition for the static attribute 
 *
 * @details Can be used to test if pisseria is doing fetenderi
 * @details Can be used to disable static functions for unit testing
 */
#ifndef _STATIC
#define _STATIC static
#endif  // _STATIC

/**
 * @brief Type definition for the inline attribute 
 *
 * @details Can be used to disable inlining functions for unit testing
 */
#ifndef _STATIC_INLINE
#define _STATIC_INLINE static inline
#endif  // _STATIC_INLINE

/**
 * @brief Type definition for the volatile attribute 
 *
 * @details Can be used to disable volatile variables for unit testing
 */
#ifndef _VOLATILE
#define _VOLATILE volatile
#endif  // _VOLATILE


/*** ######################### CONSTANTS ################################# ***/

/**
 * @defgroups constants
 * @brief Constant used throughout the project
 * {@
 */

/** @brief Total number of cellboards */
#define CELLBOARD_COUNT (CELLBOARD_ID_COUNT)

/** @brief Total number of LTC chips of the cellboards */
#define CELLBOARD_SEGMENT_LTC_COUNT (2U)
#define CELLBOARD_LTC_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_LTC_COUNT))

/** @brief Number of cells handled by the LTCs */
#define CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT (12U)

/** @brief Number of cells series of a single segment */
#define CELLBOARD_SEGMENT_SERIES_COUNT ((CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) * (CELLBOARD_SEGMENT_LTC_COUNT))
/** @brief Total number of cells series */
#define CELLBOARD_SERIES_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_SERIES_COUNT))

/** @brief Number of cells parallels of a single segment */
#define CELLBOARD_SEGMENT_PARALLELS_COUNT (3U)
/** @brief Total number of cells parallels */
#define CELLBOARD_PARALLELS_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_PARALLELS_COUNT))

/** @brief Number of cells of a single segment */
#define CELLBOARD_SEGMENT_CELLS_COUNT ((CELLBOARD_SEGMENT_SERIES_COUNT) * (CELLBOARD_SEGMENT_PARALLELS_COUNT))
/** @brief Total number of cells series */
#define CELLBOARD_CELLS_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_CELLS_COUNT))


/** @brief Number of temperatures that can be read at the same time from a single segment */
#define CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT (3U)
/** @brief Total number of temperatures that can be read at the same time */
#define CELLBOARD_TEMP_CHANNEL_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT))

/** @brief Number of temperatures sensors per channel of a single segment */
#define CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT (16U)
/** @brief Total number of temperatures sensors per channel */
#define CELLBOARD_TEMP_SENSOR_PER_CHANNEL_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT))

/** @brief Number of temperatures sensors per channel of a single segment */
#define CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT ((CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT))
/** @brief Total number of temperatures sensors per channel */
#define CELLBOARD_TEMP_SENSOR_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT))

/** @brief Number of temperatures of the discharge resitors handled by the LTCs */
#define CELLBOARD_SEGMENT_DISCHARGE_TEMP_PER_LTC_COUNT (5U)

/** @brief Number of temperatures of the discharge resitors of a single segment */
#define CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT (CELLBOARD_SEGMENT_DISCHARGE_TEMP_PER_LTC_COUNT)
/** @brief Total number of temperatures of the discharge resistance that can be read at the same time from a single segment */
#define CELLBOARD_DISCHARGE_TEMP_COUNT ((CELLBOARD_COUNT) * (CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT))

/** @} */

/*** ######################### MACROS #################################### ***/

/**
 * @defgroups macros
 * @brief Macros used throughout the project
 * {@
 */

/**
 * @brief Get the minimum value between two numbers
 *
 * @param A The first number
 * @param B the second number
 *
 * @return A if it's less or equal than B, B otherwise
 */
#define CELLBOARD_MIN(A, B) (((A) <= (B)) ? (A) : (B))

/**
 * @brief Get the maximum value between two numbers
 *
 * @param A The first number
 * @param B The second number
 *
 * @return A if it's greater or equal than B, B otherwise
 */
#define CELLBOARD_MAX(A, B) (((A) >= (B)) ? (A) : (B))

/**
 * @brief Clamp a value between a certain range
 *
 * @param VAL The value to clamp
 * @param LOW The lower value of the range
 * @param UP The upper value of the range
 *
 * @return LOW if the value is lower or equal to the smallest range value, 
 *     UP if the value is greater or equal to the largest range value
 *     VAL otherwise
 */
#define CELLBOARD_CLAMP(VAL, LOW, UP) (((VAL) <= (LOW)) ? (LOW) : (((VAL) >= (UP)) ? (UP) : (VAL)))

/**
 * @brief Get the value of a specific bit of a variable
 *
 * @param VAR The variable that should be changed
 * @param BIT The position of the bit to get
 *
 * @return The value of the chosen bit
 */
#define CELLBOARD_BIT_GET(VAR, BIT) (((VAR) & (1U << (BIT))) != 0U)

/**
 * @brief Set a specific bit of a variable to 1
 *
 * @attention This macro do not modifiy the variable but returns a new value
 *
 * @param VAR The variable that should be changed
 * @param BIT The position of the bit to set
 *
 * @return The value of the modified variable
 */
#define CELLBOARD_BIT_SET(VAR, BIT) ((VAR) | (1U << (BIT)))

/**
 * @brief Set a specific bit of a variable to 0
 *
 * @attention This macro do not modifiy the variable but returns a new value
 *
 * @param VAR The variable that should be changed
 * @param BIT The position of the bit to reset
 *
 * @return The value of the modified variable
 */
#define CELLBOARD_BIT_RESET(VAR, BIT) (~((~(VAR)) | (1U << (BIT))))

/**
 * @brief Negate a specific bit of a variable
 *
 * @attention This macro do not modifiy the variable but returns a new value
 *
 * @param VAR The variable that should be changed
 * @param BIT The position of the bit to set
 *
 * @return The value of the modified variable
 */
#define CELLBOARD_BIT_TOGGLE(VAR, BIT) ((VAR) ^ (1U << (BIT)))

/**
 * @brief Set or reset a specific bit of a variable based on a boolean condition
 *
 * @attention This macro do not modify the variable but returns a new value
 *
 * @param VAR The variable that should be changed
 * @param CONDITION The boolean condition to check
 * @param BIT The position of the bit to set
 *
 * @return The value of the modified variable
 */
#define CELLBOARD_BIT_TOGGLE_IF(VAR, CONDITION, BIT) ( \
        (CONDITION) ? \
        CELLBOARD_BIT_SET(VAR, BIT) : \
        CELLBOARD_BIT_RESET(VAR, BIT) \
    )

/**
 * @brief Convert a value gathered from an ADC to a voltage in V
 *
 * @param VALUE The raw value to convert
 * @param VREF The voltage reference in V
 * @param RES The resolution of the ADC in bits
 */
#define CELLBOARD_ADC_RAW_VALUE_TO_VOLT(VALUE, VREF, RES) ((float)(VALUE) / ((1U << (RES)) - 1U) * (VREF))

/**
 * @brief Compile time assertion
 *
 * @details Useful for debugging, should be disabled when compiled for release
 */
#ifdef CONF_FULL_ASSERT_ENABLE

/**
 * @brief Make an assertion about an expression
 *
 * @details If the assertion fails a the cellboard_assert_failed function is
 * called giving the file, line, date and time parameters to get better debug info
 */
#define CELLBOARD_ASSERT(expression) ((expression) ? \
    CELLBOARD_NOPE() : \
    cellboard_assert_failed(__FILE__, __LINE__))

/**
 * @brief Debug function called when an assertion fails
 *
 * @param file The file where the assert failed
 * @param line The line where the assert failed
 */
void cellboard_assert_failed(const char * file, const int line);

#else  // CONF_FULL_ASSERT_ENABLE

#define CELLBOARD_ASSERT(expression) CELLBOARD_NOPE()

#endif  // CONF_FULL_ASSERT_ENABLE

/** @} */

/*** ######################### TYPE DEFINITIONS ########################## ***/

/**
 * @defgroups types
 * @brief Type definition for various unit of measurements
 * {@
 */

/** @brief Type definition for bit flags */
typedef uint8_t bit_flag8_t;
typedef uint16_t bit_flag16_t;
typedef uint32_t bit_flag32_t;

/** @brief Type definition for the standard CAN 2.0a and CAN 2.0b (extended) identifiers */
typedef uint16_t can_id_t;
typedef uint32_t can_ext_id_t;

/**
 * @brief Type definition for a CAN index
 * 
 * @details Used to map the can identifiers
 */
typedef int32_t can_index_t;

/** @brief Type definition for a custom amount of elapsed time */
typedef uint32_t ticks_t;

/** @brief Type definition for the time */
typedef uint32_t seconds_t;
typedef uint32_t milliseconds_t;
typedef uint32_t microseconds_t;

/**
 * @brief Raw temperature value
 * @details This type depends on the mechanism of acquisition of the temperatures
 * @details If and ADC is used the number of bits depends on its resolution
 */
typedef uint16_t raw_temp_t;

/** @brief Temperature value in °C */
typedef float celsius_t;

/**
 * @brief Raw voltage value
 * @details This type depends on the mechanism of acquisition of the voltages
 * @details If and ADC is used the number of bits depends on its resolution
 */
typedef uint16_t raw_volt_t;

/** @brief Actual voltages */
typedef float volt_t;
typedef float millivolt_t;

/** @brief Function callback that resets the microcontroller */
typedef void (* system_reset_callback_t)(void);

/** @brief Function callback used to enter a critical section */
typedef void (* interrupt_critical_section_enter_t)(void);

/** @brief Function callback used to exit a critical section */
typedef void (* interrupt_critical_section_exit_t)(void);

/** @} */

/*** ######################### ENUMS DEFINITIONS ######################### ***/

/**
 * @defgroups enums
 * @brief Definition of different enumerations used to give meaning to integer values
 * {@
 */

/**
 * @brief Definition of the cellboard indices
 * 
 * @details Each cellboard is numbered from 0 to n where n is the last cellboard
 * The real cellboard order is not guaranteed to match this order
 *
 * @details A mainboard identifier is added for utility purposes but it is not included
 * in the cellboard id count
 */
typedef enum {
    CELLBOARD_ID_0 = 0U,
    CELLBOARD_ID_1,
    CELLBOARD_ID_2,
    CELLBOARD_ID_3,
    CELLBOARD_ID_4,
    CELLBOARD_ID_5,
    CELLBOARD_ID_COUNT,
    MAINBOARD_ID
} CellboardId;

/**
 * @brief Definition of different CAN networks
 *
 * @details
 *     - CAN_NETWORK_BMS the internal network between mainboard and cellboards
 *     - CAN_NETWORK_PRIMARY the main network where all the important message are sent
 *     - CAN_NETWORK_SECONDARY network dedicated to sensor and other measuring devices
 */
typedef enum {
    CAN_NETWORK_BMS,
    CAN_NETWORK_PRIMARY,
    CAN_NETWORK_SECONDARY,
    CAN_NETWORK_COUNT
} CanNetwork;

/**
 * @brief Definition of possible CAN frame types
 *
 * @details
 *     - CAN_FRAME_TYPE_INVALID a non existent CAN frame type
 *     - CAN_FRAME_TYPE_DATA the CAN frame that contains data
 *     - CAN_FRAME_TYPE_REMOTE the CAN frame used to request a data transmission from another node in the network
 */
typedef enum {
    CAN_FRAME_TYPE_INVALID = -1,
    CAN_FRAME_TYPE_DATA,
    CAN_FRAME_TYPE_REMOTE,
    CAN_FRAME_TYPE_COUNT
} CanFrameType;

/** @} */

#endif  // CELLBOARD_DEF_H
