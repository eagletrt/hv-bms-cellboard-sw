/**
 * @file temp.c
 * @date 2024-04-19
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Temperature measurment and control
 */

#include "temp.h"

#include <string.h>

#include "identity.h"

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

raw_temp temperatures[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT];
bms_cell_temperatures_converted_t can_payload;


TempReturnCode temp_init(void) {
    can_payload.cellboard_id = identity_get_cellboard_id();
    return TEMP_OK;
}

TempReturnCode temp_update_value(size_t index, raw_temp value) {
    if (index > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    temperatures[index] = value;
    return TEMP_OK;
}

const raw_temp * temp_get_values(void) {
    return temperatures;
}

TempReturnCode temp_dump_values(raw_temp * out, size_t start, size_t size) {
    if (out == NULL)
        return TEMP_NULL_POINTER;
    if (start >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT || start + size >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        return TEMP_OUT_OF_BOUNDS;
    memcpy(out, temperatures + start, size * sizeof(raw_temp));
    return TEMP_OK;
}

// TODO: Macro to convert raw values to °C
bms_cell_temperatures_converted_t * temp_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(can_payload);

    static size_t offset = 0;
    can_payload.offset = offset;
    can_payload.temperature0 = TEMP_VALUE_TO_CELSIUS(temperatures[offset]);
    can_payload.temperature1 = TEMP_VALUE_TO_CELSIUS(temperatures[++offset]);
    can_payload.temperature2 = TEMP_VALUE_TO_CELSIUS(temperatures[++offset]);

    if (offset >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT)
        offset = 0;

    return &can_payload;
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
    [TEMP_NULL_POINTER] = "attempt to dereference a NULL pointer",
    [TEMP_OUT_OF_BOUNDS] = "attempt to access an invalid memory region""
};

#endif // CONF_TEMPEATURE_STRINGS_ENABLE

#endif // CONF_TEMPERATURE_MODULE_ENABLE
