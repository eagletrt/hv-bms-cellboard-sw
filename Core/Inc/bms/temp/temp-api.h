/*!
 * \file temp-api.h
 * \date 2024-04-19
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Temperature measurment and control
 */

#ifndef TEMP_API_H
#define TEMP_API_H

#include "temp.h"

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

/*!
 * \brief Initialize the temperature module
 *
 * \param set_address A pointer to the function callback used to set the multiplexer address
 * \param start_conversion A pointer to the function callback used to start the ADC conversion
 *
 * \retval TEMP_RC_OK if the module is initialized successfully
 * \retval TEMP_RC_NULL_POINTER if a NULL pointer is given as parameter
 */
enum TempReturnCode temp_api_init(temp_set_mux_address_callback set_address, temp_start_conversion_callback start_conversion);

/*!
 * \brief Start the ADC conversion to get the cells temperature values
 *
 * \retval TEMP_RC_OK if the conversion is started successfully
 * \retval TEMP_RC_BUSY if the module is already busy making a conversion
 */
enum TempReturnCode temp_api_start_conversion(void);

/*!
 * \brief Notify the temperature module that the conversion is completed
 *
 * \param values A pointer to the array of voltages to copy in V
 * \param size The number of elements to copy
 *
 * \retval TEMP_RC_OK
 */
enum TempReturnCode temp_api_notify_conversion_complete(const volt *values, size_t size);

/*!
 * \brief Update a single temperature value
 *
 * \param index The index of the value to update
 * \param value The new value
 *
 * \retval TEMP_RC_OUT_OF_BOUNDS if the index is greater than the total number of values
 * \retval TEMP_RC_OK otherwise
 */
enum TempReturnCode temp_api_update_value(size_t index, celsius value);

/*!
 * \brief Update multiple temperature values
 *
 * \param index The index of the value to update
 * \param values A pointer to the array of temperatures values to copy
 * \param size The number of elements to copy
 *
 * \retval TEMP_RC_OUT_OF_BOUNDS if the index is greater than the total number of values
 * \retval TEMP_RC_OK otherwise
 */
enum TempReturnCode temp_api_update_values(
    size_t index,
    const celsius *values,
    size_t size);

/*!
 * \brief Update a single temperature value of the discharge resistors
 *
 * \param index The index of the value to update
 * \param value The value read from the ADC in V
 *
 * \retval TEMP_RC_OUT_OF_BOUNDS if the index is greater than the total number of values
 * \retval TEMP_RC_OK otherwise
 */
enum TempReturnCode temp_api_update_discharge_value(size_t index, volt value);

/*!
 * \brief Update multiple temperature values of the discharge resistors
 *
 * \param index The index of the value to update
 * \param values A pointer to the array of ADC voltage values to copy in V
 * \param size The number of elements to copy
 *
 * \retval TEMP_RC_OUT_OF_BOUNDS if the index is greater than the total number of values
 * \retval TEMP_RC_OK otherwise
 */
enum TempReturnCode temp_api_update_discharge_values(
    size_t index,
    const volt *values,
    size_t size);

/*!
 * \brief Get a pointer to the array where the temperature values are stored
 *
 * \returns cells_temp* The pointer to the array
 */
const cells_temp *temp_api_get_values(void);

/*!
 * \brief Get the minimum cell temperature in the pack
 *
 * \returns celsius The minimum temperature value in °C
 */
celsius temp_api_get_min(void);

/*!
 * \brief Get the maximum cell temperature in the pack
 *
 * \returns celsius The maximum temperature value in °C
 */
celsius temp_api_get_max(void);

/*!
 * \brief Get the sum of the cells temperatures of the pack
 *
 * \returns celsius The sum of the temperatures in °C
 */
celsius temp_api_get_sum(void);

/*!
 * \brief Get the average cell temperature of the pack
 *
 * \returns celsius The average temperature in °C
 */
celsius temp_api_get_avg(void);

/*!
 * \brief Get a pointer to the array where the discharge temperature values are stored
 *
 * \returns raw_temp_t* The pointer to the array
 */
const discharge_temp *temp_api_get_discharge_values(void);

/*!
 * \brief Copy a list of adjacent temperatures
 *
 * \attention The out array should be large enough to store the required data
 *
 * \param out The array where the temperatures values are copied into
 * \param start The index of the first element to copy
 * \param size The number of element that should be copied
 *
 * \retval TEMP_RC_NULL_POINTER if NULL is passed as parameter
 * \retval TEMP_RC_OUT_OF_BOUNDS if the required range exceeds the maximum number of temperatures
 * \retval TEMP_RC_OK otherwise
 */
enum TempReturnCode temp_api_dump_values(
    celsius *out,
    size_t start,
    size_t size);

/*!
 * \brief Get a pointer to the CAN payload of the cells temperatures
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \returns bms_cellboard_cells_temperature_converted_t* A pointer to the payload
 */
bms_cellboard_cells_temperature_converted_t *temp_api_get_cells_temp_canlib_payload(size_t *byte_size);

/*!
 * \brief Get a pointer to the CAN payload of the discharge resistors temperature
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \returns bms_cellboard_discharge_temperature_converted_t* A pointer to the payload
 */
bms_cellboard_discharge_temperature_converted_t *temp_api_get_discharge_temp_canlib_payload(size_t *byte_size);

#else // CONF_TEMPERATURE_MODULE_ENABLE

#define temp_api_init() (TEMP_RC_OK)
#define temp_api_update_value(index, value) (TEMP_RC_OK)
#define temp_api_update_values(index, values, size) (TEMP_RC_OK)
#define temp_api_update_discharge_value(index, value) (TEMP_RC_OK)
#define temp_api_update_discharge_values(index, values, size) (TEMP_RC_OK)
#define temp_api_get_values() (NULL)
#define temp_api_dump_values(out, start, size) (TEMP_RC_OK)
#define temp_api_get_cells_temp_canlib_payload(byte_size) (NULL)
#define temp_api_get_discharge_temp_canlib_payload(byte_size) (NULL)

#endif // CONF_TEMPERATURE_MODULE_ENABLE

#endif // TEMP_API_H
