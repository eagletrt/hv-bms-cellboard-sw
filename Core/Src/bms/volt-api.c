/*!
 * \file volt-api.c
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
#include "error-api.h"

#include "can-bms.h"
#include "eagletrt-api.h"
#include "usart.h"
#include "volt.h"

#ifdef CONF_VOLTAGE_MODULE_ENABLE

EAGLETRT_STATIC struct VoltHandler volt_handler;

/*!
 * \brief Check if the voltage values are in range otherwise set an error
 *
 * \param index The index of the cell voltage to check
 * \param value The cell voltage value in V
 */

EAGLETRT_STATIC_INLINE void prv_volt_api_check_value(const uint16_t index, const volt value) {
    if (value < VOLT_MIN_V) {
        error_api_set(ERROR_GROUP_UNDER_VOLTAGE, index);
    } else {
        error_api_reset(ERROR_GROUP_UNDER_VOLTAGE, index);
    }

    if (value > VOLT_MAX_V) {
        error_api_set(ERROR_GROUP_OVER_VOLTAGE, index);
    } else {
        error_api_reset(ERROR_GROUP_OVER_VOLTAGE, index);
    }
}

enum VoltReturnCode volt_api_init(void) {
    memset(&volt_handler, 0U, sizeof(volt_handler));
    return VOLT_RC_OK;
}

enum VoltReturnCode volt_api_update_value(const size_t index, const volt value) {
    if (index > CELLBOARD_SEGMENT_SERIES_COUNT) {
        return VOLT_RC_OUT_OF_BOUNDS;
    }
    volt_handler.voltages[index] = value;
    prv_volt_api_check_value(index, value);
    return VOLT_RC_OK;
}

enum VoltReturnCode volt_api_update_values(const size_t index, const volt *const values, const size_t size) {
    if (values == NULL) {
        return VOLT_RC_NULL_POINTER;
    }
    if (index + size > CELLBOARD_SEGMENT_SERIES_COUNT) {
        return VOLT_RC_OUT_OF_BOUNDS;
    }
    for (size_t i = 0U; i < size; ++i) {
        volt_handler.voltages[index + i] = values[i];
        prv_volt_api_check_value(index + i, volt_handler.voltages[index + i]);
    }
    return VOLT_RC_OK;
}

const cells_volt *volt_api_get_values(void) {
    return (const cells_volt *)&volt_handler.voltages;
}

volt volt_api_get_min(void) {
    volt min = volt_handler.voltages[0];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        min = EAGLETRT_API_MIN(min, volt_handler.voltages[i]);
    }
    return min;
}

volt volt_api_get_max(void) {
    volt max = volt_handler.voltages[0];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        max = EAGLETRT_API_MAX(max, volt_handler.voltages[i]);
    }
    return max;
}

volt volt_api_get_avg(void) {
    return volt_api_get_sum() / CELLBOARD_SEGMENT_SERIES_COUNT;
}

volt volt_api_get_sum(void) {
    volt sum = 0U;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        sum += volt_handler.voltages[i];
    }
    return sum;
}

bit_flag32 volt_api_select_values_above_target(const volt target) {
    bit_flag32 bits = 0U;
    CELLBOARD_ASSERT((uint32_t)CELLBOARD_SEGMENT_SERIES_COUNT > sizeof(bits) * 8U);

    // Iterate over cells and choose the one which voltage is greater than the target
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
        if (volt_handler.voltages[i] > target) {
            bits = EAGLETRT_API_BIT_SET(bits, i);
        }
    }
    return bits;
}

enum VoltReturnCode volt_api_dump_values(
    volt *const out,
    const size_t start,
    const size_t size) {
    if (out == NULL) {
        return VOLT_RC_NULL_POINTER;
    }
    if (start >= CELLBOARD_SEGMENT_SERIES_COUNT || start + size >= CELLBOARD_SEGMENT_SERIES_COUNT) {
        return VOLT_RC_OUT_OF_BOUNDS;
    }
    memcpy(out, volt_handler.voltages + start, size * sizeof(volt_handler.voltages[0U]));
    return VOLT_RC_OK;
}

