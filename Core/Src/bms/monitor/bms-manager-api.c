/*!
 * \file bms-manager-api.c
 * \date 2024-05-07
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Manager for the BMS monitor operations
 */

#include "bms-manager-api.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "cellboard-def.h"
#include "eagletrt-api.h"
#include "eagletrt.h"
#include "error-api.h"
#include "ltc6811-1-api.h"
#include "bms-monitor-fsm.h"
#include "volt-api.h"
#include "temp-api.h"

#ifdef CONF_BMS_MANAGER_MODULE_ENABLE

EAGLETRT_STATIC struct BmsManagerHandler bms_handler;

/*!
 * \brief Function used to send data via SPI if not provided by the user in the init function
 *
 * \param data A pointer to the data to send
 * \param size The number of bytes to send
 *
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode prv_bms_manager_api_send(uint8_t *const data, const size_t size) {
    EAGLETRT_STATIC uint8_t aux;
    return bms_handler.send_receive(data, &aux, size, 0U);
}

enum BmsManagerReturnCode bms_manager_api_init(const bms_manager_send_callback_t send, const bms_manager_send_receive_callback_t send_receive) {

    const uint8_t gpio_default = 0b11111;
    const uint8_t refon_default = 1U;

    if (send_receive == NULL) {
        return BMS_MANAGER_RC_NULL_POINTER;
    }
    memset(&bms_handler, 0U, sizeof(bms_handler));

    // Set callbacks
    bms_handler.send = (send == NULL) ? prv_bms_manager_api_send : send;
    bms_handler.send_receive = send_receive;

    // Initialize the LTCs
    ltc6811_1_init(&bms_handler.ltc_handler, CELLBOARD_SEGMENT_LTC_COUNT);

    // Initialize the LTCs configurations
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        bms_handler.requested_config[i].GPIO = gpio_default;
        bms_handler.requested_config[i].REFON = refon_default;
    }
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode bms_manager_api_routine(void) {
    EAGLETRT_STATIC bms_monitor_fsm_state_t state = BMS_MONITOR_FSM_STATE_INIT;
    state = bms_monitor_fsm_run_state(state, NULL);
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode bms_manager_api_write_configuration(void) {
    // Encode the command
    uint8_t cmd[LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    const size_t byte_size = ltc6811_1_wrcfg_encode_broadcast(
        &bms_handler.ltc_handler,
        bms_handler.requested_config,
        cmd);
    if (byte_size != LTC6811_1_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send(cmd, byte_size);
    if (code != BMS_MANAGER_RC_BUSY && code != BMS_MANAGER_RC_OK) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
    } else {
        error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
    }
    return code;
}

enum BmsManagerReturnCode bms_manager_api_read_configuration(void) {
    // Encode the command
    uint8_t cmd[LTC6811_1_READ_BUFFER_SIZE];
    size_t byte_size = ltc6811_1_rdcfg_encode_broadcast(&bms_handler.ltc_handler, cmd);
    if (byte_size != LTC6811_1_READ_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send_receive(cmd, data, byte_size, LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_RC_OK) {
        if (code != BMS_MANAGER_RC_BUSY) {
            error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
        }
        return code;
    }

    byte_size = ltc6811_1_rdcfg_decode_broadcast(&bms_handler.ltc_handler, data, bms_handler.actual_config);
    if (byte_size != LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
        return BMS_MANAGER_RC_DECODE_ERROR;
    }
    error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_CONFIGURATION);
    return code;
}

enum BmsManagerReturnCode bms_manager_api_start_volt_conversion(void) {
    // Encode the command
    uint8_t cmd[LTC6811_1_POLL_BUFFER_SIZE];
    const size_t byte_size = ltc6811_1_adcv_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        LTC6811_1_DCP_DISABLED,
        LTC6811_1_CH_ALL,
        cmd);
    if (byte_size != LTC6811_1_POLL_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send(cmd, byte_size);
    if (code != BMS_MANAGER_RC_BUSY && code != BMS_MANAGER_RC_OK) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
    } else {
        error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
    }
    return code;
}

enum BmsManagerReturnCode bms_manager_api_start_temp_conversion(void) {
    // Encode the command
    uint8_t cmd[LTC6811_1_POLL_BUFFER_SIZE];
    const size_t byte_size = ltc6811_1_adax_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        LTC6811_1_CHG_GPIO_ALL,
        cmd);
    if (byte_size != LTC6811_1_POLL_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Send command bytes
    enum BmsManagerReturnCode code = bms_handler.send(cmd, byte_size);
    if (code != BMS_MANAGER_RC_BUSY && code != BMS_MANAGER_RC_OK) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
    } else {
        error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
    }
    return code;
}

enum BmsManagerReturnCode bms_manager_api_start_open_wire_conversion(const enum BmsManagerOpenWireOperation pull_up) {
    // Encode the command

    if (pull_up >= BMS_MANAGER_OPEN_WIRE_OPERATION_COUNT) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t cmd[LTC6811_1_POLL_BUFFER_SIZE];
    size_t byte_size = ltc6811_1_adow_encode_broadcast(
        &bms_handler.ltc_handler,
        LTC6811_1_MD_27KHZ,
        (enum Ltc68111Pup)pull_up,
        LTC6811_1_DCP_DISABLED,
        LTC6811_1_CH_ALL,
        cmd);
    if (byte_size != LTC6811_1_POLL_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send(cmd, byte_size);
    if (code != BMS_MANAGER_RC_BUSY && code != BMS_MANAGER_RC_OK) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
    } else {
        error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
    }
    return code;
}

enum BmsManagerReturnCode bms_manager_api_poll_conversion_status(void) {
    // Encode command
    uint8_t cmd[LTC6811_1_POLL_BUFFER_SIZE];
    const size_t byte_size = ltc6811_1_pladc_encode_broadcast(&bms_handler.ltc_handler, cmd);
    if (byte_size != LTC6811_1_POLL_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Send command bytes
    uint8_t poll_status = 0;
    const enum BmsManagerReturnCode code = bms_handler.send_receive(cmd, &poll_status, byte_size, LTC6811_1_POLL_BYTE_COUNT);
    if (code != BMS_MANAGER_RC_OK) {
        if (code != BMS_MANAGER_RC_BUSY) {
            error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL);
        }
        return code;
    }
    error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_POLL);
    return ltc6811_1_pladc_is_completed(poll_status) ? BMS_MANAGER_RC_OK : BMS_MANAGER_RC_BUSY;
}

enum BmsManagerReturnCode bms_manager_api_read_voltages(const enum BmsManagerVoltageRegister reg) {
    // Encode the command

    if (reg >= BMS_MANAGER_VOLTAGE_REGISTER_COUNT) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t cmd[LTC6811_1_READ_BUFFER_SIZE];
    size_t byte_size = ltc6811_1_rdcv_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Cvxr)reg,
        cmd);
    if (byte_size != LTC6811_1_READ_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt volts[LTC6811_1_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send_receive(cmd, data, byte_size, LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_RC_OK) {
        if (code != BMS_MANAGER_RC_BUSY) {
            error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
        }
        return code;
    }

    byte_size = ltc6811_1_rdcv_decode_broadcast(&bms_handler.ltc_handler, data, volts);
    if (byte_size != LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);
        return BMS_MANAGER_RC_DECODE_ERROR;
    }
    error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_VOLTAGE);

    // Save voltages
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        /*
         * Each register contains 3 voltages up to 12, the first LTC is connected
         * to the first 12 cells and the second to the last 12 but the single register
         * is read from all the LTCs in the chain so the object has to be calculated
         * accordingly to the register and LTC from which the voltage is read
         *
         * The FIRST cell is connected to the FIRST pin of the LAST LTC, so the order
         * of the cells has to be swapped (cells 0 to 11 goes to 12 and cells 12 to 23 goes to 0)
         */
        const size_t index = (reg * LTC6811_1_REG_CELL_COUNT) + (ltc * LTC6811_1_CELL_COUNT);
        const size_t off = (CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U) * LTC6811_1_REG_CELL_COUNT;
        for (size_t i = 0U; i < LTC6811_1_REG_CELL_COUNT; ++i) {
            const volt value = BMS_MANAGER_RAW_VOLTAGE_TO_VOLT(volts[off + i]);
            volt_api_update_value(index + i, value);
        }
    }
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode bms_manager_api_read_temperatures(const enum BmsManagerTemperatureRegister reg) {
    // Encode the command
    uint8_t cmd[LTC6811_1_READ_BUFFER_SIZE];
    size_t byte_size = ltc6811_1_rdaux_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Avxr)reg,
        cmd);
    if (byte_size != LTC6811_1_READ_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_temp_t temp[LTC6811_1_REG_AUX_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send_receive(cmd, data, byte_size, LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_RC_OK) {
        if (code != BMS_MANAGER_RC_BUSY) {
            error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
        }
        return code;
    }

    byte_size = ltc6811_1_rdaux_decode_broadcast(&bms_handler.ltc_handler, data, temp);
    if (byte_size != LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);
        return BMS_MANAGER_RC_DECODE_ERROR;
    }
    error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_TEMPERATURE_DISCHARGE);

    // Save temperatures
    // Only the last LTC has the temperature sensors attached
    const size_t ltc = 1U;
    /*
     * Each register contains 3 temperatures up to 10 (the last value of the last
     * register is not counted) the first LTC is connected to the first 5 temperature
     * sensors and the second to the last 5 but the single register
     * is read from all the LTCs in the chain so the object has to be calculated
     * accordingly to the register and LTC from which the temperature is read
     *
     * The FIRST sensor is connected to the LAST LTC, so the order of the temperatures
     * has to be swapped (temperature 0 to 4 goes to 5 and temperature 5 to 9 goes to 0)
     */
    const size_t temp_size = reg >= BMS_MANAGER_TEMPERATURE_REGISTER_B ? LTC6811_1_REG_AUX_COUNT - 1U : LTC6811_1_REG_AUX_COUNT;
    const size_t index = reg * LTC6811_1_REG_AUX_COUNT;
    const size_t off = ltc * LTC6811_1_REG_AUX_COUNT;
    for (size_t i = 0U; i < temp_size; ++i) {
        volt value = BMS_MANAGER_RAW_GPIO_VALUE_TO_VOLT(temp[off + i]);
        temp_api_update_discharge_value(index + i, value);
    }
    return BMS_MANAGER_RC_OK;
}

