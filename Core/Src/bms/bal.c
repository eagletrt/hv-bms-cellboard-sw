/**
 * @file bal.c @date 2024-04-17
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
#include "identity.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

/**
 * @brief Definition of the balancing parameters
 *
 * @param target The minimum target voltage raw value that can be reached while discharging
 * @param threshold The minimum difference between the maxiumum and minimum
 * cell voltages that can be reached
 */
typedef struct {
    raw_volt_t target;
    raw_volt_t threshold;
} BalParams;

/**
 * @brief Balancing handler structure
 *
 * @details The requested parameters are expected to be equals to the actual parameters
 *
 * @param event The FSM event data
 * @param can_payload The canlib payload of the balancing module
 * @param watchdog The watchdog that stops the balancing procedure when timed out
 * @param active True if the balancing is active, false otherwise
 * @param paused True if the balancing is paused, false otherwise
 * @param params The balancing parameters
 */
_STATIC struct {
    fsm_event_data_t event;
    bms_cellboard_balancing_status_converted_t can_payload;
    Watchdog watchdog;

    bool active, paused;
    BalParams params;
} hbal;

void _bal_timeout(void) {
    // Stop balancing
    hbal.event.type = FSM_EVENT_TYPE_BALANCING_STOP;
    fsm_event_trigger(&hbal.event);
}

BalReturnCode bal_init(void) {
    memset(&hbal, 0U, sizeof(hbal));

    // Set default event and canlib payload
    hbal.event.type = FSM_EVENT_TYPE_IGNORED;

    hbal.can_payload.cellboard_id = identity_get_cellboard_id();

    // Set default balancing parameters
    hbal.params.target = BAL_TARGET_MAX;
    hbal.params.threshold = BAL_THRESHOLD_MAX;

    // Initialize main balancing watchog
    (void)watchdog_init(
        &hbal.watchdog,
        TIMEBASE_TIME_TO_TICKS(BAL_TIMEOUT, timebase_get_resolution()),
        _bal_timeout
    );
    return BAL_OK;
}

// TODO: Handle unavailable watchdog
void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * payload) {
    if (payload == NULL)
        return;
    // Ignore stop command if not balancing
    if (!hbal.active && !payload->start)
        return;
 
    // Update data
    hbal.params.target = CELLBOARD_CLAMP(payload->target, BAL_TARGET_MIN, BAL_TARGET_MAX);
    hbal.params.threshold = CELLBOARD_CLAMP(payload->threshold, BAL_THRESHOLD_MIN, BAL_THRESHOLD_MAX);

    // Reset watchdog for each new message
    WatchdogReturnCode code = watchdog_reset(&hbal.watchdog);
    if (code == WATCHDOG_UNAVAILABLE)
        return;

    // Send event to the FSM
    if (hbal.active != payload->start) {
        hbal.event.type = payload->start ?
            FSM_EVENT_TYPE_BALANCING_START :
            FSM_EVENT_TYPE_BALANCING_STOP;
        fsm_event_trigger(&hbal.event);
    }
}

bool bal_is_active(void) {
    return hbal.active;
}

bool bal_is_paused(void) {
    return hbal.paused;
}

BalReturnCode bal_start(void) {
    // Check actual balancing state
    if (hbal.active)
        return BAL_OK;

    // Start watchdog
    WatchdogReturnCode code = watchdog_restart(&hbal.watchdog);
    if (code == WATCHDOG_UNAVAILABLE)
        return BAL_WATCHDOG_ERROR;

    // Set discharge configuration
    raw_volt_t target = hbal.params.target + hbal.params.threshold;
    bit_flag32_t cells_to_discharge = volt_select_values(target);
    (void)bms_manager_set_discharge_cells(cells_to_discharge);

    hbal.active = true;
    return BAL_OK;
}

BalReturnCode bal_stop(void) {
    // Check actual balancing status
    if (!hbal.active)
        return BAL_OK;
 
    // Set discharge configuration
    (void)bms_manager_set_discharge_cells(0U);

    // Stop watchdog
    (void)watchdog_stop(&hbal.watchdog);
    hbal.active = false;
    return BAL_OK;
}