union CanBmsMessages *volt_api_get_voltage_info_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1voltageinfo,
        can_bms_byte_size_tsaccellboard2voltageinfo,
        can_bms_byte_size_tsaccellboard3voltageinfo,
        can_bms_byte_size_tsaccellboard4voltageinfo,
        can_bms_byte_size_tsaccellboard5voltageinfo,
        can_bms_byte_size_tsaccellboard6voltageinfo
    };
    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    union CanBmsMessages *payload = &volt_handler.libcan_message_voltage_info;
    switch (cellboard) {
        case CELLBOARD_ID_0:
            payload->tsaccellboard1voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard1voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard1voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard1voltageinfo.max = volt_api_get_max();
            break;
        case CELLBOARD_ID_1:
            payload->tsaccellboard2voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard2voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard2voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard2voltageinfo.max = volt_api_get_max();
            break;
        case CELLBOARD_ID_2:
            payload->tsaccellboard3voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard3voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard3voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard3voltageinfo.max = volt_api_get_max();
            break;
        case CELLBOARD_ID_3:
            payload->tsaccellboard4voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard4voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard4voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard4voltageinfo.max = volt_api_get_max();
            break;
        case CELLBOARD_ID_4:
            payload->tsaccellboard5voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard5voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard5voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard5voltageinfo.max = volt_api_get_max();
            break;
        case CELLBOARD_ID_5:
            payload->tsaccellboard6voltageinfo.total = volt_api_get_sum();
            payload->tsaccellboard6voltageinfo.average = volt_api_get_avg();
            payload->tsaccellboard6voltageinfo.min = volt_api_get_min();
            payload->tsaccellboard6voltageinfo.max = volt_api_get_max();
            break;
        default:
            break;
    }
    return payload;
}

union CanBmsMessages *volt_api_get_voltage_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1voltage,
        can_bms_byte_size_tsaccellboard2voltage,
        can_bms_byte_size_tsaccellboard3voltage,
        can_bms_byte_size_tsaccellboard4voltage,
        can_bms_byte_size_tsaccellboard5voltage,
        can_bms_byte_size_tsaccellboard6voltage
    };
    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    const cells_volt *voltage = volt_api_get_values();
    union CanBmsMessages *message = &volt_handler.libcan_message_voltage;
    switch (cellboard) {
        case CELLBOARD_ID_0: {
            struct CanBmsTsaccellboard1voltage *payload = &message->tsaccellboard1voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_1: {
            struct CanBmsTsaccellboard2voltage *payload = &message->tsaccellboard2voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_2: {
            struct CanBmsTsaccellboard3voltage *payload = &message->tsaccellboard3voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_3: {
            struct CanBmsTsaccellboard4voltage *payload = &message->tsaccellboard4voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_4: {
            struct CanBmsTsaccellboard5voltage *payload = &message->tsaccellboard5voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_5: {
            struct CanBmsTsaccellboard6voltage *payload = &message->tsaccellboard6voltage;
            payload->group = (payload->group >= 3) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*voltage)[0];
                    payload->group_payload.mux_0.cell2 = (*voltage)[1];
                    payload->group_payload.mux_0.cell3 = (*voltage)[2];
                    payload->group_payload.mux_0.cell4 = (*voltage)[3];
                    payload->group_payload.mux_0.cell5 = (*voltage)[4];
                    payload->group_payload.mux_0.cell6 = (*voltage)[5];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell7 = (*voltage)[6];
                    payload->group_payload.mux_1.cell8 = (*voltage)[7];
                    payload->group_payload.mux_1.cell9 = (*voltage)[8];
                    payload->group_payload.mux_1.cell10 = (*voltage)[9];
                    payload->group_payload.mux_1.cell11 = (*voltage)[10];
                    payload->group_payload.mux_1.cell12 = (*voltage)[11];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell13 = (*voltage)[12];
                    payload->group_payload.mux_2.cell14 = (*voltage)[13];
                    payload->group_payload.mux_2.cell15 = (*voltage)[14];
                    payload->group_payload.mux_2.cell16 = (*voltage)[15];
                    payload->group_payload.mux_2.cell17 = (*voltage)[16];
                    payload->group_payload.mux_2.cell18 = (*voltage)[17];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell19 = (*voltage)[18];
                    payload->group_payload.mux_3.cell20 = (*voltage)[19];
                    payload->group_payload.mux_3.cell21 = (*voltage)[20];
                    payload->group_payload.mux_3.cell22 = (*voltage)[21];
                    payload->group_payload.mux_3.cell23 = (*voltage)[22];
                    payload->group_payload.mux_3.cell24 = (*voltage)[23];
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return message;
}

#ifdef CONF_VOLTAGE_STRINGS_ENABLE

EAGLETRT_STATIC char *volt_module_name = "voltage";

EAGLETRT_STATIC char *volt_return_code_name[] = {
    [VOLT_RC_OK] = "ok",
    [VOLT_RC_NULL_POINTER] = "null pointer",
    [VOLT_RC_OUT_OF_BOUNDS] = "out of bounds"
};

EAGLETRT_STATIC char *volt_return_code_description[] = {
    [VOLT_RC_OK] = "executed successfully",
    [VOLT_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [VOLT_RC_OUT_OF_BOUNDS] = "attempt to access an invalid memory region"
};

#endif // CONF_VOLTAGE_STRINGS_ENABLE

#endif // CONF_VOLTAGE_MODULE_ENABLE
