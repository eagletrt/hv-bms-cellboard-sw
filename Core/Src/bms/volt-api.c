/*!
 * \file volt.c
 * \date 2024-04-20
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Voltage measurment and control
 */

#include "volt-api.h"

#include <string.h>

#include "cellboard-def.h"
#include "identity-api.h"
#include "timebase.h"
#include "error.h"

#ifdef CONF_VOLTAGE_MODULE_ENABLE

_STATIC struct VoltHandler volt_handler;

/**
 * \brief Check if the voltage values are in range otherwise set an error
 *
 * \param value The cell voltage value in V
 */
_STATIC_INLINE void prv_volt_check_value(const size_t index, const volt value) {
    // BUG: Ignore broken voltage readings (only for third cellboard)
    // if (index == 19 || index == 20)
    //     return;

    if (value < VOLT_MIN_V)
        error_set(ERROR_GROUP_UNDER_VOLTAGE, index);
    else
        error_reset(ERROR_GROUP_UNDER_VOLTAGE, index);

    if (value > VOLT_MAX_V)
        error_set(ERROR_GROUP_OVER_VOLTAGE, index);
    else
        error_reset(ERROR_GROUP_OVER_VOLTAGE, index);
}

enum VoltReturnCode volt_init(void) {
    memset(&volt_handler, 0U, sizeof(volt_handler));
    volt_handler.voltages_can_payload.cellboard_id = (bms_cellboard_cells_voltage_cellboard_id)identity_api_get_cellboard_id();
    return VOLT_RC_OK;
}

enum VoltReturnCode volt_update_value(const size_t index, const volt value) {
    if (index > CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_RC_OUT_OF_BOUNDS;
    volt_handler.voltages[index] = value;
    prv_volt_check_value(index, value);
    return VOLT_RC_OK;
}

enum VoltReturnCode volt_update_values(const size_t index, const volt *const values, const size_t size) {
    if (index + size > CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_RC_OUT_OF_BOUNDS;
    for (size_t i = 0U; i < size; ++i) {
        volt_handler.voltages[index + i] = values[i];
        prv_volt_check_value(index + i, volt_handler.voltages[index + i]);
    }
    return VOLT_RC_OK;
}

const cells_volt *volt_get_values(void) {
    return (const cells_volt *)&volt_handler.voltages;
}

volt volt_get_min(void) {
    volt min = VOLT_MAX_V;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        min = CELLBOARD_MIN(min, volt_handler.voltages[i]);
    }
    return min;
}

volt volt_get_max(void) {
    volt max = 0U;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        max = CELLBOARD_MAX(max, volt_handler.voltages[i]);
    }
    return max;
}

volt volt_get_avg(void) {
    return volt_get_sum() / CELLBOARD_SEGMENT_SERIES_COUNT;
}

volt volt_get_sum(void) {
    volt sum = 0U;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        sum += volt_handler.voltages[i];
    }
    return sum;
}

bit_flag32 volt_select_values_strictly_greater(const volt target) {
    bit_flag32 bits = 0U;
    CELLBOARD_ASSERT(CELLBOARD_SEGMENT_SERIES_COUNT > sizeof(bits) * 8U);

    // Iterate over cells and choose the one which voltage is greater than the target
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        if (volt_handler.voltages[i] > target)
            bits = CELLBOARD_BIT_SET(bits, i);
    }
    return bits;
}

enum VoltReturnCode volt_dump_values(
    volt *const out,
    const size_t start,
    const size_t size) {
    if (out == NULL)
        return VOLT_RC_NULL_POINTER;
    if (start >= CELLBOARD_SEGMENT_SERIES_COUNT || start + size >= CELLBOARD_SEGMENT_SERIES_COUNT)
        return VOLT_RC_OUT_OF_BOUNDS;
    memcpy(out, volt_handler.voltages + start, size * sizeof(volt_handler.voltages[0U]));
    return VOLT_RC_OK;
}

bms_cellboard_cells_voltage_converted *volt_get_canlib_payload(size_t *byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(volt_handler.voltages_can_payload);

    _STATIC size_t offset = 0U;
    volt_handler.voltages_can_payload.offset = offset;
    volt_handler.voltages_can_payload.voltage_0 = volt_handler.voltages[offset];
    volt_handler.voltages_can_payload.voltage_1 = volt_handler.voltages[offset + 1U];
    volt_handler.voltages_can_payload.voltage_2 = volt_handler.voltages[offset + 2U];

    offset += 3U;
    if (offset >= CELLBOARD_SEGMENT_SERIES_COUNT)
        offset = 0U;
    return &volt_handler.voltages_can_payload;
}

#ifdef CONF_VOLTAGE_STRINGS_ENABLE

_STATIC char *volt_module_name = "voltage";

sttic char *volt_return_code_name[] = {
    [VOLT_RC_OK] = "ok",
    [VOLT_RC_NULL_POINTER] = "null pointer",
    [VOLT_RC_OUT_OF_BOUNDS] = "out of bounds"
};

_STATIC char *volt_return_code_description[] = {
    [VOLT_RC_OK] = "executed successfully",
    [VOLT_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [VOLT_RC_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_VOLTAGE_STRINGS_ENABLE

#endif // CONF_VOLTAGE_MODULE_ENABLE
