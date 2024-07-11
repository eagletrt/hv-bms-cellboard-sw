/**
 * @file bms-manager.c
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Manager for the BMS monitor operations
 */

#include "bms-manager.h"

#include <string.h>

#include "cellboard-def.h"
#include "ltc6811.h"
#include "volt.h"
#include "temp.h"

// TODO: Refactoring
#ifdef CONF_BMS_MANAGER_MODULE_ENABLE

/** @brief Thresholds used during the open wire check */
#define BMS_MANAGER_OPEN_WIRE_THRESHOLD ((millivolt_t)-400.f)
#define BMS_MANAGER_OPEN_WIRE_ZERO ((millivolt_t)0.00005f)

/**
 * @brief BMS manager handler structure
 *
 * @details The pup and pud arrays are used for the open wire check
 * @details The requested configuration should match the actual configuration
 *
 * @param send A pointer to the callback used to send the data via SPI
 * @param send_receive A pointer to the callback used to send and receive the data via SPI
 * @param chain The LTC handler structure
 * @param actual_config The actual configuration register read from the LTC
 * @param requested_config The requested configuration register of the LTC
 * @param pup An array of cells voltages read with pull-up active and inactive (see LTC6811_PUP)
 * @param run_state Index of the current operation of the manager
 */
struct {
    bms_manager_send_callback_t send;
    bms_manager_send_receive_callback_t send_receive;

    Ltc6811Chain chain;
    Ltc6811Cfgr actual_config[CELLBOARD_SEGMENT_LTC_COUNT];
    Ltc6811Cfgr requested_config[CELLBOARD_SEGMENT_LTC_COUNT];
    raw_volt_t pup[2U][CELLBOARD_SEGMENT_SERIES_COUNT];

    size_t run_state;
} hmanager;

/**
 * @brief Function used to send data via SPI if not provided by the user in the init function
 *
 * @param data A pointer to the data to send
 * @param size The number of bytes to send
 */
void _bms_manager_send(uint8_t * data, size_t size) {
    _STATIC uint8_t aux;
    hmanager.send_receive(data, &aux, size, 0U);
}

/**
 * @brief Start the cells voltage ADC conversion
 *
 * @param cells The cells to get the voltage from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_start_volt_conversion(Ltc6811Ch cells) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adcv_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_DCP_DISABLED,
        cells,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    hmanager.send(cmd, byte_size);
    return BMS_MANAGER_OK;
}

/**
 * @brief Start the GPIO voltage ADC conversion
 *
 * @param gpio The GPIO to get the voltage from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_start_gpio_conversion(Ltc6811Chg gpio) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adax_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        gpio,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    hmanager.send(cmd, byte_size);
    return BMS_MANAGER_OK;
}

/**
 * @brief Start the combined cells and GPIO voltage ADC conversion
 *
 * @details This function start the conversion for all 12 cells and the first 2 GPIOs
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_start_combined_conversion(void) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adcvax_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_DCP_DISABLED,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    hmanager.send(cmd, byte_size);
    return BMS_MANAGER_OK;
}

/**
 * @brief Start the open wire ADC conversion
 *
 * @details The first part of the open wire procedure needs the pull up set to 1
 * the second one requires a pull up of 0
 *
 * @param pull_up Internal pull up used for the open wire ADC conversion
 * @param cells The cells to get the voltage from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_start_open_wire_conversion(Ltc6811Pup pull_up, Ltc6811Ch cells) {
    // Encode the command
    uint8_t cmd[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adow_encode_broadcast(
        &hmanager.chain,
        LTC6811_MD_27KHZ_14KHZ,
        pull_up,
        LTC6811_DCP_DISABLED,
        cells,
        cmd
    );
    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    // Send command bytes
    hmanager.send(cmd, byte_size);
    return BMS_MANAGER_OK;
}

/**
 * @brief Write the configuration into the LTCs
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_write_configuration(void) {
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
    hmanager.send(cmd, byte_size);
    return BMS_MANAGER_OK;
}

/**
 * @brief Read the configuration from the LTCs
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_read_configuration(void) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcfg_encode_broadcast(&hmanager.chain, cmd);
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];

    // Send command bytes
    hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));

    byte_size = ltc6811_rdcfg_decode_broadcast(&hmanager.chain, data, hmanager.actual_config);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;
    return BMS_MANAGER_OK;
};

/**
 * @brief Read the cells voltages from the LTCs
 *
 * @param reg The register to read from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_read_voltages(Ltc6811Cvxr reg) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcv_encode_broadcast(
        &hmanager.chain,
        reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt_t volts[LTC6811_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));

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
         */
        size_t index = (reg * LTC6811_REG_CELL_COUNT) + (ltc * LTC6811_CELL_COUNT);
        volt_update_values(index, volts, LTC6811_REG_CELL_COUNT);
    }
    return BMS_MANAGER_OK;
};

