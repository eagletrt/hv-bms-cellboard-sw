/**
 * @file temp.c
 * @date 2024-04-19
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Temperature measurment and control
 */

#include "temp.h"

#include <string.h>

#include "cellboard-def.h"
#include "identity.h"
#include "error.h"
#include "timebase.h"

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

// TODO: Send discharge temperatures
_STATIC _TempHandler htemp;

/**
 * @brief Convert a voltage into a temperature using a polynomial conversion
 *
 * @param value The voltage value in V
 *
 * @return celsius_t The converted value in °C
 */
celsius_t _temp_volt_to_celsius(volt_t value) {
    // Value is converted in V and limited to fit the polynomial range
    value = CELLBOARD_CLAMP(value, TEMP_MIN_LIMIT_V, TEMP_MAX_LIMIT_V);
    const double v = value;
    const double v2 = v * v;
    const double v3 = v2 * v;
    const double v4 = v2 * v2;
    const double v5 = v4 * v;
    const double v6 = v3 * v3;
    return TEMP_COEFF_0 + 
        TEMP_COEFF_1 * v + 
        TEMP_COEFF_2 * v2 + 
        TEMP_COEFF_3 * v3 + 
        TEMP_COEFF_4 * v4 + 
        TEMP_COEFF_5 * v5 +
        TEMP_COEFF_6 * v6;
}

/**
 * @brief Convert the discharge temp voltage value into a temperature in °C using
 * a polynomial conversion
 *
 * @param value The voltage value in V
 *
 * @return celsius_t The converted value in °C
 */
celsius_t _temp_discharge_volt_to_celsius(volt_t value) {
    // Value is converted in V and limited to fit the polynomial range
    value = CELLBOARD_CLAMP(value, TEMP_DISCHARGE_MIN_LIMIT_V, TEMP_DISCHARGE_MAX_LIMIT_V);
    const double v = value;
    const double v2 = v * v;
    const double v3 = v2 * v;
    const double v4 = v2 * v2;
    const double v5 = v4 * v;
    // const double v6 = v3 * v3;
    return TEMP_DISCHARGE_COEFF_0 + 
        TEMP_DISCHARGE_COEFF_1 * v + 
        TEMP_DISCHARGE_COEFF_2 * v2 + 
        TEMP_DISCHARGE_COEFF_3 * v3 + 
        TEMP_DISCHARGE_COEFF_4 * v4 + 
        TEMP_DISCHARGE_COEFF_5 * v5;
        // TEMP_DISCHARGE_COEFF_6 * v6;
}

/**
 * @brief Check if the cells temperature values are in range otherwise set an error
 *
 * @param value The temperature value to check in °C
 */
_STATIC_INLINE void _temp_check_cells_value(const size_t index, const celsius_t value) {
    if (value <= TEMP_MIN_C)
        error_set(ERROR_GROUP_UNDER_TEMPERATURE, index);
    else
        error_reset(ERROR_GROUP_UNDER_TEMPERATURE, index);
    if (value >= TEMP_MAX_C)
        error_set(ERROR_GROUP_OVER_TEMPERATURE, index);
    else
        error_reset(ERROR_GROUP_OVER_TEMPERATURE, index);
}

TempReturnCode temp_init(const temp_set_mux_address_callback_t set_address, const temp_start_conversion_callback_t start_conversion) {
    if (set_address == NULL || start_conversion == NULL)
        return TEMP_NULL_POINTER;
    memset(&htemp, 0U, sizeof(htemp));

    // Copy callback pointers
    htemp.set_address = set_address;
    htemp.start_conversion = start_conversion;
    htemp.temp_can_payload.cellboard_id = (bms_cellboard_cells_temperature_cellboard_id)identity_get_cellboard_id();
    return TEMP_OK;
}

TempReturnCode temp_start_conversion(void) {
    if (htemp.busy)
        return TEMP_BUSY;
    // Set busy flag
    htemp.busy = true;

    // Set mux address and start conversion
    if (++htemp.address >= CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT)
        htemp.address = 0U;
    htemp.set_address(htemp.address);
    htemp.start_conversion();
    return TEMP_OK;
}

TempReturnCode temp_notify_conversion_complete(const volt_t * const values, size_t size) {
    const size_t index = htemp.address * CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT;
    // Convert the raw value to celsius
    for (size_t i = 0U; i < size; ++i) {
        const celsius_t temp = _temp_volt_to_celsius(values[i]);
        temp_update_value(index + i, temp);
    }
    htemp.busy = false;
    return TEMP_OK;
}

TempReturnCode temp_update_value(const size_t index, const celsius_t value) {
    if (index > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.temperatures[index] = value;
    _temp_check_cells_value(index, value);
    return TEMP_OK;
}

TempReturnCode temp_update_values(
    const size_t index,
    const celsius_t * const values,
    const size_t size)
{
    if (index + size > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    for (size_t i = 0U; i < size; ++i) {
        htemp.temperatures[index + i] = values[i];
        _temp_check_cells_value(index + i, values[index + i]);
    }
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_value(const size_t index, const volt_t value) {
    if (index > CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.discharge_temperatures[index] = _temp_discharge_volt_to_celsius(value);
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_values(
    const size_t index,
    const volt_t * const values,
    const size_t size)
{
    if (index + size >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    for (size_t i = 0U; i < size; ++i)
        htemp.discharge_temperatures[index + i] = _temp_discharge_volt_to_celsius(values[i]);
    return TEMP_OK;
}

const cells_temp_t * temp_get_values(void) {
    return &htemp.temperatures;
}

const discharge_temp_t * temp_get_discharge_values(void) {
    return &htemp.discharge_temperatures;
}

TempReturnCode temp_dump_values(
    celsius_t * const out,
    const size_t start,
    const size_t size)
{
    if (out == NULL)
        return TEMP_NULL_POINTER;
    if (start >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT ||
        start + size >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(out, htemp.temperatures + start, size * sizeof(raw_temp_t));
    return TEMP_OK;
}

bms_cellboard_cells_temperature_converted_t * temp_get_cells_temp_canlib_payload(size_t * const byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(htemp.temp_can_payload);

    htemp.temp_can_payload.offset = htemp.offset;
    htemp.temp_can_payload.temperature_0 = htemp.temperatures[htemp.offset];
    htemp.temp_can_payload.temperature_1 = htemp.temperatures[htemp.offset + 1U];
    htemp.temp_can_payload.temperature_2 = htemp.temperatures[htemp.offset + 2U];
    htemp.temp_can_payload.temperature_3 = htemp.temperatures[htemp.offset + 3U];

    htemp.offset += 4U;
    if (htemp.offset >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        htemp.offset = 0U;

    return &htemp.temp_can_payload;
}

#ifdef CONF_TEMPEATURE_STRINGS_ENABLE

_STATIC char * temp_module_name = "temperature";

_STATIC char * temp_return_code_name[] = {
    [TEMP_OK] = "ok",
    [TEMP_NULL_POINTER] = "null pointer",
    [TEMP_BUSY] = "busy",
    [TEMP_OUT_OF_BOUNDS] = "out of bounds"
};

_STATIC char * temp_return_code_description[] = {
    [TEMP_OK] = "executed successfully",
    [TEMP_NULL_POINTER] = "attempt to dereference a null pointer",
    [TEMP_BUSY] = "the temperature module is busy"
    [TEMP_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_TEMPEATURE_STRINGS_ENABLE

#endif // CONF_TEMPERATURE_MODULE_ENABLE
