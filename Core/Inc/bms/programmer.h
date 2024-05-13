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

typedef enum {
    PROGRAMMER_OK
} ProgrammerReturnCode;

/**
 * @brief Intialize the internal programmer handler structure
 *
 * @return ProgrammerReturnCode
 *     PROGRAMMER_OK
 */
ProgrammerReturnCode programmer_init(void);

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
 * TODO:
 */
ProgrammerReturnCode programmer_routine(void);

#endif  // PROGRAMMER_H
