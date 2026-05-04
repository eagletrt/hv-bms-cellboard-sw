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

#include "bms_network.h"

#include "fsm.h"
#include "watchdog.h"

/*! \brief The programmer flash timeout in ms */
#define PROGRAMMER_FLASH_TIMEOUT_MS (1000U)

/*!
 * \brief Return code for the programmer module functions 
 */
typedef enum {
    PROGRAMMER_RC_OK,     /*!< The function executed successfully */
    PROGRAMMER_RC_BUSY,   /*!< The flash procedure is still ongoing, the function should be called again */
    PROGRAMMER_RC_TIMEOUT /*!< The flash procedure has timed out */
} ProgrammerReturnCode;

/*!
 * \brief Type definition for the programmer handler structure
 *
 * \attention This structure should not be used outside of this module
 *
 */
typedef struct {
    system_reset_callback_t reset;                        /*!< A pointer to a function that resets the microcontroller */
    fsm_event_data_t flash_event;                         /*!< The FSM event data related to the flash procedure */
    bms_cellboard_flash_response_converted_t can_payload; /*!< The flash response canlib data */

    enum CellboardId target; /*!< The identifier of the cellboard(or mainboard) to flash */
    bool flash_request;      /*!< True if a flash request is received, false otherwise */
    bool flashing;           /*!< True if the cellboard is flashing, false otherwise */
    bool flash_stop;         /*!< True if the flash procedure should be stopped, false otherwise */

    Watchdog watchdog; /*!< The watchdog used for the flash procedure */
} _ProgrammerHandler;

/*!
 * \brief Intialize the internal programmer handler structure
 *
 * \param reset A pointer to the function that resets the microcontroller
 *
 * \retval PROGRAMMER_RC_OK if the initialization is successful
 */
ProgrammerReturnCode programmer_init(const system_reset_callback_t reset);

/*!
 * \brief Handle the received flash request
 *
 * \param payload A pointer to the canlib payload of the request
 */
void programmer_flash_request_handle(const bms_cellboard_flash_request_converted_t *const payload);

/*!
 * \brief Handle the received actual flash command
 *
 * \param payload A pointer to the canlib payload of the command
 */
void programmer_flash_handle(const bms_cellboard_flash_converted_t *const payload);

/*!
 * \brief Routine that should be called during the flash procedure
 *
 * \attention This function can reset the microcontroller
 *
 * \retval PROGRAMMER_RC_TIMEOUT if the flash procedure has timed out
 * \retval PROGRAMMER_RC_OK if the flash procedure has finished
 * \retval PROGRAMMER_RC_BUSY otherwise
 */
ProgrammerReturnCode programmer_routine(void);

#endif // PROGRAMMER_H
