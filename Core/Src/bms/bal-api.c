/*!
 * \file bal.c
 * \date 2024-04-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Balancing handling functions
 */

#include "bal-api.h"

#include <string.h>

#include "cellboard-def.h"
#include "post.h"
#include "timebase.h"
#include "volt-api.h"
#include "identity-api.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

EAGLETRT_STATIC struct BalHandler balancing_handler;

void prv_bal_timeout(void) {
    // Stop balancing
    balancing_handler.event.type = FSM_EVENT_TYPE_BALANCING_STOP;
    fsm_event_trigger(&balancing_handler.event);
}

enum BalReturnCode bal_init(void) {
    memset(&balancing_handler, 0U, sizeof(balancing_handler));

    // Set default event and canlib payload
    balancing_handler.event.type = FSM_EVENT_TYPE_IGNORED;

    balancing_handler.status_can_payload.cellboard_id = (bms_cellboard_balancing_status_cellboard_id)identity_api_get_cellboard_id();

    // Set default balancing parameters
    balancing_handler.params.target = BAL_TARGET_MAX_V;
    balancing_handler.params.threshold = BAL_THRESHOLD_MAX_V;

    balancing_handler.status = BAL_STATUS_STOPPED;

    // Initialize main balancing watchdog
    (void)watchdog_init(
        &balancing_handler.watchdog,
        TIMEBASE_MS_TO_TICKS(BAL_TIMEOUT_MS, timebase_get_resolution()),
        prv_bal_timeout);
    return BAL_OK;
}

// TODO: Handle unavailable watchdog
enum BalReturnCode bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t *const payload) {
    if (payload == NULL) {
        return BAL_NULL_POINTER;
    }
    // Ignore stop command if not balancing
    if (!bal_is_active() && !payload->start) {
        return BAL_OK;
    }

    // Update data
    const volt target = payload->target;
    const volt threshold = payload->threshold;
    balancing_handler.params.target = EAGLETRT_API_CLAMP(target, BAL_TARGET_MIN_V, BAL_TARGET_MAX_V);             // NOLINT(readability-magic-numbers)
    balancing_handler.params.threshold = EAGLETRT_API_CLAMP(threshold, BAL_THRESHOLD_MIN_V, BAL_THRESHOLD_MAX_V); // NOLINT(readability-magic-numbers)

    // Reset watchdog for each new message
    const WatchdogReturnCode code = watchdog_reset(&balancing_handler.watchdog);
    if (code != WATCHDOG_OK && code != WATCHDOG_NOT_RUNNING) {
        return BAL_WATCHDOG_ERROR;
    }

    // Send event to the FSM
    if (bal_is_active() == !payload->start) {
        balancing_handler.event.type = payload->start ? FSM_EVENT_TYPE_BALANCING_START : FSM_EVENT_TYPE_BALANCING_STOP;
        fsm_event_trigger(&balancing_handler.event);
    }
    return BAL_OK;
}

bool bal_is_active(void) {
    return balancing_handler.status != BAL_STATUS_STOPPED;
}

bool bal_is_paused(void) {
    return balancing_handler.status == BAL_STATUS_PAUSED;
}

enum BalReturnCode bal_start(void) {
    // Check actual balancing state
    if (bal_is_active()) {
        return BAL_OK;
    }

    // Start watchdog
    const WatchdogReturnCode code = watchdog_restart(&balancing_handler.watchdog);
    if (code != WATCHDOG_OK && code != WATCHDOG_NOT_RUNNING) {
        return BAL_WATCHDOG_ERROR;
    }

    // Set discharge configuration
    const volt target = balancing_handler.params.target + balancing_handler.params.threshold;
    const bit_flag32 cells_to_discharge = volt_select_values_above_target(target);
    (void)bms_manager_set_discharge_cells(cells_to_discharge);

    balancing_handler.status = BAL_STATUS_DISCHARCING;
    return BAL_OK;
}

enum BalReturnCode bal_stop(void) {
    // Check actual balancing status
    if (!bal_is_active()) {
        return BAL_OK;
    }

    // Set discharge configuration
    (void)bms_manager_set_discharge_cells(0U);

    // Stop watchdog
    (void)watchdog_stop(&balancing_handler.watchdog);
    balancing_handler.status = BAL_STATUS_STOPPED;
    return BAL_OK;
}

enum BalReturnCode bal_pause(void) {
    if (!bal_is_active() || bal_is_paused()) {
        return BAL_OK;
    }
    // Set discharge configuration
    (void)bms_manager_set_discharge_cells(0U);
    balancing_handler.status = BAL_STATUS_PAUSED;
    return BAL_OK;
}