enum BmsManagerReturnCode bms_manager_api_read_open_wire_voltages(const enum BmsManagerVoltageRegister reg, const enum BmsManagerOpenWireOperation pull_up_operation) {

    if (reg >= BMS_MANAGER_VOLTAGE_REGISTER_COUNT || pull_up_operation >= BMS_MANAGER_OPEN_WIRE_OPERATION_COUNT) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    // Encode the command
    uint8_t cmd[LTC6811_1_READ_BUFFER_SIZE];
    size_t byte_size = ltc6811_1_rdcv_encode_broadcast(
        &bms_handler.ltc_handler,
        (enum Ltc68111Cvxr)reg,
        cmd);
    if (byte_size != LTC6811_1_READ_BUFFER_SIZE) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        return BMS_MANAGER_RC_ENCODE_ERROR;
    }

    uint8_t data[LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt volts[LTC6811_1_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    const enum BmsManagerReturnCode code = bms_handler.send_receive(cmd, data, byte_size, LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_RC_OK) {
        if (code != BMS_MANAGER_RC_BUSY) {
            error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        }
        return code;
    }

    byte_size = ltc6811_1_rdcv_decode_broadcast(&bms_handler.ltc_handler, data, volts);
    if (byte_size != LTC6811_1_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)) {
        error_api_set(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);
        return BMS_MANAGER_RC_DECODE_ERROR;
    }
    error_api_reset(ERROR_GROUP_BMS_MONITOR_COMMUNICATION, ERROR_BMS_MONITOR_COMMUNICATION_INSTANCE_OPEN_WIRE);

    // Save voltages
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        /*
         * Each register contains 3 voltages up to 12, the first LTC is connected
         * to the first 12 cells and the second to the last 12 but the single register
         * is read from all the LTCs in the chain so the object has to be calculated
         * accordingly to the register and LTC from which the voltage is read
         *
         * The FIRST cell is connected to the FIRST pin of the LAST LTC, so the order
         * of the cells has to be swapped (cells 0 to 11 goes to 12 and cells 12 to 23 goes to 0)
         */
        const size_t index = (reg * LTC6811_1_REG_CELL_COUNT) + (ltc * LTC6811_1_CELL_COUNT);
        const size_t off = (CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U) * LTC6811_1_REG_CELL_COUNT;
        for (size_t i = 0U; i < LTC6811_1_REG_CELL_COUNT; ++i) {
            const volt value = BMS_MANAGER_RAW_VOLTAGE_TO_VOLT(volts[off + i]);
            bms_handler.pup[pull_up_operation][index + i] = value;
        }
    }
    return BMS_MANAGER_RC_OK;
}

