/*!
 * \file bal-api.c
 * \date 2024-04-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Balancing handling functions
 */

#include "bal-api.h"

#include <stdint.h>
#include <string.h>

#include "bal.h"
#include "can-bms.h"
#include "eagletrt-api.h"
#include "cellboard-def.h"
#include "timebase.h"
#include "volt-api.h"
#include "identity-api.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

EAGLETRT_STATIC struct BalHandler balancing_handler;

void prv_bal_api_timeout(void) {
    // Stop balancing
    balancing_handler.event.type = FSM_EVENT_TYPE_BALANCING_STOP;
    fsm_event_trigger(&balancing_handler.event);
}

enum BalReturnCode bal_api_init(void) {
    memset(&balancing_handler, 0U, sizeof(balancing_handler));

    // Set default event and canlib payload
    balancing_handler.event.type = FSM_EVENT_TYPE_IGNORED;

    // Set default balancing parameters
    balancing_handler.params.target = BAL_TARGET_MAX_V;
    balancing_handler.params.threshold = BAL_THRESHOLD_MAX_V;

    balancing_handler.status = BAL_STATUS_STOPPED;

    // Initialize main balancing watchdog
    (void)watchdog_init(
        &balancing_handler.watchdog,
        TIMEBASE_MS_TO_TICKS(BAL_TIMEOUT_MS, timebase_get_resolution()),
        prv_bal_api_timeout);
    return BAL_OK;
}

// TODO: Handle unavailable watchdog
void bal_api_balancing_set_handle(bool start, volt target, volt threshold) {
    // Ignore stop command if not balancing
    if (!bal_api_is_active() && !start) {
        return;
    }

    // Update data
    constexpr float min_target = BAL_TARGET_MIN_V;
    constexpr float max_target = BAL_TARGET_MAX_V;
    constexpr float min_threshold = BAL_THRESHOLD_MIN_V;
    constexpr float max_threshold = BAL_THRESHOLD_MIN_V;
    balancing_handler.params.target = EAGLETRT_API_CLAMP(target, min_target, max_target);
    balancing_handler.params.threshold = EAGLETRT_API_CLAMP(threshold, min_threshold, max_threshold);

    // Reset watchdog for each new message
    const WatchdogReturnCode code = watchdog_reset(&balancing_handler.watchdog);
    if (code != WATCHDOG_OK && code != WATCHDOG_NOT_RUNNING) {
        return;
    }

    // Send event to the FSM
    if (bal_api_is_active() == !start) {
        balancing_handler.event.type = start ? FSM_EVENT_TYPE_BALANCING_START : FSM_EVENT_TYPE_BALANCING_STOP;
        fsm_event_trigger(&balancing_handler.event);
    }
}

bool bal_api_is_active(void) {
    return balancing_handler.status != BAL_STATUS_STOPPED;
}

bool bal_api_is_paused(void) {
    return balancing_handler.status == BAL_STATUS_PAUSED;
}

enum BalReturnCode bal_api_start(void) {
    // Check actual balancing state
    if (bal_api_is_active()) {
        return BAL_OK;
    }

    // Start watchdog
    const WatchdogReturnCode code = watchdog_restart(&balancing_handler.watchdog);
    if (code != WATCHDOG_OK && code != WATCHDOG_NOT_RUNNING) {
        return BAL_WATCHDOG_ERROR;
    }

    // Set discharge configuration
    const volt target = balancing_handler.params.target + balancing_handler.params.threshold;
    const bit_flag32 cells_to_discharge = volt_api_select_values_above_target(target);
    (void)bms_manager_api_set_discharge_cells(cells_to_discharge);

    balancing_handler.status = BAL_STATUS_DISCHARCING;
    return BAL_OK;
}

enum BalReturnCode bal_api_stop(void) {
    // Check actual balancing status
    if (!bal_api_is_active()) {
        return BAL_OK;
    }

    // Set discharge configuration
    (void)bms_manager_api_set_discharge_cells(0U);

    // Stop watchdog
    (void)watchdog_stop(&balancing_handler.watchdog);
    balancing_handler.status = BAL_STATUS_STOPPED;
    return BAL_OK;
}

enum BalReturnCode bal_api_pause(void) {
    if (!bal_api_is_active() || bal_api_is_paused()) {
        return BAL_OK;
    }
    // Set discharge configuration
    (void)bms_manager_api_set_discharge_cells(0U);
    balancing_handler.status = BAL_STATUS_PAUSED;
    return BAL_OK;
}

enum BalReturnCode bal_api_resume(void) {
    if ((bal_api_is_active() && !bal_api_is_paused()) || balancing_handler.status == BAL_STATUS_STOPPED) {
        return BAL_OK;
    }
    // Set discharge configuration
    const volt target = balancing_handler.params.target + balancing_handler.params.threshold;
    const bit_flag32 cells_to_discharge = volt_api_select_values_above_target(target);
    (void)bms_manager_api_set_discharge_cells(cells_to_discharge);
    balancing_handler.status = BAL_STATUS_DISCHARCING;
    return BAL_OK;
}

union CanBmsMessages *bal_api_get_canlib_payload(size_t *byte_size) {
    enum CellboardId cellboard = identity_api_get_cellboard_id();
    uint32_t can_byte_size[] = {
        can_bms_byte_size_tsaccellboard1balancing,
        can_bms_byte_size_tsaccellboard2balancing,
        can_bms_byte_size_tsaccellboard3balancing,
        can_bms_byte_size_tsaccellboard4balancing,
        can_bms_byte_size_tsaccellboard5balancing,
        can_bms_byte_size_tsaccellboard6balancing
    };

