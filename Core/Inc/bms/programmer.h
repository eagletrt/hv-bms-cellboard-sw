/**
 * @file programmer.h
 * @date 2024-05-12
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt load the new code inside the flash memory
 */

#ifndef PROGRAMMER_H
#define PROGRAMMER_H

#include "bms_network.h"
#include "cellboard-def.h"

/**
 * @brief Return code for the programmer module functions 
 *
 * @details
 *     - PROGRAMMER_OK the function executed succesfully
 *     - PROGRAMMER_BUSY the flash procedure is ongoing
 *     - PROGRAMMER_TIMEOUT the procedure has timed out
 */
typedef enum {
    PROGRAMMER_OK,
    PROGRAMMER_BUSY,
    PROGRAMMER_TIMEOUT
} ProgrammerReturnCode;

/**
 * @brief Intialize the internal programmer handler structure
 *
 * @param reset A pointer to the function that resets the microcontroller
 *
 * @return ProgrammerReturnCode
 *     - PROGRAMMER_OK
 */
ProgrammerReturnCode programmer_init(system_reset_callback_t reset);

/**
 * @brief Handle the received flash request
 *
 * @param payload A pointer to the canlib payload of the request
 */
void programmer_flash_request_handle(bms_cellboard_flash_request_converted_t * payload);

/**
 * @brief Handle the received actual flash command
 *
 * @param payload A pointer to the canlib payload of the command
 */
void programmer_flash_handle(bms_cellboard_flash_converted_t * payload);

/**
 * @brief Routine that should be called during the flash procedure
 *
 * @attention This function can reset the microcontroller
 *
 * @return ProgrammerReturnCode
 *     - PROGRAMMER_TIMEOUT if the flash procedure has timed out
 *     - PROGRAMMER_OK if the flash procedure has finished
 *     - PROGRAMMER_BUSY otherwise
 */
ProgrammerReturnCode programmer_routine(void);

#endif  // PROGRAMMER_H
