/**
 * @file programmer.c
 * @date 2024-05-12
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt load the new code inside the flash memory
 */

#include "programmer.h"
#include "fsm.h"
#include "identity.h"

struct {
    fsm_event_data_t flash_event;
    bms_cellboard_flash_response_converted_t can_payload;

    CellboardId target;
} hprogrammer;

ProgrammerReturnCode programmer_init(void) {
    hprogrammer.flash_event.type = FSM_EVENT_TYPE_FLASH_REQUEST;
    hprogrammer.can_payload.cellboard_id = identity_get_cellboard_id();    
    hprogrammer.can_payload.ready = true;
}

void programmer_flash_request_handle(bms_cellboard_flash_request_converted_t * payload) {
    if (payload == NULL)
        return;

    // Copy the target
    hprogrammer.target = payload->mainboard ? MAINBOARD_ID : payload->cellboard_id;
    hprogrammer.flash_event.type = FSM_EVENT_TYPE_FLASH_REQUEST;

    // Trigger event
    fsm_event_trigger(&hprogrammer.flash_event);
}

void programmer_flash_handle(bms_cellboard_flash_converted_t * payload) {
    if (payload == NULL)
        return;

    hprogrammer.flash_event.type = FSM_EVENT_TYPE_FLASH;
    fsm_event_trigger(&hprogrammer.flash_event);
}

ProgrammerReturnCode programmer_routine(void) {

}