/**
 * @brief Read the gpio voltages from the LTCs
 *
 * @param reg The register to read from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_read_gpios(Ltc6811Avxr reg) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdaux_encode_broadcast(
        &hmanager.chain,
        reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_temp_t temps[LTC6811_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));

    byte_size = ltc6811_rdaux_decode_broadcast(&hmanager.chain, data, temps);
    if (byte_size != LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_DECODE_ERROR;

    // Save temperatures
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        /*
         * Each register contains 3 voltages up to 12, the first LTC is connected
         * to the first 12 cells and the second to the last 12 but the single register
         * is read from all the LTCs in the chain so the object has to be calculated
         * accordingly to the register and LTC from which the voltage is read
         */
        size_t index = (reg * LTC6811_REG_AUX_COUNT) + (ltc * LTC6811_AUX_COUNT);
        (void)temp_update_discharge_values(index, temps, LTC6811_REG_AUX_COUNT);
    }
    return BMS_MANAGER_OK;
};

/**
 * @brief Read the cells voltages after the open wire conversion from the LTCs
 *
 * @param reg The register to read from
 * @param pull_up Internal pull up used for the open wire ADC conversion
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_read_open_wire_voltages(Ltc6811Cvxr reg, Ltc6811Pup pull_up) {
    // Encode the command
    uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_rdcv_encode_broadcast(
        &hmanager.chain,
        reg,
        cmd
    );
    if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MANAGER_ENCODE_ERROR;

    uint8_t data[LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    raw_volt_t volts[LTC6811_REG_CELL_COUNT * CELLBOARD_SEGMENT_LTC_COUNT];

    // Send command bytes
    hmanager.send_receive(cmd, data, byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));

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
         */
        size_t index = (reg * LTC6811_REG_CELL_COUNT) + (ltc * LTC6811_CELL_COUNT);
        raw_volt_t * dest = hmanager.pup[pull_up];
        memcpy(dest + index, volts, LTC6811_REG_CELL_COUNT);
    }
    return BMS_MANAGER_OK;
};

/**
 * @brief Check for open wire
 *
 * @details To check for open wire the delta between the converted voltages values
 * is calculated for all the 12 cells excluded the first, then the open wire
 * is detected if:
 *     - The first pull up voltage value is 0.0000 (an epsilon is used to avoid float precision errors)
 *     - The last pull-down voltage value is 0.0000 (same as above)
 *     - At least one delta voltage value is below the -400 mV threshold
 *     
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OPEN_WIRE if an open wire is detected
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode _bms_manager_check_open_wire(void) {
    /*
     */
    float dv;
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // Check first and last voltages
        dv = VOLT_VALUE_TO_MILLIVOLT(hmanager.pup[LTC6811_PUP_ACTIVE][0U]);
        if (dv >= -BMS_MANAGER_OPEN_WIRE_ZERO && dv <= BMS_MANAGER_OPEN_WIRE_ZERO)
            return BMS_MANAGER_OPEN_WIRE;
        dv = VOLT_VALUE_TO_MILLIVOLT(hmanager.pup[LTC6811_PUP_INACTIVE][CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT - 1U]);
        if (dv >= -BMS_MANAGER_OPEN_WIRE_ZERO && dv <= BMS_MANAGER_OPEN_WIRE_ZERO)
            return BMS_MANAGER_OPEN_WIRE;

        // Check other voltages
        for (size_t i = 1U; i <= CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT; ++i) {
            dv = VOLT_VALUE_TO_MILLIVOLT(hmanager.pup[LTC6811_PUP_ACTIVE][i] - hmanager.pup[LTC6811_PUP_INACTIVE][i]);
            if (dv < BMS_MANAGER_OPEN_WIRE_ZERO)
                return BMS_MANAGER_OPEN_WIRE;
        }
    }
    return BMS_MANAGER_OK;
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
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        hmanager.requested_config[i].REFON = 1U;
    }

    return BMS_MANAGER_OK;
}

