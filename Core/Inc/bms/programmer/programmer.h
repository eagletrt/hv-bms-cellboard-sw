/*!
 * \file programmer.h
 * \date 2024-05-12
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt loads the new code inside the flash memory
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include "cellboard-def.h"

#include "fsm.h"
#include "watchdog.h"

/*! \brief The programmer flash timeout in ms */
#define PROGRAMMER_FLASH_TIMEOUT_MS (1000U)

/*!
 * \brief Return code for the programmer module functions
 */
enum ProgrammerReturnCode {
    PROGRAMMER_RC_OK,           /*!< The function executed successfully */
    PROGRAMMER_RC_BUSY,         /*!< The flash procedure is still ongoing, the function should be called again */
    PROGRAMMER_RC_TIMEOUT,      /*!< The flash procedure has timed out */
    PROGRAMMER_RC_NULL_POINTER, /*!< The function recieved a null pointer*/
    PROGRAMMER_RC_ERROR         /*!< An error occurred during the execution of the function */
};

/*!
 * \brief Type definition for the programmer handler structure
 *
 * \attention This structure should not be used outside of this module
 *
 */
struct ProgrammerHandler {
    system_reset_callback reset;  /*!< A pointer to a function that resets the microcontroller */
    fsm_event_data_t flash_event; /*!< The FSM event data related to the flash procedure */
    // TODO: update libcan
    // bms_cellboard_flash_response_converted_t can_payload; /*!< The flash response canlib data */

    enum CellboardId target; /*!< The identifier of the cellboard(or mainboard) to flash */
    bool flash_request;      /*!< True if a flash request is received, false otherwise */
    bool flashing;           /*!< True if the cellboard is flashing, false otherwise */
    bool flash_stop;         /*!< True if the flash procedure should be stopped, false otherwise */

    Watchdog watchdog; /*!< The watchdog used for the flash procedure */
};

#endif // PROGRAMMER_H
