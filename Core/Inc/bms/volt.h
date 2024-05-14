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
VoltReturnCode volt_update_value(size_t index, raw_volt value);

/**
 * @brief Get a pointer to the array where the voltage values are stored
 *
 * @return raw_volt * The pointer to the array
 */
const raw_volt * volt_get_values(void);

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
VoltReturnCode volt_dump_values(raw_volt * out, size_t start, size_t size);

/**
 * @brief Get a pointer to the CAN payload of the cells voltages
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cells_voltages_converted_t* A pointer to the payload
 */
bms_cell_voltages_converted_t * volt_get_canlib_payload(size_t * byte_size);

#else  // CONF_VOLTAGE_MODULE_ENABLE

#define volt_init() (VOLT_OK)
#define volt_update_value(index, value) (VOLT_OK)
#define volt_get_values() (NULL)
#define volt_dump_values(out, start, size) (VOLT_OK)
#define volt_get_canlib_payload(byte_size) (NULL)

#endif  // CONF_VOLTAGE_MODULE_ENABLE

#endif  // VOLT_H
