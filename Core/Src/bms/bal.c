/**
 * @file bal.c
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#include "bal.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

struct {
    bool active, new_request;
    millivolt_t target;
    millivolt_t threshold;
} hbal;

BalReturnCode bal_init(void) {
    hbal.active = hbal.new_request = false;
    hbal.target = BAL_TARGET_MAX;
    hbal.threshold = BAL_THRESHOLD_MIN;
    return BAL_OK;
}

BalReturnCode bal_start(millivolt_t target, millivolt_t threshold) {
    if (hbal.new_request)
        return BAL_BUSY;
    if (hbal.active)
        return BAL_OK;

    hbal.new_request = true;
    hbal.target = CELLBOARD_CLAMP(target, BAL_TARGET_MIN, BAL_TARGET_MAX); 
    hbal.threshold = CELLBOARD_CLAMP(threshold, BAL_THRESHOLD_MIN, BAL_THRESHOLD_MAX);

    // TODO: Start balancing
    // TODO: Start watchdog
    // TODO: Update active flag and reset new_request flag after CFGR is read correctly

    return BAL_OK;
}

BalReturnCode bal_stop(void) {
    if (hbal.new_request)
        return BAL_BUSY;
    if (!hbal.active)
        return BAL_OK;

    hbal.new_request = true;
    hbal.target = BAL_TARGET_MAX; 
    hbal.threshold = BAL_THRESHOLD_MIN;

    // TODO: Stop balancing
    // TODO: Stop watchdog 
    // TODO: Update active flag and reset new_request flag after CFGR is read correctly

    return BAL_OK;
}

void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * payload) {
    if (payload == NULL)
        return;

    // Update values
    if (payload->status)
        bal_start(payload->target, payload->threshold);
    else
        bal_stop();
}


#ifdef CONF_BALANCING_STRINGS_ENABLE

static char * bal_module_name = "balancing";

static char * bal_return_code_name[] = {
    [BAL_OK] = "ok",
    [BAL_NULL_POINTER] = "null pointer",
    [BAL_BUSY] = "busy"
};

static char * bal_return_code_description[] = {
    [BAL_OK] = "executed succesfully",
    [BAL_NULL_POINTER] = "attempt to dereference a null pointer",
    [BAL_BUSY] = "the target is busy"
};

#endif // CONF_BALANCING_STRINGS_ENABLE

#endif // CONF_BALANCING_MODULE_ENABLE
