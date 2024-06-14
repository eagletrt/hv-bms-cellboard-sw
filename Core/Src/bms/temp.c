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

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

// TODO: Send discharge temperatures
static struct {
    raw_temp_t temperatures[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT];
    raw_temp_t discharge_temperatures[CELLBOARD_SEGMENT_DISCHARGE_TEMP_SENSOR_COUNT];

    bms_cell_temperatures_converted_t can_payload;
} htemp;

inline _temp_check_values(raw_temp_t value) {
    ERROR_TOGGLE_IF(value <= TEMP_CELSIUS_TO_VALUE(TEMP_), ERROR_GROUP_UNDER_TEMPERATURE, 0U, timebase_get_time());
    ERROR_TOGGLE_IF(value <= TEMP_VOL, ERROR_GROUP_OVER_TEMPERATURE, 0U, timebase_get_time());
}

TempReturnCode temp_init(void) {
    memset(&htemp, 0U, sizeof(htemp));
    htemp.can_payload.cellboard_id = identity_get_cellboard_id();
    return TEMP_OK;
}

TempReturnCode temp_update_value(size_t index, raw_temp_t value) {
    if (index > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.temperatures[index] = value;
    return TEMP_OK;
}

TempReturnCode temp_update_values(size_t index, raw_temp_t * values, size_t size) {
    if (index + size >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(htemp.temperatures, values + index, size * sizeof(htemp.temperatures[0]));
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_value(size_t index, raw_temp_t value) {
    if (index > CELLBOARD_SEGMENT_DISCHARGE_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    htemp.discharge_temperatures[index] = value;
    return TEMP_OK;
}

TempReturnCode temp_update_discharge_values(size_t index, raw_temp_t * values, size_t size) {
    if (index + size >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(htemp.discharge_temperatures, values + index, size * sizeof(htemp.discharge_temperatures[0]));
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

// TODO: Macro to convert raw values to °C
bms_cell_temperatures_converted_t * temp_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(htemp.can_payload);

    static size_t offset = 0U;
    htemp.can_payload.offset = offset;
    htemp.can_payload.temperature0 = TEMP_VALUE_TO_CELSIUS(htemp.temperatures[offset]);
    htemp.can_payload.temperature1 = TEMP_VALUE_TO_CELSIUS(htemp.temperatures[++offset]);
    htemp.can_payload.temperature2 = TEMP_VALUE_TO_CELSIUS(htemp.temperatures[++offset]);

    if (offset >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        offset = 0U;

    return &htemp.can_payload;
}

#ifdef CONF_TEMPEATURE_STRINGS_ENABLE

static char * temp_module_name = "temperature";

static char * temp_return_code_name[] = {
    [TEMP_OK] = "ok",
    [TEMP_NULL_POINTER] = "null pointer",
    [TEMP_OUT_OF_BOUNDS] = "out of bounds"
};

static char * temp_return_code_description[] = {
    [TEMP_OK] = "executed successfully",
    [TEMP_NULL_POINTER] = "attempt to dereference a null pointer",
    [TEMP_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_TEMPEATURE_STRINGS_ENABLE

#endif // CONF_TEMPERATURE_MODULE_ENABLE
