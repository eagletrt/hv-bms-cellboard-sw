/**
 * @file volt.c
 * @date 2024-04-20
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Voltage measurment and control
 */

#include "volt.h"

#include <string.h>

#include "cellboard-def.h"
#include "identity.h"
#include "timebase.h"
#include "error.h"

#ifdef CONF_VOLTAGE_MODULE_ENABLE

_STATIC _VoltHandler hvolt;

/**
 * @brief Check if the voltage values are in range otherwise set an error
 *
 * @param value The cell voltage value in V
 */
_STATIC_INLINE void _volt_check_value(const size_t index, const volt_t value) {

    // BUG: Ignore under and over voltages for cells 19 and 20 for the moment
    if (index == 20 || index == 19)
        return;

    if (value <= VOLT_MIN_V)
        error_set(ERROR_GROUP_UNDER_VOLTAGE, index);
    else
        error_reset(ERROR_GROUP_UNDER_VOLTAGE, index);

    if (value >= VOLT_MAX_V)
        error_set(ERROR_GROUP_OVER_VOLTAGE, index);
    else
        error_reset(ERROR_GROUP_OVER_VOLTAGE, index);
}

VoltReturnCode volt_init(void) {
    memset(&hvolt, 0U, sizeof(hvolt));
    hvolt.voltages_can_payload.cellboard_id = (bms_cellboard_cells_voltage_cellboard_id)identity_get_cellboard_id();
    return VOLT_OK;
}

VoltReturnCode volt_update_value(const size_t index, const volt_t value) {
    if (index > CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_OUT_OF_BOUNDS;
    hvolt.voltages[index] = value;
    _volt_check_value(index, value);
    return VOLT_OK;
}

VoltReturnCode volt_update_values(const size_t index, const volt_t * const values, const size_t size) {
    if (index + size > CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_OUT_OF_BOUNDS;
    for (size_t i = 0U; i < size; ++i) {
        hvolt.voltages[index + i] = values[i];
        _volt_check_value(index + i, hvolt.voltages[index + i]);
    }
    return VOLT_OK;
}

const cells_volt_t * volt_get_values(void) {
    return (const cells_volt_t *)&hvolt.voltages;
}

bit_flag32_t volt_select_values(const volt_t target) {
    bit_flag32_t bits = 0U;
    CELLBOARD_ASSERT(CELLBOARD_SEGMENT_SERIES_COUNT > sizeof(bits) * 8U);

    // Iterate over cells and choose the one which voltage is greater than the target
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        if (hvolt.voltages[i] > target)
            bits = CELLBOARD_BIT_SET(bits, i);
    }
    return bits;
}

VoltReturnCode volt_dump_values(
    volt_t * const out,
    const size_t start,
    const size_t size)
{
    if (out == NULL)
        return VOLT_NULL_POINTER;
    if (start >= CELLBOARD_SEGMENT_SERIES_COUNT || start + size >= CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_OUT_OF_BOUNDS;
    memcpy(out, hvolt.voltages + start, size * sizeof(hvolt.voltages[0U]));
    return VOLT_OK;
}

bms_cellboard_cells_voltage_converted_t * volt_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hvolt.voltages_can_payload);

    _STATIC size_t offset = 0U;
    hvolt.voltages_can_payload.offset = offset;
    hvolt.voltages_can_payload.voltage_0 = hvolt.voltages[offset];
    hvolt.voltages_can_payload.voltage_1 = hvolt.voltages[offset + 1U];
    hvolt.voltages_can_payload.voltage_2 = hvolt.voltages[offset + 2U];

    offset += 3U;
    if (offset >= CELLBOARD_SEGMENT_SERIES_COUNT)
        offset = 0U;
    return &hvolt.voltages_can_payload;
}

#ifdef CONF_VOLTAGE_STRINGS_ENABLE

_STATIC char * volt_module_name = "voltage";

sttic char * volt_return_code_name[] = {
    [VOLT_OK] = "ok",
    [VOLT_NULL_POINTER] = "null pointer",
    [VOLT_OUT_OF_BOUNDS] = "out of bounds"
};

_STATIC char * volt_return_code_description[] = {
    [VOLT_OK] = "executed successfully",
    [VOLT_NULL_POINTER] = "attempt to dereference a null pointer"
    [VOLT_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_VOLTAGE_STRINGS_ENABLE

#endif // CONF_VOLTAGE_MODULE_ENABLE
