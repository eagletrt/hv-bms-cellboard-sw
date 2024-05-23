/**
 * @file bal.c
 * @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#include "bal.h"

#include <string.h>

#include "cellboard-def.h"
#include "fsm.h"
#include "post.h"
#include "timebase.h"
#include "watchdog.h"
#include "volt.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

/** @brief Balancing watchdog timeout in ms */
#define BAL_WATCHDOG_TIMEOUT ((time_t)(3000U))

/**
 * @brief Definition of the balancing parameters
 *
 * @param active True if the balancing is active, false otherwise
 * @param target The minimum target voltage that can be reached while discharging (in mV)
 * @param threshold The minimum difference between the maxiumum and minimum
 * cell voltages that can be reached (in mV)
 */
typedef struct {
    bool active;
    millivolt_t target;
    millivolt_t threshold;
} BalParams;

/**
 * @brief Balancing handler structure
 *
 * @details The requested parameters are expected to be equals to the actual parameters
 *
 * @param bal_event The FSM event data
 * @param actual The actual balancing parameters
 * @param requested The given balancing parameters
 */
static struct {
    fsm_event_data_t bal_event;
    Watchdog watchdog;

    BalParams actual;
    BalParams requested;
} hbal;


// TODO: Handle discharge and cooldown
void _bal_timeout(void) { }

BalReturnCode bal_init(void) {
    memset(&hbal, 0U, sizeof(hbal));
    watchdog_init(
        &hbal.watchdog,
        TIMEBASE_TIME_TO_TICKS(BAL_WATCHDOG_TIMEOUT, timebase_get_resolution()),
        _bal_timeout        
    );
    return BAL_OK;
}

void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * payload) {
    if (payload == NULL)
        return;
    // Ignore stop if not balancing
    if (!hbal.actual.active && !payload->status)
        return;
 
    // Update data
    hbal.requested.active = payload->status;
    hbal.requested.target = CELLBOARD_CLAMP(payload->target, BAL_TARGET_MIN, BAL_TARGET_MAX);
    hbal.requested.threshold = CELLBOARD_CLAMP(payload->threshold, BAL_THRESHOLD_MIN, BAL_THRESHOLD_MAX);

    // Check if requested status match actual
    if (hbal.actual.active == hbal.requested.active) {
        watchdog_reset(&hbal.watchdog);
    }
    else {
        // Send event to the FSM
        hbal.bal_event = payload->status ?
            FSM_EVENT_TYPE_BALANCING_START :
            FSM_EVENT_TYPE_BALANCING_STOP;
        fsm_event_trigger(&hbal.bal_event);
    }
}

BalReturnCode bal_start(void) {
    // Check actual balancing state
    if (hbal.actual.active)
        return BAL_OK;

    // Set discharge configuration
    const millivolt_t target = hbal.requested.target + hbal.requested.threshold;
    bit_flag32_t cells_to_discharge = volt_select_values(target);
    bms_manager_set_discharge_cells(cells_to_discharge);

    // Start watchdog
    watchdog_restart(&hbal.watchdog);
    return BAL_OK;
}

BalReturnCode bal_stop(void) {
    // Check actual balancing status
    if (!hbal.actual.active)
        return BAL_OK;

    // Set discharge configuration
    bms_manager_set_discharge_cells(0U);

    // Stop watchdog
    watchdog_stop(&hbal.watchdog);
    return BAL_OK;
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
