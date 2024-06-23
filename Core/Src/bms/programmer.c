/**
 * @file programmer.c
 * @date 2024-05-12
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt load the new code inside the flash memory
 */

#include "programmer.h"

#include <string.h>
#include <stdbool.h>

#include "fsm.h"
#include "identity.h"
#include "timebase.h"
#include "watchdog.h"

/** @brief The programmer flash timeout in ms */
#define PROGRAMMER_FLASH_TIMEOUT ((milliseconds_t)(30000U))

_STATIC _ProgrammerHandler hprogrammer;

/** @brief Function called when the watchdog times-out */
void _programmer_flash_timeout(void) {
    hprogrammer.flash_request = false;
    hprogrammer.flashing = false;
    hprogrammer.flash_stop = false;
}

/** @brief Function called when the flash procedure is completed */
void _programmer_flash_stop(void) {
    hprogrammer.flash_request = false;
    hprogrammer.flashing = false;
    hprogrammer.flash_stop = true;
}

/** @brief Resets all the flash flags */
void _programmer_flash_reset_flags(void) { 
    hprogrammer.flash_request = false;
    hprogrammer.flashing = false;
    hprogrammer.flash_stop = false;
}

ProgrammerReturnCode programmer_init(system_reset_callback_t reset) {
    memset(&hprogrammer, 0U, sizeof(hprogrammer));

    hprogrammer.reset = reset;
    hprogrammer.flash_event.type = FSM_EVENT_TYPE_FLASH_REQUEST;
    hprogrammer.can_payload.cellboard_id = identity_get_cellboard_id();    
    hprogrammer.can_payload.ready = true;

    // Reset flash procedure data
    hprogrammer.target = MAINBOARD_ID;
    _programmer_flash_reset_flags();

    // Initialize watchdogs
    (void)watchdog_init(
        &hprogrammer.watchdog,
        TIMEBASE_TIME_TO_TICKS(PROGRAMMER_FLASH_TIMEOUT, timebase_get_resolution()),
        _programmer_flash_timeout
    );

    return PROGRAMMER_OK;
}

void programmer_flash_request_handle(bms_cellboard_flash_request_converted_t * payload) {
    if (payload == NULL)
        return;
    if (hprogrammer.flash_request)
        return;
    fsm_state_t status = fsm_get_status();
    if (status != FSM_STATE_IDLE && status != FSM_STATE_FATAL)
        return;

    // TODO: Check the payload content

    hprogrammer.target = payload->mainboard ? MAINBOARD_ID : payload->cellboard_id;
    hprogrammer.flash_request = true;
    hprogrammer.flash_stop = false;
    hprogrammer.flashing = false;

    watchdog_start(&hprogrammer.watchdog);

    // Trigger event
    fsm_event_trigger(&hprogrammer.flash_event);
}

void programmer_flash_handle(bms_cellboard_flash_converted_t * payload) {
    if (payload == NULL)
        return;
    if (payload->start == hprogrammer.flashing)
        return;
    if (fsm_get_status() != FSM_STATE_FLASH || !hprogrammer.flash_request)
        return;

    if (payload->start) {
        watchdog_reset(&hprogrammer.watchdog);
        hprogrammer.flashing = true;
    }
    else {
        watchdog_stop(&hprogrammer.watchdog);
        _programmer_flash_stop();
    }
}

ProgrammerReturnCode programmer_routine(void) {
    if (watchdog_is_timed_out(&hprogrammer.watchdog))
        return PROGRAMMER_TIMEOUT;
    if (hprogrammer.flash_stop)
        return PROGRAMMER_OK;

    // Reset the microcontroller if the current cellboard is the target
    if (hprogrammer.flashing && identity_get_cellboard_id() == hprogrammer.target)
        hprogrammer.reset();

    return PROGRAMMER_BUSY;
}

