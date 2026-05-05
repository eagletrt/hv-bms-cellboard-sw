/*!
 * \file programmer-api.c
 * \date 2024-05-12
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt load the new code inside the flash memory
 */

#include "programmer.h"

#include <string.h>
#include <stdbool.h>

#include "identity-api.h"
#include "timebase.h"

EAGLETRT_STATIC struct ProgrammerHandler programmer_handler;

/*! \brief Function called when the watchdog times-out */
void prv_programmer_flash_timeout(void) {
    programmer_handler.flash_request = false;
    programmer_handler.flashing = false;
    programmer_handler.flash_stop = false;
}

/*! \brief Function called when the flash procedure is completed */
void prv_programmer_flash_stop(void) {
    programmer_handler.flash_request = false;
    programmer_handler.flashing = false;
    programmer_handler.flash_stop = true;
}

/*! \brief Resets all the flash flags */
void prv_programmer_flash_reset_flags(void) {
    programmer_handler.flash_request = false;
    programmer_handler.flashing = false;
    programmer_handler.flash_stop = false;
}

enum ProgrammerReturnCode programmer_init(const system_reset_callback_t reset) {

    if (reset == NULL) {
        return PROGRAMMER_RC_NULL_POINTER;
    }

    memset(&programmer_handler, 0U, sizeof(programmer_handler));

    programmer_handler.reset = reset;
    programmer_handler.flash_event.type = FSM_EVENT_TYPE_FLASH_REQUEST;
    programmer_handler.can_payload.cellboard_id = (bms_cellboard_flash_response_cellboard_id)identity_api_get_cellboard_id();
    programmer_handler.can_payload.ready = true;

    // Reset flash procedure data
    programmer_handler.target = MAINBOARD_ID;
    prv_programmer_flash_reset_flags();

    // TODO: Watchdog for the entire procedure?
    // Initialize watchdogs
    (void)watchdog_init(
        &programmer_handler.watchdog,
        TIMEBASE_MS_TO_TICKS(PROGRAMMER_FLASH_TIMEOUT_MS, timebase_get_resolution()),
        prv_programmer_flash_timeout);

    return PROGRAMMER_RC_OK;
}

enum ProgrammerReturnCode programmer_flash_request_handle(const bms_cellboard_flash_request_converted_t *const payload) {
    if (payload == NULL) {
        return PROGRAMMER_RC_NULL_POINTER;
    }
    if (programmer_handler.flash_request) {
        return PROGRAMMER_RC_BUSY;
    }
    const fsm_state_t status = fsm_get_status();
    if (status != FSM_STATE_IDLE && status != FSM_STATE_FATAL) {
        return PROGRAMMER_RC_ERROR;
    }
    if (payload->cellboard_id >= CELLBOARD_ID_COUNT && !payload->mainboard) {
        return PROGRAMMER_RC_ERROR;
    }
    programmer_handler.target = payload->mainboard ? MAINBOARD_ID : (enum CellboardId)payload->cellboard_id;
    programmer_handler.flash_request = true;
    programmer_handler.flash_stop = false;
    programmer_handler.flashing = false;

    watchdog_restart(&programmer_handler.watchdog);

    // Trigger event
    fsm_event_trigger(&programmer_handler.flash_event);
    return PROGRAMMER_RC_OK;
}

enum ProgrammerReturnCode programmer_flash_handle(const bms_cellboard_flash_converted_t *const payload) {
    if (payload == NULL) {
        return PROGRAMMER_RC_NULL_POINTER;
    }
    if (payload->start == programmer_handler.flashing) {
        return PROGRAMMER_RC_OK; // No change, just ignore the message
    }
    if (fsm_get_status() != FSM_STATE_FLASH || !programmer_handler.flash_request) {
        return PROGRAMMER_RC_ERROR;
    }
    if (payload->start) {
        watchdog_reset(&programmer_handler.watchdog);
        programmer_handler.flashing = true;
    } else {
        watchdog_stop(&programmer_handler.watchdog);
        prv_programmer_flash_stop();
    }
    return PROGRAMMER_RC_OK;
}

enum ProgrammerReturnCode programmer_routine(void) {
    if (watchdog_is_timed_out(&programmer_handler.watchdog)) {
        return PROGRAMMER_RC_TIMEOUT;
    }
    if (programmer_handler.flash_stop) {
        return PROGRAMMER_RC_OK;
    }
    // Reset the microcontroller if the current cellboard is the target
    if (programmer_handler.flashing && identity_api_get_cellboard_id() == programmer_handler.target) {
        programmer_handler.reset();
    }

    return PROGRAMMER_RC_BUSY;
}
