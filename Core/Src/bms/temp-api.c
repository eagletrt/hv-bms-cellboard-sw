/**
 * @file temp-api.c
 * @date 2024-04-19
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Temperature measurment and control
 */

#include "temp-api.h"

#include <string.h>

#include "cellboard-def.h"
#include "identity-api.h"
#include "error-api.h"
#include "eagletrt-api.h"
#include "usart.h"

#ifdef CONF_TEMPERATURE_MODULE_ENABLE

// TODO: Send discharge temperatures
EAGLETRT_STATIC struct TempHandler temp_handler;

//
celsius prv_temp_api_volt_to_celsius(volt value) {
    constexpr volt temp_min_limit = TEMP_MIN_LIMIT_V;
    constexpr volt temp_max_limit = TEMP_MAX_LIMIT_V;

    // Value is converted in V and limited to fit the polynomial range
    value = EAGLETRT_API_CLAMP(value, temp_min_limit, temp_max_limit);
    const double val = value;
    const double val2 = val * val;
    const double val3 = val2 * val;
    const double val4 = val2 * val2;
    const double val5 = val4 * val;
    const double val6 = val3 * val3;
    return TEMP_COEFF_0 +
           (TEMP_COEFF_1 * val) +
           (TEMP_COEFF_2 * val2) +
           (TEMP_COEFF_3 * val3) +
           (TEMP_COEFF_4 * val4) +
           (TEMP_COEFF_5 * val5) +
           (TEMP_COEFF_6 * val6);
}

celsius prv_temp_api_discharge_volt_to_celsius(volt value) {
    constexpr volt temp_discharge_min_limit = TEMP_DISCHARGE_MIN_LIMIT_V;
    constexpr volt temp_discharge_max_limit = TEMP_DISCHARGE_MAX_LIMIT_V;

    // Value is converted in V and limited to fit the polynomial range
    value = EAGLETRT_API_CLAMP(value, temp_discharge_min_limit, temp_discharge_max_limit);
    const double val = value;
    const double val2 = val * val;
    const double val3 = val2 * val;
    const double val4 = val2 * val2;
    const double val5 = val4 * val;
    return TEMP_DISCHARGE_COEFF_0 +
           (TEMP_DISCHARGE_COEFF_1 * val) +
           (TEMP_DISCHARGE_COEFF_2 * val2) +
           (TEMP_DISCHARGE_COEFF_3 * val3) +
           (TEMP_DISCHARGE_COEFF_4 * val4) +
           (TEMP_DISCHARGE_COEFF_5 * val5);
}

EAGLETRT_STATIC_INLINE void prv_temp_api_check_cells_value(const uint16_t index, const celsius value) {
    if (value < TEMP_MIN_C) {
        error_api_set(ERROR_GROUP_UNDER_TEMPERATURE_CELLS, index);
    } else {
        error_api_reset(ERROR_GROUP_UNDER_TEMPERATURE_CELLS, index);
    }
    if (value > TEMP_MAX_C) {
        error_api_set(ERROR_GROUP_OVER_TEMPERATURE_CELLS, index);
    } else {
        error_api_reset(ERROR_GROUP_OVER_TEMPERATURE_CELLS, index);
    }
}

enum TempReturnCode temp_api_init(const temp_set_mux_address_callback set_address, const temp_start_conversion_callback start_conversion) {
    if (set_address == NULL || start_conversion == NULL) {
        return TEMP_RC_NULL_POINTER;
    }
    memset(&temp_handler, 0U, sizeof(temp_handler));

    // Copy callback pointers
    temp_handler.set_address = set_address;
    temp_handler.start_conversion = start_conversion;
    return TEMP_RC_OK;
}

