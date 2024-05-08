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
    bms_monitor_send_callback spi_send;
    bms_monitor_send_receive_callback spi_send_receive;

    Ltc6811Chain chain;
    Ltc6811Cfgr config[CELLBOARD_SEGMENT_LTC_COUNT];
} hmonitor;

void _bms_monitor_send(uint8_t * data, size_t size) {
    uint8_t aux;
    hmonitor.spi_send_receive(data, &aux, size, 0U);
}

BmsMonitorReturnCode bms_monitor_init(
    bms_monitor_send_callback spi_send,
    bms_monitor_send_receive_callback spi_send_receive)
{
    if (spi_send_receive == NULL)
        return BMS_MONITOR_NULL_POINTER;

    hmonitor.spi_send = spi_send == NULL ? _bms_monitor_send : spi_send;
    hmonitor.spi_send_receive = spi_send_receive;

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


#ifdef CONF_BMS_STRINGS_MODULE_ENABLE

static char * bms_monitor_module_name = "bms monitor";

static char * bms_monitor_return_code_name[] = {
    [BMS_MONITOR_OK] = "ok",
    [BMS_MONITOR_NULL_POINTER] = "null pointer"
};

static char * bms_monitor_return_code_description[] = {
    [BMS_MONITOR_OK] = "executed succesfully",
    [BMS_MONITOR_NULL_POINTER] = "attempt to dereference a NULL pointer"
};

#endif // CONF_BMS_STRINGS_MODULE_ENABLE

#endif // CONF_BMS_MONITOR_MODULE_ENABLE
