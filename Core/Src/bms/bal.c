/**
 * @file bal.c
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#include "bal.h"

#include "ltc6811.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

struct {
    Ltc6811Chain chain;
    Ltc6811Cfgr config[CELLBOARD_SEGMENT_LTC_COUNT];
} hbal;

BalReturnCode bal_init(void) {
    ltc6811_chain_init(&hbal.chain, CELLBOARD_SEGMENT_LTC_COUNT);

    for (size_t i = 0; i < CELLBOARD_SEGMENT_LTC_COUNT; ++i) {
        hbal.config[i].ADCOPT = 0;
        hbal.config[i].REFON = 1;
        hbal.config[i].GPIO = 0;
        hbal.config[i].VUV = 0;
        hbal.config[i].VOV = 0;
        hbal.config[i].DCC = 0;
        hbal.config[i].DCTO = LTC6811_DCTO_OFF;
    }

    return BAL_OK;
}

#ifdef CONF_BALANCING_STRINGS_ENABLE

static char * bal_module_name = "balancing";

static char * bal_return_code_name[] = {
    [BAL_OK] = "ok"
};

static char * bal_return_code_description[] = {
    [BAL_OK] = "executed succesfully"
};

#endif // CONF_BALANCING_STRINGS_ENABLE

#endif // CONF_BALANCING_MODULE_ENABLE