enum BalReturnCode bal_resume(void) {
    if ((bal_is_active() && !bal_is_paused()) || balancing_handler.status == BAL_STATUS_STOPPED) {
        return BAL_OK;
    }
    // Set discharge configuration
    const volt target = balancing_handler.params.target + balancing_handler.params.threshold;
    const bit_flag32 cells_to_discharge = volt_select_values_above_target(target);
    (void)bms_manager_set_discharge_cells(cells_to_discharge);
    balancing_handler.status = BAL_STATUS_DISCHARCING;
    return BAL_OK;
}

bms_cellboard_balancing_status_converted_t *bal_get_status_canlib_payload(size_t *const byte_size) {
    if (byte_size != NULL) {
        *byte_size = sizeof(balancing_handler.status_can_payload);
    }
    balancing_handler.status_can_payload.status = bms_cellboard_balancing_status_status_stopped;

    // Update balancing status
    if (bal_is_active()) {
        balancing_handler.status_can_payload.status = bal_is_paused() ? bms_cellboard_balancing_status_status_paused : bms_cellboard_balancing_status_status_running;
    }
    // Update discharging cells
    const uint32_t cells = bms_manager_get_discharge_cells();
    balancing_handler.status_can_payload.discharging_cell_0 = EAGLETRT_API_BIT_GET(cells, 0U);
    balancing_handler.status_can_payload.discharging_cell_1 = EAGLETRT_API_BIT_GET(cells, 1U);
    balancing_handler.status_can_payload.discharging_cell_2 = EAGLETRT_API_BIT_GET(cells, 2U);
    balancing_handler.status_can_payload.discharging_cell_3 = EAGLETRT_API_BIT_GET(cells, 3U);
    balancing_handler.status_can_payload.discharging_cell_4 = EAGLETRT_API_BIT_GET(cells, 4U);
    balancing_handler.status_can_payload.discharging_cell_5 = EAGLETRT_API_BIT_GET(cells, 5U);
    balancing_handler.status_can_payload.discharging_cell_6 = EAGLETRT_API_BIT_GET(cells, 6U);
    balancing_handler.status_can_payload.discharging_cell_7 = EAGLETRT_API_BIT_GET(cells, 7U);
    balancing_handler.status_can_payload.discharging_cell_8 = EAGLETRT_API_BIT_GET(cells, 8U);
    balancing_handler.status_can_payload.discharging_cell_9 = EAGLETRT_API_BIT_GET(cells, 9U);
    balancing_handler.status_can_payload.discharging_cell_10 = EAGLETRT_API_BIT_GET(cells, 10U);
    balancing_handler.status_can_payload.discharging_cell_11 = EAGLETRT_API_BIT_GET(cells, 11U);
    balancing_handler.status_can_payload.discharging_cell_12 = EAGLETRT_API_BIT_GET(cells, 12U);
    balancing_handler.status_can_payload.discharging_cell_13 = EAGLETRT_API_BIT_GET(cells, 13U);
    balancing_handler.status_can_payload.discharging_cell_14 = EAGLETRT_API_BIT_GET(cells, 14U);
    balancing_handler.status_can_payload.discharging_cell_15 = EAGLETRT_API_BIT_GET(cells, 15U);
    balancing_handler.status_can_payload.discharging_cell_16 = EAGLETRT_API_BIT_GET(cells, 16U);
    balancing_handler.status_can_payload.discharging_cell_17 = EAGLETRT_API_BIT_GET(cells, 17U);
    balancing_handler.status_can_payload.discharging_cell_18 = EAGLETRT_API_BIT_GET(cells, 18U);
    balancing_handler.status_can_payload.discharging_cell_19 = EAGLETRT_API_BIT_GET(cells, 19U);
    balancing_handler.status_can_payload.discharging_cell_20 = EAGLETRT_API_BIT_GET(cells, 20U);
    balancing_handler.status_can_payload.discharging_cell_21 = EAGLETRT_API_BIT_GET(cells, 21U);
    balancing_handler.status_can_payload.discharging_cell_22 = EAGLETRT_API_BIT_GET(cells, 22U);
    balancing_handler.status_can_payload.discharging_cell_23 = EAGLETRT_API_BIT_GET(cells, 23U);
    return &balancing_handler.status_can_payload;
}

#ifdef CONF_BALANCING_STRINGS_ENABLE

EAGLETRT_STATIC char *bal_module_name = "balancing";

EAGLETRT_STATIC char *bal_return_code_name[] = {
    [BAL_OK] = "ok",
    [BAL_NULL_POINTER] = "null pointer",
    [BAL_BUSY] = "busy",
    [BAL_WATCHDOG_ERROR] = "watchdog error"
};

EAGLETRT_STATIC char *bal_return_code_description[] = {
    [BAL_OK] = "executed succesfully",
    [BAL_NULL_POINTER] = "attempt to dereference a null pointer",
    [BAL_BUSY] = "the target is busy",
    [BAL_WATCHDOG_ERROR] = "the internal watchdog encountered an error"
};

#endif // CONF_BALANCING_STRINGS_ENABLE

#endif // CONF_BALANCING_MODULE_ENABLE
