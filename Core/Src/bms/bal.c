/**
 * @file bal.c @date 2024-04-17
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Balancing handling functions
 */

#include "bal.h"

#include <string.h>

#include "cellboard-def.h"
#include "post.h"
#include "timebase.h"
#include "volt.h"
#include "identity.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

_STATIC _BalHandler hbal;

void _bal_timeout(void) {
    // Stop balancing
    hbal.event.type = FSM_EVENT_TYPE_BALANCING_STOP;
    fsm_event_trigger(&hbal.event);
}

BalReturnCode bal_init(void) {
    memset(&hbal, 0U, sizeof(hbal));

    // Set default event and canlib payload
    hbal.event.type = FSM_EVENT_TYPE_IGNORED;

    hbal.status_can_payload.cellboard_id = (bms_cellboard_balancing_status_cellboard_id)identity_get_cellboard_id();

    // Set default balancing parameters
    hbal.params.target = BAL_TARGET_MAX_V;
    hbal.params.threshold = BAL_THRESHOLD_MAX_V;

    // Initialize main balancing watchdog
    (void)watchdog_init(
        &hbal.watchdog,
        TIMEBASE_MS_TO_TICKS(BAL_TIMEOUT_MS, timebase_get_resolution()),
        _bal_timeout
    );
    return BAL_OK;
}

// TODO: Handle unavailable watchdog
void bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t * const payload) {
    if (payload == NULL)
        return;
    // Ignore stop command if not balancing
    if (!hbal.active && !payload->start)
        return;
 
    // Update data
    const volt_t target = payload->target;
    const volt_t threshold = payload->threshold;
    hbal.params.target = CELLBOARD_CLAMP(target, BAL_TARGET_MIN_V, BAL_TARGET_MAX_V);
    hbal.params.threshold = CELLBOARD_CLAMP(threshold, BAL_THRESHOLD_MIN_V, BAL_THRESHOLD_MAX_V);

    // Reset watchdog for each new message
    const WatchdogReturnCode code = watchdog_reset(&hbal.watchdog);
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
    const WatchdogReturnCode code = watchdog_restart(&hbal.watchdog);
    if (code == WATCHDOG_UNAVAILABLE)
        return BAL_WATCHDOG_ERROR;

    // Set discharge configuration
    const volt_t target = hbal.params.target + hbal.params.threshold;
    const bit_flag32_t cells_to_discharge = volt_select_values(target);
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
    const volt_t target = hbal.params.target + hbal.params.threshold;
    const bit_flag32_t cells_to_discharge = volt_select_values(target);
    (void)bms_manager_set_discharge_cells(cells_to_discharge);
    hbal.paused = false;
    return BAL_OK;
}

bms_cellboard_balancing_status_converted_t * bal_get_status_canlib_payload(size_t * const byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hbal.status_can_payload);

    // Update balancing status
    hbal.status_can_payload.status = bms_cellboard_balancing_status_status_stopped;
    if (bal_is_active()) {
        hbal.status_can_payload.status = bal_is_paused() ?
            bms_cellboard_balancing_status_status_paused :
            bms_cellboard_balancing_status_status_running;
    }

    // Update discharging cells
    const uint32_t cells = bms_manager_get_discharge_cells();
    hbal.status_can_payload.discharging_cell_0 = CELLBOARD_BIT_GET(cells, 0U);
    hbal.status_can_payload.discharging_cell_1 = CELLBOARD_BIT_GET(cells, 1U);
    hbal.status_can_payload.discharging_cell_2 = CELLBOARD_BIT_GET(cells, 2U);
    hbal.status_can_payload.discharging_cell_3 = CELLBOARD_BIT_GET(cells, 3U);
    hbal.status_can_payload.discharging_cell_4 = CELLBOARD_BIT_GET(cells, 4U);
    hbal.status_can_payload.discharging_cell_5 = CELLBOARD_BIT_GET(cells, 5U);
    hbal.status_can_payload.discharging_cell_6 = CELLBOARD_BIT_GET(cells, 6U);
    hbal.status_can_payload.discharging_cell_7 = CELLBOARD_BIT_GET(cells, 7U);
    hbal.status_can_payload.discharging_cell_8 = CELLBOARD_BIT_GET(cells, 8U);
    hbal.status_can_payload.discharging_cell_9 = CELLBOARD_BIT_GET(cells, 9U);
    hbal.status_can_payload.discharging_cell_10 = CELLBOARD_BIT_GET(cells, 10U);
    hbal.status_can_payload.discharging_cell_11 = CELLBOARD_BIT_GET(cells, 11U);
    hbal.status_can_payload.discharging_cell_12 = CELLBOARD_BIT_GET(cells, 12U);
    hbal.status_can_payload.discharging_cell_13 = CELLBOARD_BIT_GET(cells, 13U);
    hbal.status_can_payload.discharging_cell_14 = CELLBOARD_BIT_GET(cells, 14U);
    hbal.status_can_payload.discharging_cell_15 = CELLBOARD_BIT_GET(cells, 15U);
    hbal.status_can_payload.discharging_cell_16 = CELLBOARD_BIT_GET(cells, 16U);
    hbal.status_can_payload.discharging_cell_17 = CELLBOARD_BIT_GET(cells, 17U);
    hbal.status_can_payload.discharging_cell_18 = CELLBOARD_BIT_GET(cells, 18U);
    hbal.status_can_payload.discharging_cell_19 = CELLBOARD_BIT_GET(cells, 19U);
    hbal.status_can_payload.discharging_cell_20 = CELLBOARD_BIT_GET(cells, 20U);
    hbal.status_can_payload.discharging_cell_21 = CELLBOARD_BIT_GET(cells, 21U);
    hbal.status_can_payload.discharging_cell_22 = CELLBOARD_BIT_GET(cells, 22U);
    hbal.status_can_payload.discharging_cell_23 = CELLBOARD_BIT_GET(cells, 23U);
    return &hbal.status_can_payload;
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
