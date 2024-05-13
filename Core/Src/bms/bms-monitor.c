/**
 * @file bms-monitor.c
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Handling function for the BMS monitor
 */

#include "bms-monitor.h"

#include "cellboard-def.h"
#include "ltc6811.h"

#ifdef CONF_BMS_MONITOR_MODULE_ENABLE

struct {
    bms_monitor_send_callback send;
    bms_monitor_send_receive_callback send_receive;

    Ltc6811Chain chain;
    Ltc6811Cfgr config[CELLBOARD_SEGMENT_LTC_COUNT];
    bool busy;
} hmonitor;

void _bms_monitor_send(uint8_t * data, size_t size) {
    uint8_t aux;
    hmonitor.send_receive(data, &aux, size, 0U);
}

/**
 * @brief Check if the ADCs are busy or the conversion can be started
 *
 * @attention This function updates the busy field inside the handler
 *
 * @return BmsMonitorReturnCode
 *     BMS_MONITOR_ENCODE_ERROR if the poll command cant be encoded correctly
 *     BMS_MONITOR_BUSY if the ADCs are busy
 *     BMS_MONITOR_OK otherwise
 */
BmsMonitorReturnCode _bms_monitor_chek_busy(void) {
    if (!hmonitor.busy)
        return BMS_MONITOR_OK;

    // Poll for conversion status
    uint8_t poll_result = 0;
    uint8_t poll_data[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_pladc_encode_broadcast(&hmonitor.chain, poll_data);

    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MONITOR_ENCODE_ERROR;

    // TODO: Check for errors
    hmonitor.send_receive(poll_data, &poll_result, byte_size, 1U);
    hmonitor.busy = ltc6811_pladc_check(poll_result);

    return hmonitor.busy ? BMS_MONITOR_BUSY : BMS_MONITOR_OK;
}

BmsMonitorReturnCode bms_monitor_init(
    bms_monitor_send_callback send,
    bms_monitor_send_receive_callback send_receive)
{
    if (send_receive == NULL)
        return BMS_MONITOR_NULL_POINTER;

    hmonitor.send = send == NULL ? _bms_monitor_send : send;
    hmonitor.send_receive = send_receive;
    hmonitor.busy = false;

    ltc6811_chain_init(&hmonitor.chain, CELLBOARD_SEGMENT_LTC_COUNT);

    for (size_t i = 0U; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        hmonitor.config[i].ADCOPT = 0U;
        hmonitor.config[i].DTEN = 0U;
        hmonitor.config[i].REFON = 1U;
        hmonitor.config[i].GPIO = 0U;
        hmonitor.config[i].VUV = 0U;
        hmonitor.config[i].VOV = 0U;
        hmonitor.config[i].DCC = 0U;
        hmonitor.config[i].DCTO = 0U;
    }
}

BmsMonitorReturnCode bms_monitor_start_volt_conversion(void) {
    if (hmonitor.busy)
        return BMS_MONITOR_BUSY;

    uint8_t data[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adcv_encode_broadcast(
        &hmonitor.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_DCP_DISABLED,
        LTC6811_CH_ALL,
        data
    );

    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MONITOR_ENCODE_ERROR;

    // TODO: Return and check errors
    hmonitor.send(data, byte_size);
    hmonitor.busy = true;
    return BMS_MONITOR_OK;
}

BmsMonitorReturnCode bms_monitor_start_gpio_conversion(void) {
    if (hmonitor.busy)
        return BMS_MONITOR_BUSY;

    uint8_t data[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adax_encode_broadcast(
        &hmonitor.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_CHG_GPIO_ALL,
        data
    );

    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MONITOR_ENCODE_ERROR;

    // TODO: Return and check errors
    hmonitor.send(data, byte_size);
    hmonitor.busy = true;
    return BMS_MONITOR_OK;
}

BmsMonitorReturnCode bms_monitor_start_volt_and_gpio_conversion(void) {
    if (hmonitor.busy)
        return BMS_MONITOR_BUSY;

    uint8_t data[LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
    size_t byte_size = ltc6811_adcvax_encode_broadcast(
        &hmonitor.chain,
        LTC6811_MD_27KHZ_14KHZ,
        LTC6811_DCP_DISABLED,
        data
    );

    if (byte_size != LTC6811_POLL_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
        return BMS_MONITOR_ENCODE_ERROR;

    // TODO: Return and check errors
    hmonitor.send(data, byte_size);
    hmonitor.busy = true;
    return BMS_MONITOR_OK;
}

BmsMonitorReturnCode bms_monitor_read_voltages(raw_volt * out) {
    BmsMonitorReturnCode ret = _bms_monitor_chek_busy();
    if (ret != BMS_MONITOR_OK)
        return ret;
    
    uint8_t data[LTC6811_CVXR_COUNT][LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];

    // Read data
    for (size_t reg = LTC6811_CVAR; reg < LTC6811_CVXR_COUNT; ++reg) {
        uint8_t cmd[LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT)];
        size_t byte_size = ltc6811_rdcv_encode_broadcast(&hmonitor.chain, reg, cmd);
        
        if (byte_size != LTC6811_READ_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT))
            return BMS_MONITOR_ENCODE_ERROR;

        hmonitor.send_receive(cmd, data[reg], byte_size, LTC6811_DATA_BUFFER_SIZE(CELLBOARD_SEGMENT_LTC_COUNT));
    }

    // TODO: Decode (and change order?)
    for (size_t reg = LTC6811_CVAR; reg < LTC6811_CVXR_COUNT; ++reg) {
        // ltc6811_rdcv_decode_broadcast(&hmonitor.chain, data[reg], out);
    }

    return BMS_MONITOR_OK;
}


#ifdef CONF_BMS_STRINGS_MODULE_ENABLE

static char * bms_monitor_module_name = "bms monitor";

static char * bms_monitor_return_code_name[] = {
    [BMS_MONITOR_OK] = "ok",
    [BMS_MONITOR_NULL_POINTER] = "null pointer",
    [BMS_MONITOR_ENCODE_ERROR] = "encode error",
    [BMS_MONITOR_BUSY] = "busy"
};

static char * bms_monitor_return_code_description[] = {
    [BMS_MONITOR_OK] = "executed succesfully",
    [BMS_MONITOR_NULL_POINTER] = "attempt to dereference a NULL pointer",
    [BMS_MONITOR_ENCODE_ERROR] = "error during encoding of data",
    [BMS_MONITOR_BUSY] = "monitor busy making conversions"
};

#endif // CONF_BMS_STRINGS_MODULE_ENABLE

#endif // CONF_BMS_MONITOR_MODULE_ENABLE
