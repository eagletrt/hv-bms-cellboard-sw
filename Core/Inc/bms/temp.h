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

/** @brief Minimum and maximum allowed cell temperature in celsius */
#define TEMP_MIN_C (-10.f)
#define TEMP_MAX_C (60.f)

/**
 * @brief Minimum and maximum limit for the temperature voltages in V
 *
 * @details This limit is applied to fit into the polynomial conversion
 * to get a plausible temperature value
 */
#define TEMP_MIN_LIMIT_V (0.f)
#define TEMP_MAX_LIMIT_V (3.f)

/** @brief Coefficients used for the polynomial conversion of the NTC temperatures values */
#define TEMP_COEFF_0 ( 148.305319086073000)
#define TEMP_COEFF_1 (-317.553729396941300)
#define TEMP_COEFF_2 ( 444.564306449468700)
#define TEMP_COEFF_3 (-378.912004657724100)
#define TEMP_COEFF_4 ( 180.457759604731300)
#define TEMP_COEFF_5 (- 44.504609710405890)
#define TEMP_COEFF_6 (   4.399756702462762)

/**
 * @brief Type definition for a function callback that sets the muliplexer address
 * 
 * @param address The address to set
 */
typedef void (* temp_set_mux_address_callback_t)(const uint8_t address);

/** @brief Type definition for a function callback that starts the ADC conversion */
typedef void (* temp_start_conversion_callback_t)(void);

/** @brief Type definition for the array of cells and discharge temperatures */
typedef celsius_t cells_temp_t[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT];
typedef celsius_t discharge_temp_t[CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT];

/**
 * @brief Return code for the temperature module functions
 *
 * @details
 *     - TEMP_OK the function executed successfully
 *     - TEMP_BUSY the module is busy and cannot execute the requested function
 *     - TEMP_NULL_POINTER a NULL pointer is given as parameter or used inside the function
 *     - TEMP_OUT_OF_BOUNDS an index (or pointer) value is greater/lower than the maximum/minimum allowed value
 */
typedef enum {
    TEMP_OK,
    TEMP_NULL_POINTER,
    TEMP_BUSY,
    TEMP_OUT_OF_BOUNDS
} TempReturnCode;

/**
 * @brief Type definition for the temperature module handler structure
 *
 * @attention Do not use this structure outside of this module
 *
 * @param set_address A pointer to the function callback used to set the multiplexer address
 * @param start_conversion A pointer to the function callback used to start the ADC conversion
 * @param busy Flag that is true if the ADC is busy making conversions
 * @param address The current address of the multiplexer
 * @param temperature The cells temperature values in °C
 * @param discharge_temperature The discharge resistors temperature values in °C
 * @param temp_can_payload The canlib payload used to send the cells temperatures data via CAN
 * @param offset An offset used when the canlib payload is sent
 */
typedef struct {
    temp_set_mux_address_callback_t set_address;
    temp_start_conversion_callback_t start_conversion;

    bool busy;
    uint8_t address;
    cells_temp_t temperatures;
    discharge_temp_t discharge_temperatures;

    bms_cellboard_cells_temperature_converted_t temp_can_payload;
    size_t offset;
} _TempHandler;


#ifdef CONF_TEMPERATURE_MODULE_ENABLE

/**
 * @brief Initialize the temperature module
 *
 * @param set_address A pointer to the function callback used to set the multiplexer address
 * @param start_conversion A pointer to the function callback used to start the ADC conversion
 *
 * @return TempReturnCode
 *     - TEMP_NULL_POINTER if any of the parameters are NULL
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_init(const temp_set_mux_address_callback_t set_address, const temp_start_conversion_callback_t start_conversion);

/**
 * @brief Start the ADC conversion to get the cells temperature values
 *
 * @return TempRetutrnCode
 *     - TEMP_OK
 */
TempReturnCode temp_start_conversion(void);

/**
 * @brief Notify the temperature module that the conversion is completed
 *
 * @param values A pointer to the array of voltages to copy in V
 * @param size The number of elements to copy
 */
TempReturnCode temp_notify_conversion_complete(const volt_t * const values, const size_t size);

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
TempReturnCode temp_update_value(const size_t index, const celsius_t value);

/**
 * @brief Update multiple temperature values
 *
 * @param index The index of the value to update
 * @param values A pointer to the array of temperatures values to copy
 * @param size The number of elements to copy
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_values(
    const size_t index,
    const celsius_t * const values,
    const size_t size
);

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
TempReturnCode temp_update_discharge_value(const size_t index, const celsius_t value);

/**
 * @brief Update multiple temperature values of the discharge resistors
 *
 * @param index The index of the value to update
 * @param values A pointer to the array of temperatures values to copy
 * @param size The number of elements to copy
 *
 * @return TempReturnCode
 *     - TEMP_OUT_OF_BOUNDS if the index is greater than the total number of values
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_update_discharge_values(
    const size_t index,
    const celsius_t * const values,
    const size_t size
);

/**
 * @brief Get a pointer to the array where the temperature values are stored
 *
 * @return cells_temp_t* The pointer to the array
 */
const cells_temp_t * temp_get_values(void);

/**
 * @brief Get a pointer to the array where the discharge temperature values are stored
 *
 * @return raw_temp_t* The pointer to the array
 */
const discharge_temp_t * temp_get_discharge_values(void);

/**
 * @brief Copy a list of adjacent temperatures
 *
 * @attention The out array should be large enough to store the required data
 *
 * @param out The array where the temperatures values are copied into
 * @param start The index of the first element to copy
 * @param size The number of element that should be copied
 *
 * @return TempReturnCode
 *     - TEMP_NULL_POINTER if NULL is passed as parameter
 *     - TEMP_OUT_OF_BOUNDS if the required range exceeds the maximum number of temperatures
 *     - TEMP_OK otherwise
 */
TempReturnCode temp_dump_values(
    celsius_t * const out,
    const size_t start,
    const size_t size
);

/**
 * @brief Get a pointer to the CAN payload of the cells temperatures
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_cells_temperature_converted_t* A pointer to the payload
 */
bms_cellboard_cells_temperature_converted_t * temp_get_cells_temp_canlib_payload(size_t * const byte_size);

#else  // CONF_TEMPERATURE_MODULE_ENABLE

#define temp_init() (TEMP_OK)
#define temp_update_value(index, value) (TEMP_OK)
#define temp_update_values(index, values, size) (TEMP_OK)
#define temp_update_discharge_value(index, value) (TEMP_OK)
#define temp_update_discharge_values(index, values, size) (TEMP_OK)
#define temp_get_values() (NULL)
#define temp_dump_values(out, start, size) (TEMP_OK)
#define temp_get_cells_temp_canlib_payload(byte_size) (NULL)

#endif // CONF_TEMPERATURE_MODULE_ENABLE

#endif  // TEMPERATURE_H