BmsManagerReturnCode bms_manager_set_discharge_cells(bit_flag32_t cells) {
    for (size_t ltc = 0U; ltc < CELLBOARD_SEGMENT_LTC_COUNT; ++ltc) {
        // Select the correct cells for each LTC
        bit_flag16_t dcc = (cells >> (ltc * CELLBOARD_SEGMENT_SERIES_PER_LTC_COUNT)) &
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

BmsManagerReturnCode bms_manager_run(void) {
#define BMS_MANAGER_COUNTER ((__COUNTER__) - counter_base)

    const size_t counter_base = __COUNTER__;
    BmsManagerReturnCode ret = BMS_MANAGER_OK;
    switch (hmanager.run_state) {
        case 0U:
            ret = _bms_manager_start_volt_conversion(LTC6811_CH_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_read_voltages(LTC6811_CVAR);
            ret = _bms_manager_read_voltages(LTC6811_CVBR);
            ret = _bms_manager_read_voltages(LTC6811_CVCR);
            ret = _bms_manager_read_voltages(LTC6811_CVDR);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_start_gpio_conversion(LTC6811_CHG_GPIO_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_read_gpios(LTC6811_AVAR);
            ret = _bms_manager_read_gpios(LTC6811_AVBR);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_start_open_wire_conversion(LTC6811_PUP_ACTIVE, LTC6811_CH_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_start_open_wire_conversion(LTC6811_PUP_ACTIVE, LTC6811_CH_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVAR, LTC6811_PUP_ACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVBR, LTC6811_PUP_ACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVCR, LTC6811_PUP_ACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVDR, LTC6811_PUP_ACTIVE);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_start_open_wire_conversion(LTC6811_PUP_INACTIVE, LTC6811_CH_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_start_open_wire_conversion(LTC6811_PUP_INACTIVE, LTC6811_CH_ALL);
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVAR, LTC6811_PUP_INACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVBR, LTC6811_PUP_INACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVCR, LTC6811_PUP_INACTIVE);
            ret = _bms_manager_read_open_wire_voltages(LTC6811_CVDR, LTC6811_PUP_INACTIVE);
            // TODO: Set error with open wire
            ret = _bms_manager_check_open_wire();
            break;
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_write_configuration();
            break; 
        case BMS_MANAGER_COUNTER:
            ret = _bms_manager_read_configuration();
            break;

        default:
            break;
    }
    if (++hmanager.run_state >= BMS_MANAGER_COUNTER)
        hmanager.run_state = 0U;
    return ret;

#undef BMS_MANAGER_COUNTER
}

#ifdef CONF_BMS_STRINGS_MODULE_ENABLE

_STATIC char * bms_manager_module_name = "bms manager";

_STATIC char * bms_manager_return_code_name[] = {
    [BMS_MANAGER_OK] = "ok",
    [BMS_MANAGER_NULL_POINTER] = "null pointer",
    [BMS_MANAGER_ENCODE_ERROR] = "encode error",
    [BMS_MANAGER_DECODE_ERROR] = "decode error",
    [BMS_MANAGER_OPEN_WIRE] = "open wire",
    [BMS_MANAGER_BUSY] = "busy"
};

_STATIC char * bms_manager_return_code_description[] = {
    [BMS_MANAGER_OK] = "executed succesfully",
    [BMS_MANAGER_NULL_POINTER] = "attempt to dereference a null pointer",
    [BMS_MANAGER_ENCODE_ERROR] = "error while encoding of data",
    [BMS_MANAGER_DECODE_ERROR] = "error while decoding of data",
    [BMS_MANAGER_OPEN_WIRE] = "open wire detected",
    [BMS_MANAGER_BUSY] = "manager busy making conversions"
};

#endif // CONF_BMS_STRINGS_MODULE_ENABLE

#endif // CONF_BMS_MANAGER_MODULE_ENABLE
