/**
 * @file bms-manager.c
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Manager for the BMS monitor operations
 */

#include "bms-manager.h"

#include <string.h>

#include "bms-monitor-fsm.h"
#include "temp.h"

// TODO: Refactoring
#ifdef CONF_BMS_MANAGER_MODULE_ENABLE

_STATIC _BmsManagerHandler hmanager;

/**
 * @brief Function used to send data via SPI if not provided by the user in the init function
 *
 * @param data A pointer to the data to send
 * @param size The number of bytes to send
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_send(uint8_t * data, size_t size) {
    _STATIC uint8_t aux;
    return hmanager.send_receive(data, &aux, size, 0U);
}

/**
 * @brief Increment the communication error counter and check if a critical
 * error should be set
 */
_STATIC_INLINE void _bms_manager_inc_communication_error_counter(void) {
    ++hmanager.communication_error_count;
    // TODO: Set error if count is greater than a certain threshold
}

/** @brief Reset the communication error counter */
_STATIC_INLINE void _bms_manager_reset_communication_error_counter(void) {
    hmanager.communication_error_count = 0U;
}

BmsManagerReturnCode bms_manager_init(
    bms_manager_send_callback_t send,
    bms_manager_send_receive_callback_t send_receive)
{
    if (send_receive == NULL)
        return BMS_MANAGER_NULL_POINTER;
    memset(&hmanager, 0U, sizeof(hmanager));

    // Set callbacks
    hmanager.send = (send == NULL) ? _bms_manager_send : send;
    hmanager.send_receive = send_receive;

    // Initialize the LTCs
    ltc6811_chain_init(&hmanager.chain, CELLBOARD_SEGMENT_LTC_COUNT);

    // Initialize the LTCs configurations
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i)
        hmanager.requested_config[i].REFON = 1U;
    return BMS_MANAGER_OK;
}

BmsManagerReturnCode bms_manager_routine(void) {
    _STATIC bms_monitor_fsm_state_t state = BMS_MONITOR_FSM_STATE_INIT;
    state = bms_monitor_fsm_run_state(state, NULL);
    return BMS_MANAGER_OK;
}

BmsManagerReturnCode bms_manager_write_configuration(void) {
    // Encode the command
    uint8_t cmd[LTC6811_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_wrcfg_encode_broadcast(
        &hmanager.chain,
        hmanager.requested_config,
        cmd
    );
    if (byte_size != LTC6811_WRITE_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send(cmd, byte_size);
    if (code != BMS_MANAGER_BUSY && code != BMS_MANAGER_OK)
        _bms_manager_inc_communication_error_counter();
    else
        _bms_manager_reset_communication_error_counter();
    return code;
}

BmsManagerReturnCode bms_manager_read_configuration(void) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcfg_encode_broadcast(&hmanager.chain, cmd);
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)] = { 0 };

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_OK) {
        if (code != BMS_MANAGER_BUSY)
            _bms_manager_inc_communication_error_counter();
        return code;
    }
    _bms_manager_reset_communication_error_counter();

    byte_size = ltc6811_rdcfg_decode_broadcast(&hmanager.chain, data, hmanager.actual_config);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;
    return code;
};

BmsManagerReturnCode bms_manager_start_volt_conversion(void) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adcv_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_DCP_DISABLED,
        LTC6811_CH_ALL,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send(cmd, byte_size);
    if (code != BMS_MANAGER_BUSY && code != BMS_MANAGER_OK)
        _bms_manager_inc_communication_error_counter();
    else
        _bms_manager_reset_communication_error_counter();
    return code;
}

BmsManagerReturnCode bms_manager_start_temp_conversion(void) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adax_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_CHG_GPIO_ALL,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send(cmd, byte_size);
    if (code != BMS_MANAGER_BUSY && code != BMS_MANAGER_OK)
        _bms_manager_inc_communication_error_counter();
    else
        _bms_manager_reset_communication_error_counter();
    return code;
}