enum TempReturnCode temp_api_start_conversion(void) {
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

enum TempReturnCode temp_api_notify_conversion_complete(const volt *const values, size_t size) {
    const size_t index = temp_handler.address * CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT;
    // Convert the raw value to celsius
    for (size_t i = 0U; i < size; ++i) {
        const celsius temp = prv_temp_api_volt_to_celsius(values[i]);
        temp_api_update_value(index + i, temp);
    }
    temp_handler.busy = false;
    return TEMP_RC_OK;
}

enum TempReturnCode temp_api_update_value(const size_t index, const celsius value) {
    if (index >= CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    temp_handler.temperatures[index] = value;
    prv_temp_api_check_cells_value(index, value);
    return TEMP_RC_OK;
}

enum TempReturnCode temp_api_update_values(
    const size_t index,
    const celsius *const values,
    const size_t size) {
    if (index + size > CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    for (size_t i = 0U; i < size; ++i) {
        temp_handler.temperatures[index + i] = values[i];
        prv_temp_api_check_cells_value(index + i, values[i]);
    }
    return TEMP_RC_OK;
}

enum TempReturnCode temp_api_update_discharge_value(const size_t index, const volt value) {
    if (index >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    temp_handler.discharge_temperatures[index] = prv_temp_api_discharge_volt_to_celsius(value);
    return TEMP_RC_OK;
}

enum TempReturnCode temp_api_update_discharge_values(
    const size_t index,
    const volt *const values,
    const size_t size) {
    if (index + size >= CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT) {
        return TEMP_RC_OUT_OF_BOUNDS;
    }
    for (size_t i = 0U; i < size; ++i) {
        temp_handler.discharge_temperatures[index + i] = prv_temp_api_discharge_volt_to_celsius(values[i]);
    }
    return TEMP_RC_OK;
}

const cells_temp *temp_api_get_values(void) {
    return &temp_handler.temperatures;
}

celsius temp_api_get_min(void) {
    celsius min = temp_handler.temperatures[0U];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        min = EAGLETRT_API_MIN(min, temp_handler.temperatures[i]);
    }
    return min;
}

celsius temp_api_get_max(void) {
    celsius max = temp_handler.temperatures[0U];
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        max = EAGLETRT_API_MAX(max, temp_handler.temperatures[i]);
    }
    return max;
}

celsius temp_api_get_avg(void) {
    return temp_api_get_sum() / CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT;
}

celsius temp_api_get_sum(void) {
    celsius sum = 0U;
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT; ++i) {
        sum += temp_handler.temperatures[i];
    }
    return sum;
}

const discharge_temp *temp_api_get_discharge_values(void) {
    return &temp_handler.discharge_temperatures;
}

enum TempReturnCode temp_api_dump_values(
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

union CanBmsMessages *temp_api_get_temperature_info_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1temperatureinfo,
        can_bms_byte_size_tsaccellboard2temperatureinfo,
        can_bms_byte_size_tsaccellboard3temperatureinfo,
        can_bms_byte_size_tsaccellboard4temperatureinfo,
        can_bms_byte_size_tsaccellboard5temperatureinfo,
        can_bms_byte_size_tsaccellboard6temperatureinfo
    };
    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    union CanBmsMessages *payload = &temp_handler.libcan_message_temperature_info;
    switch (cellboard) {
        case CELLBOARD_ID_0:
            payload->tsaccellboard1temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard1temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard1temperatureinfo.max = temp_api_get_max();
            break;
        case CELLBOARD_ID_1:
            payload->tsaccellboard2temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard2temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard2temperatureinfo.max = temp_api_get_max();
            break;
        case CELLBOARD_ID_2:
            payload->tsaccellboard3temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard3temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard3temperatureinfo.max = temp_api_get_max();
            break;
        case CELLBOARD_ID_3:
            payload->tsaccellboard4temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard4temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard4temperatureinfo.max = temp_api_get_max();
            break;
        case CELLBOARD_ID_4:
            payload->tsaccellboard5temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard5temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard5temperatureinfo.max = temp_api_get_max();
            break;
        case CELLBOARD_ID_5:
            payload->tsaccellboard6temperatureinfo.average = temp_api_get_avg();
            payload->tsaccellboard6temperatureinfo.min = temp_api_get_min();
            payload->tsaccellboard6temperatureinfo.max = temp_api_get_max();
            break;
        default:
            break;
    }
    return payload;
}

