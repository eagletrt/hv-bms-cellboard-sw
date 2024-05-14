/**
 * @file temp.h
 * @date 2024-04-19
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Temperature measurment and control
 */

#ifndef TEMP_H
#define TEMP_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"

/**
 * @brief Return code for the temperature module functions
 *
 * @details
 *     - TEMP_OK the function executed successfully
 *     - TEMP_NULL_POINTER a NULL pointer is given as parameter or used inside the function
 *     - TEMP_OUT_OF_BOUNDS an index (or pointer) value is greater/lower than the maximum/minimum allowed value
 */
typedef enum {
    TEMP_OK,
    TEMP_NULL_POINTER,
    TEMP_OUT_OF_BOUNDS
} TempReturnCode;

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

/**
 * @brief Initialize the temperature module
 *
 * @return TempReturnCode
 *     - TEMP_OK
 */
TempReturnCode temp_init(void);

/**
 * @brief Update a single temperature value
 *
 * @param index The index of the value to update
 * @param value The new value
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_value(size_t index, raw_temp value);

/**
 * @brief Get a pointer to the array where the temperature values are stored
 *
 * @return raw_temp * The pointer to the array
 */
const raw_temp * temp_get_values(void);

/**
 * @brief Copy a list of adjacent temperatures
 *
 * @attention The out array should be large enough to store the required data
 *
 * @param out The array where the values are copied into
 * @param start The index of the first element to copy
 * @param size The number of element that should be copied
 *
 * @return TempReturnCode
 *     - TEMP_NULL_POINTER if NULL is passed as parameter
 *     - TEMP_OUT_OF_BOUNDS if the required range exceeds the maximum number of temperatures
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_dump_values(raw_temp * out, size_t start, size_t size);

/**
 * @brief Get a pointer to the CAN payload of the cells temperatures
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cells_temperatures_converted_t* A pointer to the payload
 */
bms_cell_temperatures_converted_t * temp_get_canlib_payload(size_t * byte_size);

#else  // CONF_TEMPERATURE_MODULE_ENABLE

#define temp_init() (TEMP_OK)
#define temp_update_value(index, value) (TEMP_OK)
#define temp_get_values() (NULL)
#define temp_dump_values(out, start, size) (TEMP_OK)
#define temp_get_canlib_payload(byte_size) (NULL)

#endif // CONF_TEMPERATURE_MODULE_ENABLE

#endif  // TEMPERATURE_H