    if (byte_size != NULL) {
        *byte_size = can_byte_size[cellboard];
    }

    const uint32_t cells = bms_manager_api_get_discharge_cells();
    union CanBmsMessages *payload = &balancing_handler.libcan_message_balancing;
    switch (cellboard) {
        case CELLBOARD_ID_0:
            payload->tsaccellboard1balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard1balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard1balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard1balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard1balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard1balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard1balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard1balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard1balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard1balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard1balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard1balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard1balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard1balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard1balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard1balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard1balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard1balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard1balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard1balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard1balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard1balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard1balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard1balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        case CELLBOARD_ID_1:
            payload->tsaccellboard2balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard2balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard2balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard2balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard2balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard2balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard2balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard2balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard2balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard2balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard2balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard2balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard2balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard2balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard2balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard2balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard2balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard2balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard2balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard2balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard2balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard2balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard2balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard2balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        case CELLBOARD_ID_2:
            payload->tsaccellboard3balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard3balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard3balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard3balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard3balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard3balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard3balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard3balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard3balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard3balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard3balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard3balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard3balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard3balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard3balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard3balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard3balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard3balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard3balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard3balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard3balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard3balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard3balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard3balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        case CELLBOARD_ID_3:
            payload->tsaccellboard4balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard4balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard4balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard4balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard4balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard4balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard4balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard4balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard4balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard4balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard4balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard4balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard4balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard4balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard4balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard4balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard4balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard4balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard4balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard4balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard4balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard4balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard4balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard4balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        case CELLBOARD_ID_4:
            payload->tsaccellboard5balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard5balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard5balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard5balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard5balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard5balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard5balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard5balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard5balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard5balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard5balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard5balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard5balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard5balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard5balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard5balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard5balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard5balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard5balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard5balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard5balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard5balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard5balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard5balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        case CELLBOARD_ID_5:
            payload->tsaccellboard6balancing.cell1 = EAGLETRT_API_BIT_GET(cells, 0U);
            payload->tsaccellboard6balancing.cell2 = EAGLETRT_API_BIT_GET(cells, 1U);
            payload->tsaccellboard6balancing.cell3 = EAGLETRT_API_BIT_GET(cells, 2U);
            payload->tsaccellboard6balancing.cell4 = EAGLETRT_API_BIT_GET(cells, 3U);
            payload->tsaccellboard6balancing.cell5 = EAGLETRT_API_BIT_GET(cells, 4U);
            payload->tsaccellboard6balancing.cell6 = EAGLETRT_API_BIT_GET(cells, 5U);
            payload->tsaccellboard6balancing.cell7 = EAGLETRT_API_BIT_GET(cells, 6U);
            payload->tsaccellboard6balancing.cell8 = EAGLETRT_API_BIT_GET(cells, 7U);
            payload->tsaccellboard6balancing.cell9 = EAGLETRT_API_BIT_GET(cells, 8U);
            payload->tsaccellboard6balancing.cell10 = EAGLETRT_API_BIT_GET(cells, 9U);
            payload->tsaccellboard6balancing.cell11 = EAGLETRT_API_BIT_GET(cells, 10U);
            payload->tsaccellboard6balancing.cell12 = EAGLETRT_API_BIT_GET(cells, 11U);
            payload->tsaccellboard6balancing.cell13 = EAGLETRT_API_BIT_GET(cells, 12U);
            payload->tsaccellboard6balancing.cell14 = EAGLETRT_API_BIT_GET(cells, 13U);
            payload->tsaccellboard6balancing.cell15 = EAGLETRT_API_BIT_GET(cells, 14U);
            payload->tsaccellboard6balancing.cell16 = EAGLETRT_API_BIT_GET(cells, 15U);
            payload->tsaccellboard6balancing.cell17 = EAGLETRT_API_BIT_GET(cells, 16U);
            payload->tsaccellboard6balancing.cell18 = EAGLETRT_API_BIT_GET(cells, 17U);
            payload->tsaccellboard6balancing.cell19 = EAGLETRT_API_BIT_GET(cells, 18U);
            payload->tsaccellboard6balancing.cell20 = EAGLETRT_API_BIT_GET(cells, 19U);
            payload->tsaccellboard6balancing.cell21 = EAGLETRT_API_BIT_GET(cells, 20U);
            payload->tsaccellboard6balancing.cell22 = EAGLETRT_API_BIT_GET(cells, 21U);
            payload->tsaccellboard6balancing.cell23 = EAGLETRT_API_BIT_GET(cells, 22U);
            payload->tsaccellboard6balancing.cell24 = EAGLETRT_API_BIT_GET(cells, 23U);
            break;
        default:
            break;
    }
    return payload;
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