union CanBmsMessages *temp_api_get_temperature_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1temperature,
        can_bms_byte_size_tsaccellboard2temperature,
        can_bms_byte_size_tsaccellboard3temperature,
        can_bms_byte_size_tsaccellboard4temperature,
        can_bms_byte_size_tsaccellboard5temperature,
        can_bms_byte_size_tsaccellboard6temperature
    };
    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    const cells_temp *temperature = temp_api_get_values();
    union CanBmsMessages *message = &temp_handler.libcan_message_temperature;
    switch (cellboard) {
        case CELLBOARD_ID_0: {
            struct CanBmsTsaccellboard1temperature *payload = &message->tsaccellboard1temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_1: {
            struct CanBmsTsaccellboard2temperature *payload = &message->tsaccellboard2temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_2: {
            struct CanBmsTsaccellboard3temperature *payload = &message->tsaccellboard3temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_3: {
            struct CanBmsTsaccellboard4temperature *payload = &message->tsaccellboard4temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_4: {
            struct CanBmsTsaccellboard5temperature *payload = &message->tsaccellboard5temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
                    break;
                default:
                    break;
            }
            break;
        }
        case CELLBOARD_ID_5: {
            struct CanBmsTsaccellboard6temperature *payload = &message->tsaccellboard6temperature;
            payload->group = (payload->group >= 9) ? 0 : payload->group + 1;
            switch (payload->group) {
                case 0:
                    payload->group_payload.mux_0.cell1 = (*temperature)[0];
                    payload->group_payload.mux_0.cell2 = (*temperature)[1];
                    payload->group_payload.mux_0.cell3 = (*temperature)[2];
                    payload->group_payload.mux_0.cell4 = (*temperature)[3];
                    payload->group_payload.mux_0.cell5 = (*temperature)[4];
                    break;
                case 1:
                    payload->group_payload.mux_1.cell6 = (*temperature)[5];
                    payload->group_payload.mux_1.cell7 = (*temperature)[6];
                    payload->group_payload.mux_1.cell8 = (*temperature)[7];
                    payload->group_payload.mux_1.cell9 = (*temperature)[8];
                    payload->group_payload.mux_1.cell10 = (*temperature)[9];
                    break;
                case 2:
                    payload->group_payload.mux_2.cell11 = (*temperature)[10];
                    payload->group_payload.mux_2.cell12 = (*temperature)[11];
                    payload->group_payload.mux_2.cell13 = (*temperature)[12];
                    payload->group_payload.mux_2.cell14 = (*temperature)[13];
                    payload->group_payload.mux_2.cell15 = (*temperature)[14];
                    break;
                case 3:
                    payload->group_payload.mux_3.cell16 = (*temperature)[15];
                    payload->group_payload.mux_3.cell17 = (*temperature)[16];
                    payload->group_payload.mux_3.cell18 = (*temperature)[17];
                    payload->group_payload.mux_3.cell19 = (*temperature)[18];
                    payload->group_payload.mux_3.cell20 = (*temperature)[19];
                    break;
                case 4:
                    payload->group_payload.mux_4.cell21 = (*temperature)[20];
                    payload->group_payload.mux_4.cell22 = (*temperature)[21];
                    payload->group_payload.mux_4.cell23 = (*temperature)[22];
                    payload->group_payload.mux_4.cell24 = (*temperature)[23];
                    payload->group_payload.mux_4.cell25 = (*temperature)[24];
                    break;
                case 5:
                    payload->group_payload.mux_5.cell26 = (*temperature)[25];
                    payload->group_payload.mux_5.cell27 = (*temperature)[26];
                    payload->group_payload.mux_5.cell28 = (*temperature)[27];
                    payload->group_payload.mux_5.cell29 = (*temperature)[28];
                    payload->group_payload.mux_5.cell30 = (*temperature)[29];
                    break;
                case 6:
                    payload->group_payload.mux_6.cell31 = (*temperature)[30];
                    payload->group_payload.mux_6.cell32 = (*temperature)[31];
                    payload->group_payload.mux_6.cell33 = (*temperature)[32];
                    payload->group_payload.mux_6.cell34 = (*temperature)[33];
                    payload->group_payload.mux_6.cell35 = (*temperature)[34];
                    break;
                case 7:
                    payload->group_payload.mux_7.cell36 = (*temperature)[35];
                    payload->group_payload.mux_7.cell37 = (*temperature)[36];
                    payload->group_payload.mux_7.cell38 = (*temperature)[37];
                    payload->group_payload.mux_7.cell39 = (*temperature)[38];
                    payload->group_payload.mux_7.cell40 = (*temperature)[39];
                    break;
                case 8:
                    payload->group_payload.mux_8.cell41 = (*temperature)[40];
                    payload->group_payload.mux_8.cell42 = (*temperature)[41];
                    payload->group_payload.mux_8.cell43 = (*temperature)[42];
                    payload->group_payload.mux_8.cell44 = (*temperature)[43];
                    payload->group_payload.mux_8.cell45 = (*temperature)[44];
                    break;
                case 9:
                    payload->group_payload.mux_9.cell46 = (*temperature)[45];
                    payload->group_payload.mux_9.cell47 = (*temperature)[46];
                    payload->group_payload.mux_9.cell48 = (*temperature)[47];
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

// TODO: update libcan
// bms_cellboard_discharge_temperature_converted_t *temp_api_get_discharge_temp_canlib_payload(size_t *const byte_size) {
//     if (byte_size != NULL) {
//         *byte_size = sizeof(temp_handler.discharge_temp_can_payload);
//     }
//
//     temp_handler.discharge_temp_can_payload.temperature_0 = temp_handler.discharge_temperatures[0U];
//     temp_handler.discharge_temp_can_payload.temperature_1 = temp_handler.discharge_temperatures[1U];
//     temp_handler.discharge_temp_can_payload.temperature_2 = temp_handler.discharge_temperatures[2U];
//     temp_handler.discharge_temp_can_payload.temperature_3 = temp_handler.discharge_temperatures[3U];
//     temp_handler.discharge_temp_can_payload.temperature_4 = temp_handler.discharge_temperatures[4U];
//     return &temp_handler.discharge_temp_can_payload;
// }

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
