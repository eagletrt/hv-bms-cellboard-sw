/**
 * @file temp.c
 * @date 2024-04-19
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Temperature measurment and control
 */

#include "temp-api.h"

#include <string.h>

#include "cellboard-def.h"
#include "identity-api.h"
#include "error.h"
#include "timebase.h"
#include "eagletrt-api.h"

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

// TODO: Send discharge temperatures
EAGLETRT_STATIC struct TempHandler temp_handler;

//
celsius prv_temp_volt_to_celsius(volt value) {

    // To please clang-tidy, really don't like this
    const volt temp_min_limit_v = TEMP_MIN_LIMIT_V;
    const volt temp_max_limit_v = TEMP_MAX_LIMIT_V;

    // Value is converted in V and limited to fit the polynomial range
    value = EAGLETRT_API_CLAMP(value, temp_min_limit_v, temp_max_limit_v);
    const double val = value;
    const double val2 = val * val;
    const double val3 = val2 * val;
    const double val4 = val2 * val2;
    const double val5 = val4 * val;
    const double val6 = val3 * val3;
    return TEMP_COEFF_0 +
           TEMP_COEFF_1 * val +
           TEMP_COEFF_2 * val2 +
           TEMP_COEFF_3 * val3 +
           TEMP_COEFF_4 * val4 +
           TEMP_COEFF_5 * val5 +
           TEMP_COEFF_6 * val6;
}

celsius prv_temp_discharge_volt_to_celsius(volt value) {

    const volt temp_discharge_min_limit_v = TEMP_DISCHARGE_MIN_LIMIT_V;
    const volt temp_discharge_max_limit_v = TEMP_DISCHARGE_MAX_LIMIT_V;

    // Value is converted in V and limited to fit the polynomial range
    value = EAGLETRT_API_CLAMP(value, temp_discharge_min_limit_v, temp_discharge_max_limit_v);
    const double val = value;
    const double val2 = val * val;
    const double val3 = val2 * val;
    const double val4 = val2 * val2;
    const double val5 = val4 * val;
    // const double v6 = v3 * v3;
    return TEMP_DISCHARGE_COEFF_0 +
           TEMP_DISCHARGE_COEFF_1 * val +
           TEMP_DISCHARGE_COEFF_2 * val2 +
           TEMP_DISCHARGE_COEFF_3 * val3 +
           TEMP_DISCHARGE_COEFF_4 * val4 +
           TEMP_DISCHARGE_COEFF_5 * val5;
    // TEMP_DISCHARGE_COEFF_6 * v6;
}

EAGLETRT_STATIC_INLINE void prv_temp_check_cells_value(const uint16_t index, const celsius value) { //NOLINT(bugprone-easily-swappable-parameters)
    // BUG: Ignore under temp caused by broken NTCs
    // if (value < TEMP_MIN_C)
    //     error_set(ERROR_GROUP_UNDER_TEMPERATURE_CELLS, index);
    // else
    //     error_reset(ERROR_GROUP_UNDER_TEMPERATURE_CELLS, index);
    if (value > TEMP_MAX_C) {
        error_set(ERROR_GROUP_OVER_TEMPERATURE_CELLS, index);
    } else {
        error_reset(ERROR_GROUP_OVER_TEMPERATURE_CELLS, index);
    }
}

enum TempReturnCode temp_init(const temp_set_mux_address_callback set_address, const temp_start_conversion_callback start_conversion) {
    if (set_address == NULL || start_conversion == NULL) {
        return TEMP_RC_NULL_POINTER;
    }
    memset(&temp_handler, 0U, sizeof(temp_handler));

    // Copy callback pointers
    temp_handler.set_address = set_address;
    temp_handler.start_conversion = start_conversion;
    temp_handler.temp_can_payload.cellboard_id = (bms_cellboard_cells_temperature_cellboard_id)identity_api_get_cellboard_id();
    return TEMP_RC_OK;
}

enum TempReturnCode temp_start_conversion(void) {
    if (temp_handler.busy) {
        return TEMP_RC_BUSY;
    }
    // Set busy flag
    temp_handler.busy = true;

    // Set mux address and start conversion
    if (++temp_handler.address >= CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT) {
        temp_handler.address = 0U;
    }
    temp_handler.set_address(temp_handler.address);
    temp_handler.start_conversion();
    return TEMP_RC_OK;
}

enum TempReturnCode temp_notify_conversion_complete(const volt *const values, size_t size) {
    const size_t index = temp_handler.address * CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT;
    // Convert the raw value to celsius
    for (size_t i = 0U; i < size; ++i) {
        const celsius temp = prv_temp_volt_to_celsius(values[i]);
        temp_update_value(index + i, temp);
    }
    temp_handler.busy = false;
    return TEMP_RC_OK;
}