BmsManagerReturnCode bms_manager_start_open_wire_conversion(Ltc6811Pup pull_up) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adow_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        pull_up,
        LTC6811_DCP_DISABLED,
        LTC6811_CH_ALL,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send(cmd, byte_size);
    if (code != BMS_MANAGER_BUSY && code != BMS_MANAGER_OK)
        _bms_manager_inc_communication_error_counter();
    else
        _bms_manager_reset_communication_error_counter();
    return code;
}

BmsManagerReturnCode bms_manager_poll_conversion_status(void) {
    // Encode command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_pladc_encode_broadcast(&hmanager.chain, cmd);
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    uint8_t poll_status = 0;
    BmsManagerReturnCode code = hmanager.send_receive(cmd, &poll_status, byte_size, LTC6811_POLL_BYTE_COUNT);
    if (code != BMS_MANAGER_OK) {
        if (code != BMS_MANAGER_BUSY)
            _bms_manager_inc_communication_error_counter();
        return code;
    }
    _bms_manager_reset_communication_error_counter();
    return ltc6811_pladc_check(poll_status) ? BMS_MANAGER_OK : BMS_MANAGER_BUSY;
}

BmsManagerReturnCode bms_manager_read_voltages(BmsManagerVoltageRegister reg) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcv_encode_broadcast(
        &hmanager.chain,
        (Ltc6811Cvxr)reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt_t volts[LTC6811_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_OK) {
        if (code != BMS_MANAGER_BUSY)
            _bms_manager_inc_communication_error_counter();
        return code;
    }
    _bms_manager_reset_communication_error_counter();

    byte_size = ltc6811_rdcv_decode_broadcast(&hmanager.chain, data, volts);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;

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
        size_t index = (reg * LTC6811_REG_CELL_COUNT) + (ltc * LTC6811_CELL_COUNT);
        size_t off = (CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U) * LTC6811_REG_CELL_COUNT;
        volt_update_values(index, volts + off, LTC6811_REG_CELL_COUNT);
    }
    return BMS_MANAGER_OK;
};

BmsManagerReturnCode bms_manager_read_temperatures(BmsManagerTemperatureRegister reg) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdaux_encode_broadcast(
        &hmanager.chain,
        (Ltc6811Avxr)reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_temp_t temp[LTC6811_REG_AUX_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_OK) {
        if (code != BMS_MANAGER_BUSY)
            _bms_manager_inc_communication_error_counter();
        return code;
    }
    _bms_manager_reset_communication_error_counter();

    byte_size = ltc6811_rdaux_decode_broadcast(&hmanager.chain, data, temp);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;

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
    const size_t temp_size = reg >= BMS_MANAGER_TEMPERATURE_REGISTER_B ?
        LTC6811_REG_AUX_COUNT - 1U :
        LTC6811_REG_AUX_COUNT;
    size_t index = reg * LTC6811_REG_AUX_COUNT;
    size_t off = (CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U) * LTC6811_REG_AUX_COUNT;
    temp_update_discharge_values(index, temp + off, temp_size);
    return BMS_MANAGER_OK;
}

BmsManagerReturnCode bms_manager_read_open_wire_voltages(BmsManagerVoltageRegister reg, BmsManagerOpenWireOperation op) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcv_encode_broadcast(
        &hmanager.chain,
        (Ltc6811Cvxr)reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt_t volts[LTC6811_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    BmsManagerReturnCode code = hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    if (code != BMS_MANAGER_OK) {
        if (code != BMS_MANAGER_BUSY)
            _bms_manager_inc_communication_error_counter();
        return code;
    }
    _bms_manager_reset_communication_error_counter();

    byte_size = ltc6811_rdcv_decode_broadcast(&hmanager.chain, data, volts);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;

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
        size_t index = (reg * LTC6811_REG_CELL_COUNT) + (ltc * LTC6811_CELL_COUNT);
        size_t off = (CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U) * LTC6811_REG_CELL_COUNT;
        memcpy(hmanager.pup[op] + index, volts + off, LTC6811_REG_CELL_COUNT * sizeof(volts[0]));
        // memcpy(hvolt.voltages + index, values, size * sizeof(hvolt.voltages[0]));
        // volt_update_values(index, volts + off, LTC6811_REG_CELL_COUNT);
    }
    return BMS_MANAGER_OK;
};