bit_flag32 bms_manager_api_check_open_wire(void) {

    bit_flag32 open_wire_cells = 0U;

    size_t offset = 0;
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ltc++) {

        offset = ltc * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT;

        // Check first and last voltages
        if (bms_handler.pup[LTC6811_1_PUP_ACTIVE][0U + offset] == BMS_MANAGER_OPEN_WIRE_ZERO_V) {
            error_api_set(ERROR_GROUP_OPEN_WIRE, 0U);
            open_wire_cells = EAGLETRT_API_BIT_SET(open_wire_cells, 0U + offset);
        }
        if (bms_handler.pup[LTC6811_1_PUP_INACTIVE][CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT - 1U + offset] == BMS_MANAGER_OPEN_WIRE_ZERO_V) {
            error_api_set(ERROR_GROUP_OPEN_WIRE, 0U);
            open_wire_cells = EAGLETRT_API_BIT_SET(open_wire_cells, CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT - 1U + offset);
        }

        // Check other voltages
        for (size_t i = 1U; i < CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT; ++i) {
            // TODO: Save and send via CAN cell that failed the open wire check
            const volt delta_v = bms_handler.pup[LTC6811_1_PUP_ACTIVE][i + offset] - bms_handler.pup[LTC6811_1_PUP_INACTIVE][i + offset];
            if (delta_v < LTC6811_1_OPEN_WIRE_THRESHOLD_V) {
                error_api_set(ERROR_GROUP_OPEN_WIRE, 0U);
                open_wire_cells = EAGLETRT_API_BIT_SET(open_wire_cells, i + offset);
            }
        }
    }

    if (open_wire_cells == 0U) {
        error_api_reset(ERROR_GROUP_OPEN_WIRE, 0U);
    }

    return open_wire_cells;
}