BalReturnCode bal_pause(void) {
    if (!hbal.active || hbal.paused)
        return BAL_OK;

    // Set discharge configuration
    (void)bms_manager_set_discharge_cells(0U);
    hbal.paused = true;
    return BAL_OK;
}

BalReturnCode bal_resume(void) {
    if (!hbal.active || !hbal.paused)
        return BAL_OK;

    // Set discharge configuration
    raw_volt_t target = hbal.params.target + hbal.params.threshold;
    bit_flag32_t cells_to_discharge = volt_select_values(target);
    (void)bms_manager_set_discharge_cells(cells_to_discharge);
    hbal.paused = false;
    return BAL_OK;
}

bms_cellboard_balancing_status_converted_t * bal_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hbal.can_payload);

    uint32_t cells = bms_manager_get_discharge_cells();
    hbal.can_payload.discharging_cell_0 = CELLBOARD_BIT_GET(cells, 0U);
    hbal.can_payload.discharging_cell_1 = CELLBOARD_BIT_GET(cells, 1U);
    hbal.can_payload.discharging_cell_2 = CELLBOARD_BIT_GET(cells, 2U);
    hbal.can_payload.discharging_cell_3 = CELLBOARD_BIT_GET(cells, 3U);
    hbal.can_payload.discharging_cell_4 = CELLBOARD_BIT_GET(cells, 4U);
    hbal.can_payload.discharging_cell_5 = CELLBOARD_BIT_GET(cells, 5U);
    hbal.can_payload.discharging_cell_6 = CELLBOARD_BIT_GET(cells, 6U);
    hbal.can_payload.discharging_cell_7 = CELLBOARD_BIT_GET(cells, 7U);
    hbal.can_payload.discharging_cell_8 = CELLBOARD_BIT_GET(cells, 8U);
    hbal.can_payload.discharging_cell_9 = CELLBOARD_BIT_GET(cells, 9U);
    hbal.can_payload.discharging_cell_10 = CELLBOARD_BIT_GET(cells, 10U);
    hbal.can_payload.discharging_cell_11 = CELLBOARD_BIT_GET(cells, 11U);
    hbal.can_payload.discharging_cell_12 = CELLBOARD_BIT_GET(cells, 12U);
    hbal.can_payload.discharging_cell_13 = CELLBOARD_BIT_GET(cells, 13U);
    hbal.can_payload.discharging_cell_14 = CELLBOARD_BIT_GET(cells, 14U);
    hbal.can_payload.discharging_cell_15 = CELLBOARD_BIT_GET(cells, 15U);
    hbal.can_payload.discharging_cell_16 = CELLBOARD_BIT_GET(cells, 16U);
    hbal.can_payload.discharging_cell_17 = CELLBOARD_BIT_GET(cells, 17U);
    hbal.can_payload.discharging_cell_18 = CELLBOARD_BIT_GET(cells, 18U);
    hbal.can_payload.discharging_cell_19 = CELLBOARD_BIT_GET(cells, 19U);
    hbal.can_payload.discharging_cell_20 = CELLBOARD_BIT_GET(cells, 20U);
    hbal.can_payload.discharging_cell_21 = CELLBOARD_BIT_GET(cells, 21U);
    hbal.can_payload.discharging_cell_22 = CELLBOARD_BIT_GET(cells, 22U);
    hbal.can_payload.discharging_cell_23 = CELLBOARD_BIT_GET(cells, 23U);
    return &hbal.can_payload;
}

#ifdef CONF_BALANCING_STRINGS_ENABLE

_STATIC char * bal_module_name = "balancing";

_STATIC char * bal_return_code_name[] = {
    [BAL_OK] = "ok",
    [BAL_NULL_POINTER] = "null pointer",
    [BAL_BUSY] = "busy",
    [BAL_WATCHDOG_ERROR] = "watchdog error"
};

_STATIC char * bal_return_code_description[] = {
    [BAL_OK] = "executed succesfully",
    [BAL_NULL_POINTER] = "attempt to dereference a null pointer",
    [BAL_BUSY] = "the target is busy",
    [BAL_WATCHDOG_ERROR] = "the internal watchdog encountered an error"
};

#endif // CONF_BALANCING_STRINGS_ENABLE

#endif // CONF_BALANCING_MODULE_ENABLE
