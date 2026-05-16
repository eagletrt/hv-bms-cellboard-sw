/*!
 * \file programmer-api.h
 * \date 2024-05-12
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Functions used during the flash procedure where the microcontroller
 * is reset and openblt loads the new code inside the flash memory
 */

#ifndef PROGRAMMER_API_H
#define PROGRAMMER_API_H

#include "programmer.h"
#include <stdint.h>

/*!
 * \brief Intialize the internal programmer handler structure
 *
 * \param reset A pointer to the function that resets the microcontroller
 *
 * \retval PROGRAMMER_RC_OK if the initialization is successful
 */
enum ProgrammerReturnCode programmer_api_init(system_reset_callback_t reset);

/*!
 * \brief Handle the received flash request
 *
 * \param payload A pointer to the canlib payload of the request
 *
 * \retval PROGRAMMER_RC_OK if the request is handled successfully
 * \retval -PROGRAMMER_RC_ERROR if the flash request procedure cannot be started
 * \retval -PROGRAMMER_RC_NULL_POINTER if the payload is a null pointer
 */
int32_t programmer_api_flash_request_handle(const bms_cellboard_flash_request_converted_t *payload);

/*!
 * \brief Handle the received actual flash command
 *
 * \param payload A pointer to the canlib payload of the command
 *
 * \retval PROGRAMMER_RC_OK if the command is handled successfully
 * \retval -PROGRAMMER_RC_ERROR if the flash procedure cannot be started
 * \retval -PROGRAMMER_RC_NULL_POINTER if the payload is a null pointer
 */
int32_t programmer_api_flash_handle(const bms_cellboard_flash_converted_t *payload);

/*!
 * \brief Routine that should be called during the flash procedure
 *
 * \attention This function can reset the microcontroller
 *
 * \retval PROGRAMMER_RC_TIMEOUT if the flash procedure has timed out
 * \retval PROGRAMMER_RC_OK if the flash procedure has finished
 * \retval PROGRAMMER_RC_BUSY otherwise
 */
enum ProgrammerReturnCode programmer_api_routine(void);

#endif // PROGRAMMER_API_H