enum BmsManagerReturnCode bms_manager_api_set_discharge_cells(bit_flag32 cells) {
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // The first 12 cells are connected to the last LTC
        const size_t ltc_index = CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U;

        // Select the correct cells for each LTC
        const bit_flag16_t dcc = (bit_flag16_t)((cells >> (ltc_index * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) &
                                                ((1U << CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) - 1U));

        // Set configuration
        bms_handler.requested_config[ltc].DCTO = (dcc == 0U) ? LTC6811_1_DCTO_OFF : LTC6811_1_DCTO_30S;
        bms_handler.requested_config[ltc].DCC = dcc;
    }
    return BMS_MANAGER_RC_OK;
}

bit_flag32 bms_manager_api_get_discharge_cells(void) {
    bit_flag32 cells = 0U;
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // Get cells from config
        cells |= (bms_handler.actual_config[ltc].DCC << (ltc * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT));
    }
    return cells;
}

#ifdef CONF_BMS_STRINGS_MODULE_ENABLE

EAGLETRT_STATIC char *bms_manager_module_name = "bms manager";

EAGLETRT_STATIC char *bms_manager_return_code_name[] = {
    [BMS_MANAGER_RC_OK] = "ok",
    [BMS_MANAGER_RC_NULL_POINTER] = "null pointer",
    [BMS_MANAGER_RC_ENCODE_ERROR] = "encode error",
    [BMS_MANAGER_RC_DECODE_ERROR] = "decode error",
    [BMS_MANAGER_RC_OPEN_WIRE] = "open wire",
    [BMS_MANAGER_RC_BUSY] = "busy",
    [BMS_MANAGER_RC_COMMUNICATION_ERROR] = "communication error",
    [BMS_MANAGER_RC_ERROR] = "error"
};

EAGLETRT_STATIC char *bms_manager_return_code_description[] = {
    [BMS_MANAGER_RC_OK] = "executed succesfully",
    [BMS_MANAGER_RC_NULL_POINTER] = "attempt to dereference a null pointer",
    [BMS_MANAGER_RC_ENCODE_ERROR] = "error while encoding of data",
    [BMS_MANAGER_RC_DECODE_ERROR] = "error while decoding of data",
    [BMS_MANAGER_RC_OPEN_WIRE] = "open wire detected",
    [BMS_MANAGER_RC_BUSY] = "the manager or peripheral are busy",
    [BMS_MANAGER_RC_COMMUNICATION_ERROR] = "error during data transmission or reception",
    [BMS_MANAGER_RC_ERROR] = "unknown error"
};

int bms_manager_api_get_config_string(
    const Ltc68111Cfgr config,
    char *const out,
    const size_t size) {
    const char *const fmt =
        "adcopt: %3hu\r\n"
        "dten:   %3hu\r\n"
        "refon:  %3hu\r\n"
        "gpio: 0x%03hx\r\n"
        "vuv:  0x%03hx\r\n"
        "vov:  0x%03hx\r\n"
        "dcc:  0x%03hx\r\n"
        "dcto:   %3hu\r\n";
    return snprintf(
        out, size, fmt, config.ADCOPT, config.DTEN, config.REFON, config.GPIO, config.VUV, config.VOV, config.DCC, config.DCTO);
}

int bms_manager_api_get_requested_config_string(
    const size_t ltc,
    char *const out,
    size_t size) {
    if (ltc >= CELLBOARD_SEGMENT_LTC_COUNT)
        return 0;
    return bms_manager_api_get_config_string(bms_handler.requested_config[ltc], out, size);
}

int bms_manager_api_get_actual_config_string(
    const size_t ltc,
    char *const out,
    const size_t size) {
    if (ltc >= CELLBOARD_SEGMENT_LTC_COUNT)
        return 0;
    return bms_manager_api_get_config_string(bms_handler.actual_config[ltc], out, size);
}

#endif // CONF_BMS_STRINGS_MODULE_ENABLE

#endif // CONF_BMS_MANAGER_MODULE_ENABLE