BmsManagerReturnCode bms_manager_check_open_wire(void) {
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // Check first and last voltages
        if (hmanager.pup[LTC6811_PUP_ACTIVE][0U] == BMS_MANAGER_OPEN_WIRE_ZERO)
            return BMS_MANAGER_OPEN_WIRE;
        if (hmanager.pup[LTC6811_PUP_INACTIVE][CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT - 1U] == BMS_MANAGER_OPEN_WIRE_ZERO)
            return BMS_MANAGER_OPEN_WIRE;

        // Check other voltages
        for (size_t i = 1U; i <= CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT; ++i) {
            int16_t dv = (int16_t)hmanager.pup[LTC6811_PUP_ACTIVE][i] - (int16_t)hmanager.pup[LTC6811_PUP_INACTIVE][i];
            if (dv < BMS_MANAGER_OPEN_WIRE_THRESHOLD)
                return BMS_MANAGER_OPEN_WIRE;
        }
    }
    return BMS_MANAGER_OK;
}

BmsManagerReturnCode bms_manager_set_discharge_cells(bit_flag32_t cells) {
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // The first 12 cells are connected to the last LTC
        const size_t ltc_index = CELLBOARD_SEGMENT_LTC_COUNT - ltc - 1U;

        // Select the correct cells for each LTC
        bit_flag16_t dcc = (cells >> (ltc_index * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) &
            ((1U << CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT) - 1U);

        // Set configuration
        hmanager.requested_config[ltc].DCTO = (dcc == 0U) ? LTC6811_DCTO_OFF : LTC6811_DCTO_30S;
        hmanager.requested_config[ltc].DCC = dcc;
    }
    return BMS_MANAGER_OK;
}

bit_flag32_t bms_manager_get_discharge_cells(void) {
    bit_flag32_t cells = 0U;
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // Get cells from config
        cells |= (hmanager.actual_config[ltc].DCC << (ltc * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT));
    }
    return cells;
}

#ifdef CONF_BMS_STRINGS_MODULE_ENABLE

_STATIC char * bms_manager_module_name = "bms manager";

_STATIC char * bms_manager_return_code_name[] = {
    [BMS_MANAGER_OK] = "ok",
    [BMS_MANAGER_NULL_POINTER] = "null pointer",
    [BMS_MANAGER_ENCODE_ERROR] = "encode error",
    [BMS_MANAGER_DECODE_ERROR] = "decode error",
    [BMS_MANAGER_OPEN_WIRE] = "open wire",
    [BMS_MANAGER_BUSY] = "busy",
    [BMS_MANAGER_COMMUNICATION_ERROR] = "communication error",
    [BMS_MANAGER_ERROR] = "error"
};

_STATIC char * bms_manager_return_code_description[] = {
    [BMS_MANAGER_OK] = "executed succesfully",
    [BMS_MANAGER_NULL_POINTER] = "attempt to dereference a null pointer",
    [BMS_MANAGER_ENCODE_ERROR] = "error while encoding of data",
    [BMS_MANAGER_DECODE_ERROR] = "error while decoding of data",
    [BMS_MANAGER_OPEN_WIRE] = "open wire detected",
    [BMS_MANAGER_BUSY] = "the manager or peripheral are busy",
    [BMS_MANAGER_COMMUNICATION_ERROR] = "error during data transmission or reception",
    [BMS_MANAGER_ERROR] = "unknown error"
};

#endif // CONF_BMS_STRINGS_MODULE_ENABLE

#endif // CONF_BMS_MANAGER_MODULE_ENABLE
