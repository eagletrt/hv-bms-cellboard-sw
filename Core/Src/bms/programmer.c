/**
 * @file programmer.c
 * @date 2024-05-12
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt load the new code inside the flash memory
 */

#include "programmer.h"

#include <stdbool.h>

#include "fsm.h"
#include "identity.h"
#include "timebase.h"
#include "watchdog.h"

/** @brief The programmer flash timeout in ms */
#define PROGRAMMER_FLASH_TIMEOUT ((time_t)30000U)

static struct {
    system_reset_callback_t reset;
    fsm_event_t flash_event;
    bms_cellboard_flash_response_converted_t can_payload;

    CellboardId target;
    bool flashing;

    Watchdog watchdog;
    bool timeout;
} hprogrammer;

void _programmer_flash_timeout(void) {
    hprogrammer.timeout = true;
}

ProgrammerReturnCode programmer_init(system_reset_callback_t reset) {
    hprogrammer.reset = reset;
    hprogrammer.flash_event.type = FSM_EVENT_TYPE_FLASH_REQUEST;
    hprogrammer.can_payload.cellboard_id = identity_get_cellboard_id();    
    hprogrammer.can_payload.ready = true;
    hprogrammer.flashing = false;

    // Initialize watchdogs
    watchdog_init(
        &hprogrammer.watchdog,
        TIMEBASE_TIME_TO_TICKS(PROGRAMMER_FLASH_TIMEOUT, timebase_get_resolution()),
        _programmer_flash_timeout
    );
    hprogrammer.timeout = false;
}

void programmer_flash_request_handle(bms_cellboard_flash_request_converted_t * payload) {
    if (payload == NULL)
        return;

    // TODO: Check the payload content

    // Copy the target
    hprogrammer.target = payload->mainboard ? MAINBOARD_ID : payload->cellboard_id;

    // Trigger event
    fsm_event_trigger(&hprogrammer.flash_event);
}

void programmer_flash_handle(bms_cellboard_flash_converted_t * payload) {
    if (payload == NULL)
        return;
    
    // Start flash procedure and watchdog
    hprogrammer.timeout = !payload->start;
    payload->start ? watchdog_start(&hprogrammer.watchdog) : watchdog_stop(&hprogrammer.watchdog);
    hprogrammer.flashing = payload->start;
}

ProgrammerReturnCode programmer_routine(void) {
    if (hprogrammer.timeout)
        return PROGRAMMER_TIMEOUT;

    // Reset the microcontroller if the current cellboard is the target
    if (hprogrammer.flashing && identity_get_cellboard_id() == hprogrammer.target)
        hprogrammer.reset();

    return PROGRAMMER_OK;
}