enum TempReturnCode temp_update_value(const size_t index, const celsius value) {
    if (index >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    temp_handler.temperatures[index] = value;
    prv_temp_check_cells_value(index, value);
    return TEMP_RC_OK;
}

enum TempReturnCode temp_update_values(
    const size_t index,
    const celsius *const values,
    const size_t size) {
    if (index + size > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    for (size_t i = 0U; i < size; ++i) {
        temp_handler.temperatures[index + i] = values[i];
        prv_temp_check_cells_value(index + i, values[i]);
    }
    return TEMP_RC_OK;
}

enum TempReturnCode temp_update_discharge_value(const size_t index, const volt value) {
    if (index >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    temp_handler.discharge_temperatures[index] = prv_temp_discharge_volt_to_celsius(value);
    return TEMP_RC_OK;
}

enum TempReturnCode temp_update_discharge_values(
    const size_t index,
    const volt *const values,
    const size_t size) {
    if (index + size >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    for (size_t i = 0U; i < size; ++i) {
        temp_handler.discharge_temperatures[index + i] = prv_temp_discharge_volt_to_celsius(values[i]);
    }
    return TEMP_RC_OK;
}

const cells_temp *temp_get_values(void) {
    return &temp_handler.temperatures;
}

celsius temp_get_min(void) {
    celsius min = temp_handler.temperatures[0U];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        min = EAGLETRT_API_MIN(min, temp_handler.temperatures[i]);
    }
    return min;
}

celsius temp_get_max(void) {
    celsius max = temp_handler.temperatures[0U];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        max = EAGLETRT_API_MAX(max, temp_handler.temperatures[i]);
    }
    return max;
}

celsius temp_get_avg(void) {
    return temp_get_sum() / CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT;
}

celsius temp_get_sum(void) {
    celsius sum = 0U;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        sum += temp_handler.temperatures[i];
    }
    return sum;
}

const discharge_temp *temp_get_discharge_values(void) {
    return &temp_handler.discharge_temperatures;
}

enum TempReturnCode temp_dump_values(
    celsius *const out,
    const size_t start,
    const size_t size) {
    if (out == NULL) {
        return TEMP_RC_NULL_POINTER;
    }
    if (start >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT ||
        start + size >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    memcpy(out, temp_handler.temperatures + start, size * sizeof(*out));
    return TEMP_RC_OK;
}

bms_cellboard_cells_temperature_converted_t *temp_get_cells_temp_canlib_payload(size_t *const byte_size) {
    if (byte_size != NULL) {
        *byte_size = sizeof(temp_handler.temp_can_payload);
    }

    temp_handler.temp_can_payload.offset = temp_handler.offset;
    temp_handler.temp_can_payload.temperature_0 = temp_handler.temperatures[temp_handler.offset];
    temp_handler.temp_can_payload.temperature_1 = temp_handler.temperatures[temp_handler.offset + 1U];
    temp_handler.temp_can_payload.temperature_2 = temp_handler.temperatures[temp_handler.offset + 2U];
    temp_handler.temp_can_payload.temperature_3 = temp_handler.temperatures[temp_handler.offset + 3U];

    temp_handler.offset += 4U;
    if (temp_handler.offset >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        temp_handler.offset = 0U;
    }
    return &temp_handler.temp_can_payload;
}

bms_cellboard_discharge_temperature_converted_t *temp_get_discharge_temp_canlib_payload(size_t *const byte_size) {
    if (byte_size != NULL) {
        *byte_size = sizeof(temp_handler.discharge_temp_can_payload);
    }

    temp_handler.discharge_temp_can_payload.temperature_0 = temp_handler.discharge_temperatures[0U];
    temp_handler.discharge_temp_can_payload.temperature_1 = temp_handler.discharge_temperatures[1U];
    temp_handler.discharge_temp_can_payload.temperature_2 = temp_handler.discharge_temperatures[2U];
    temp_handler.discharge_temp_can_payload.temperature_3 = temp_handler.discharge_temperatures[3U];
    temp_handler.discharge_temp_can_payload.temperature_4 = temp_handler.discharge_temperatures[4U];
    return &temp_handler.discharge_temp_can_payload;
}

#ifdef CONF_TEMPEATURE_STRINGS_ENABLE

EAGLETRT_STATIC char *temp_module_name = "temperature";

EAGLETRT_STATIC char *temp_return_code_name[] = {
    [TEMP_RC_OK] = "ok",
    [TEMP_RC_NULL_POINTER] = "null pointer",
    [TEMP_RC_BUSY] = "busy",
    [TEMP_RC_OUT_OF_BOUNDS] = "out of bounds"
};

EAGLETRT_STATIC char *temp_return_code_description[] = {
    [TEMP_RC_OK] = "executed successfully",
    [TEMP_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [TEMP_RC_BUSY] = "the temperature module is busy",
    [TEMP_RC_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_TEMPEATURE_STRINGS_ENABLE

#endif // CONF_TEMPERATURE_MODULE_ENABLE
