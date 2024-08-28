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
 * @brief Check if the cells temperature values are in range otherwise set an error
 *
 * @param value The raw temperature value to check
 */
_STATIC_INLINE void _temp_check_cells_value(raw_temp_t value) {
    // TODO: Set temp errors
    // ERROR_TOGGLE_IF(
    //     value <= TEMP_MIN_VALUE,
    //     ERROR_GROUP_UNDER_TEMPERATURE,
    //     ERROR_UNDER_TEMPERATURE_INSTANCE_CELLS,
    //     timebase_get_time()
    // );
    // ERROR_TOGGLE_IF(
    //     value >= TEMP_MAX_VALUE, 
    //     ERROR_GROUP_OVER_TEMPERATURE,
    //     ERROR_OVER_TEMPERATURE_INSTANCE_CELLS, 
    //     timebase_get_time()
    // );
}

/**
 * @brief Check if the discharge temperature values are in range otherwise set an error
 *
 * @param value The raw temperature value to check
 */
_STATIC_INLINE void _temp_check_discharge_value(raw_temp_t value) {
    // TODO: Set temp errors
    // ERROR_TOGGLE_IF(
    //     value <= TEMP_MIN_VALUE,
    //     ERROR_GROUP_UNDER_TEMPERATURE,
    //     ERROR_UNDER_TEMPERATURE_INSTANCE_DISCHARGE,
    //     timebase_get_time()
    // );
    // ERROR_TOGGLE_IF(
    //     value >= TEMP_MAX_VALUE,
    //     ERROR_GROUP_OVER_TEMPERATURE,
    //     ERROR_OVER_TEMPERATURE_INSTANCE_DISCHARGE,
    //     timebase_get_time()
    // );
}

TempReturnCode temp_init(
    temp_set_mux_address_callback_t set_address,
    temp_start_conversion_callback_t start_conversion)
{
    if (set_address == NULL || start_conversion == NULL)
        return TEMP_NULL_POINTER;
    memset(&htemp, 0U, sizeof(htemp));

    // Copy callback pointers
    htemp.set_address = set_address;
    htemp.start_conversion = start_conversion;
    htemp.can_payload.cellboard_id = identity_get_cellboard_id();
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

celsius_t temp_volt_to_celsius(volt_t value) {
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

TempReturnCode temp_notify_conversion_complete(raw_temp_t * values, size_t size) {
    htemp.busy = false;
    size_t index = htemp.address * CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT;
    return temp_update_values(index, values, size);
}

TempReturnCode temp_update_value(size_t index, raw_temp_t value) {
    if (index > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.temperatures[index] = value;
    _temp_check_cells_value(value);
    return TEMP_OK;
}

TempReturnCode temp_update_values(size_t index, raw_temp_t * values, size_t size) {
    if (index + size > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(htemp.temperatures + index, values, size * sizeof(htemp.temperatures[0]));
    for (size_t i = 0U; i < size; ++i)
        _temp_check_cells_value(htemp.temperatures[index + i]);
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_value(size_t index, raw_temp_t value) {
    if (index > CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.discharge_temperatures[index] = value;
    _temp_check_discharge_value(value);
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_values(size_t index, raw_temp_t * values, size_t size) {
    if (index + size >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(htemp.discharge_temperatures + index, values, size * sizeof(htemp.discharge_temperatures[0]));
    for (size_t i = 0U; i < size; ++i)
        _temp_check_discharge_value(htemp.discharge_temperatures[index + i]);
    return TEMP_OK;
}

const raw_temp_t * temp_get_values(void) {
    return htemp.temperatures;
}

const raw_temp_t * temp_get_discharge_values(void) {
    return htemp.discharge_temperatures;
}

TempReturnCode temp_dump_values(raw_temp_t * out, size_t start, size_t size) {
    if (out == NULL)
        return TEMP_NULL_POINTER;
    if (start >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT || start + size >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(out, htemp.temperatures + start, size * sizeof(raw_temp_t));
    return TEMP_OK;
}

bms_cellboard_cells_temperature_converted_t * temp_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(htemp.can_payload);

    htemp.can_payload.offset = htemp.offset;
    htemp.can_payload.temperature_0 = htemp.temperatures[htemp.offset];
    htemp.can_payload.temperature_1 = htemp.temperatures[htemp.offset + 1U];
    htemp.can_payload.temperature_2 = htemp.temperatures[htemp.offset + 2U];
    htemp.can_payload.temperature_3 = htemp.temperatures[htemp.offset + 3U];

    htemp.offset += 4U;
    if (htemp.offset >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        htemp.offset = 0U;

    return &htemp.can_payload;
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
