/*!
 * \file volt-api.h
 * \date 2024-04-20
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Voltage measurment and control
 */

#ifndef VOLT_API_H
#define VOLT_API_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"
#include "volt.h"

#ifdef CONF_VOLTAGE_MODULE_ENABLE

/*!
 * \brief Initialize the voltage module
 *
 * \retval VOLT_RC_OK if the initialization is successful
 * \retval VOLT_RC_NULL_POINTER if a NULL pointer is used inside the function
 */
enum VoltReturnCode volt_init(void);

/*!
 * \brief Update a single voltage value
 *
 * \param index The index of the value to update
 * \param value The new value in V
 *
 * \retval VOLT_RC_OUT_OF_BOUNDS if the index is greater than the total number of values
 * \retval VOLT_RC_OK otherwise
 */
enum VoltReturnCode volt_update_value(size_t index, volt value);

/*!
 * \brief Update multiple voltage values
 *
 * \attention The array of values have to be a countigous memory area
 *
 * \param index The start index of the values to update
 * \param values A pointer to the array of values to copy
 * \param size The number of elements to copy
 *
 * \retval VOLT_RC_OUT_OF_BOUNDS if the index plus the size exceed the maximum number of values
 * \retval VOLT_RC_NULL_POINTER if a NULL pointer is given as parameter
 * \retval VOLT_RC_OK otherwise
 */
enum VoltReturnCode volt_update_values(
    size_t index,
    volt *values,
    size_t size);

/*!
 * \brief Get a pointer to the array where the voltage values are stored
 *
 * \returns cells_volt* The pointer to the array
 */
const cells_volt *volt_get_values(void);

/*!
 * \brief Get the minimum cell voltage
 *
 * \returns volt The minimum voltage in V
 */
volt volt_get_min(void);

/*!
 * \brief Get the maximum cell voltage
 *
 * \returns volt The maximum voltage in V
 */
volt volt_get_max(void);

/*!
 * \brief Get the average cell voltage
 *
 * \returns volt The average voltage in V
 */
volt volt_get_avg(void);

/*!
 * \brief Get the sum of the cells voltages
 *
 * \returns volt The cells voltages sum
 */
volt volt_get_sum(void);

/*!
 * \brief Get a bitmask of cells which voltage is STRICTLY greater than
 * the given target value
 *
 * \attention The maximum number of cells this function can handle is equal
 * to the maximum number of bits of the returned variable (32 in this case)
 *
 * \details In the returned bitmask the n-th bit represent the n-th cell and
 * if the bit value is 1 the cell voltage is greater than the target, less or
 * equal otherwise
 *
 * \param target The target voltage in V
 *
 * \returns bit_flag32 The bitmask of cells
 */
bit_flag32 volt_select_values_above_target(volt target);

/*!
 * \brief Copy a list of adjacent voltages
 *
 * \attention The out array should be large enough to store the required data
 *
 * \param out[out] The array where the values are copied into
 * \param start The index of the first element to copy
 * \param size The number of element that should be copied
 *
 * \retval VOLT_RC_NULL_POINTER if NULL is passed as parameter
 * \retval VOLT_RC_OUT_OF_BOUNDS if the required range exceeds the maximum number of voltages
 * \retval VOLT_RC_OK otherwise
 */
enum VoltReturnCode volt_dump_values(
    volt *out,
    size_t start,
    size_t size);

/*!
 * \brief Get a pointer to the CAN payload of the cells voltages
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \returns bms_cellboard_cells_voltage_converted_t* A pointer to the payload
 */
bms_cellboard_cells_voltage_converted_t *volt_get_canlib_payload(size_t *byte_size);

#else // CONF_VOLTAGE_MODULE_ENABLE

#define volt_init() (VOLT_RC_OK)
#define volt_update_value(index, value) (VOLT_RC_OK)
#define volt_update_values(index, value, size) (VOLT_RC_OK)
#define volt_get_values() (NULL)
#define volt_select_values_above_target(target) (0U)
#define volt_dump_values(out, start, size) (VOLT_RC_OK)
#define volt_get_canlib_payload(byte_size) (NULL)

#endif // CONF_VOLTAGE_MODULE_ENABLE

#endif // VOLT_API_H
