/**
 * @file volt.h
 * @date 2024-04-20
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Voltage measurment and control
 */

#ifndef VOLT_H
#define VOLT_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"

/** @brief Minimum and maximum allowed cell voltage in millivolt */
#define VOLT_MIN_MILLIVOLT (2800.f)
#define VOLT_MAX_MILLIVOLT (4200.f)

/** @brief Minimum and maximum allowed cell voltage raw values */
#define VOLT_MIN_VALUE (VOLT_MILLIVOLT_TO_VALUE(VOLT_MIN_MILLIVOLT))
#define VOLT_MAX_VALUE (VOLT_MILLIVOLT_TO_VALUE(VOLT_MAX_MILLIVOLT))

/**
 * @brief Convert a raw voltage value to millivolt
 *
 * @param value The value to convert
 *
 * @return millivolt_t The converted voltage in millivolt
 */
#define VOLT_VALUE_TO_MILLIVOLT(value) ((millivolt_t)(value / 10.f))
/**
 * @brief Convert a voltage in millivolt to the raw voltage value
 *
 * @param value The value to convert in millivolt
 *
 * @return raw_volt_t The raw voltage value
 */
#define VOLT_MILLIVOLT_TO_VALUE(value) ((raw_volt_t)(value * 10.f))

/**
 * @brief Return code for the voltage module functions
 *
 * @details
 *     - VOLT_OK the function executed successfully
 *     - VOLT_NULL_POINTER a NULL pointer is given as parameter or used inside the function
 *     - VOLT_OUT_OF_BOUNDS an index (or pointer) value is greater/lower than the maximum/minimum allowed value
 */
typedef enum {
    VOLT_OK,
    VOLT_NULL_POINTER,
    VOLT_OUT_OF_BOUNDS
} VoltReturnCode;


/**
 * @brief Voltages handler structure
 *
 * @param voltages The array of raw cells voltages
 * @param can_payload The canlib payload of the cells voltages
 */
typedef struct {
    raw_volt_t voltages[CELLBOARD_SEGMENT_SERIES_COUNT];
    bms_cellboard_cells_voltage_converted_t can_payload;
} _VoltHandler;

#ifdef CONF_VOLTAGE_MODULE_ENABLE

/**
 * @brief Initialize the voltage module
 *
 * @return VoltReturnCode
 *     - VOLT_OK
 */
VoltReturnCode volt_init(void);

/**
 * @brief Update a single voltage value
 *
 * @param index The index of the value to update
 * @param value The new value
 *
 * @return VoltReturnCode
 *     - VOLT_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - VOLT_OK otherwise
 */
VoltReturnCode volt_update_value(size_t index, raw_volt_t value);

/**
 * @brief Update multiple voltage values
 *
 * @attention The array of values should be a countigous memory area
 *
 * @param index The start index of the values to update
 * @param values A pointer to the array of values to copy
 * @param size The number of elements to copy
 *
 * @return VoltReturnCode
 *     - VOLT_OUT_OF_BOUNDS if the index plus the size exceed the maximum number of values
 *     - VOLT_OK otherwise
 */
VoltReturnCode volt_update_values(size_t index, raw_volt_t * values, size_t size);

/**
 * @brief Get a pointer to the array where the voltage values are stored
 *
 * @return raw_volt_t * The pointer to the array
 */
const raw_volt_t * volt_get_values(void);

/**
 * @brief Get a bitmask of cells which voltage is STRICTLY greater than
 * the given target value
 *
 * @attention The maximum number of cells this function can handle is equal
 * to the maximum number of bits of the returned variable (32 in this case)
 *
 * @details In the returned bitmask the n-th bit represent the n-th cell and
 * if the bit value is 1 the cell voltage is greater than the target, less or
 * equal otherwise
 *
 * @param target The target voltage in mV
 *
 * @return bit_flag32_t The bitmask of cells
 */
bit_flag32_t volt_select_values(millivolt_t target);

/**
 * @brief Copy a list of adjacent voltages
 *
 * @attention The out array should be large enough to store the required data
 *
 * @param out The array where the values are copied into
 * @param start The index of the first element to copy
 * @param size The number of element that should be copied
 *
 * @return VoltReturnCode
 *     - VOLT_NULL_POINTER if NULL is passed as parameter
 *     - VOLT_OUT_OF_BOUNDS if the required range exceeds the maximum number of voltages
 *     - VOLT_OK otherwise
 */
VoltReturnCode volt_dump_values(raw_volt_t * out, size_t start, size_t size);

/**
 * @brief Get a pointer to the CAN payload of the cells voltages
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_cells_voltage_converted_t* A pointer to the payload
 */
bms_cellboard_cells_voltage_converted_t * volt_get_canlib_payload(size_t * byte_size);

#else  // CONF_VOLTAGE_MODULE_ENABLE

#define volt_init() (VOLT_OK)
#define volt_update_value(index, value) (VOLT_OK)
#define volt_update_values(index, value) (VOLT_OK)
#define volt_get_values() (NULL)
#define volt_select_values(target) (0U)
#define volt_dump_values(out, start, size) (VOLT_OK)
#define volt_get_canlib_payload(byte_size) (NULL)

#endif  // CONF_VOLTAGE_MODULE_ENABLE

#endif  // VOLT_H
