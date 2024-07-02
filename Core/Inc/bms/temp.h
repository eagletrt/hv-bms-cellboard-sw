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

// TODO: Set minimum and maximum allowed temperatures
/** @brief Minimum and maximum allowed cell temperature in celsius */
#define TEMP_MIN_CELSIUS (-10.f)
#define TEMP_MAX_CELSIUS (100.f)

/** @brief Minimum and maximum allowed cell temperature raw values */
#define TEMP_MIN_VALUE (TEMP_CELSIUS_TO_VALUE(TEMP_MIN_CELSIUS))
#define TEMP_MAX_VALUE (TEMP_CELSIUS_TO_VALUE(TEMP_MAX_CELSIUS))

// TODO: Define conversion macros for the cells temperatures
/**
 * @brief Convert a raw temperature value to celsius
 *
 * @param value The value to convert
 *
 * @return celsius_t The converted temperature in celsius
 */
#define TEMP_VALUE_TO_CELSIUS(value) ((celsius_t)(0U))
/**
 * @brief Convert a temperature in celsius to the raw tempearture value
 *
 * @param value The value to convert in celsius
 *
 * @return raw_temp_t The raw temperature value
 */
#define TEMP_CELSIUS_TO_VALUE(value) ((raw_temp_t)(0U))

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
TempReturnCode temp_update_value(size_t index, raw_temp_t value);

/**
 * @brief Update multiple temperature values
 *
 * @param index The index of the value to update
 * @param values A pointer to the array of values to copy
 * @param size The number of elements to copy
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_values(size_t index, raw_temp_t * values, size_t size);

/**
 * @brief Update a single temperature value of the discharge resistors
 *
 * @param index The index of the value to update
 * @param value The new value
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_discharge_value(size_t index, raw_temp_t value);

/**
 * @brief Update multiple temperature values of the discharge resistors
 *
 * @param index The index of the value to update
 * @param values A pointer to the array of values to copy
 * @param size The number of elements to copy
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_discharge_values(size_t index, raw_temp_t * values, size_t size);

/**
 * @brief Get a pointer to the array where the temperature values are stored
 *
 * @return raw_temp_t * The pointer to the array
 */
const raw_temp_t * temp_get_values(void);

/**
 * @brief Get a pointer to the array where the discharge temperature values are stored
 *
 * @return raw_temp_t * The pointer to the array
 */
const raw_temp_t * temp_get_discharge_values(void);

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
TempReturnCode temp_dump_values(raw_temp_t * out, size_t start, size_t size);

/**
 * @brief Get a pointer to the CAN payload of the cells temperatures
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_cells_temperature_converted_t* A pointer to the payload
 */
bms_cellboard_cells_temperature_converted_t * temp_get_canlib_payload(size_t * byte_size);

#else  // CONF_TEMPERATURE_MODULE_ENABLE

#define temp_init() (TEMP_OK)
#define temp_update_value(index, value) (TEMP_OK)
#define temp_update_values(index, values, size) (TEMP_OK)
#define temp_update_discharge_value(index, value) (TEMP_OK)
#define temp_update_discharge_values(index, values, size) (TEMP_OK)
#define temp_get_values() (NULL)
#define temp_dump_values(out, start, size) (TEMP_OK)
#define temp_get_canlib_payload(byte_size) (NULL)

#endif // CONF_TEMPERATURE_MODULE_ENABLE

#endif  // TEMPERATURE_H
