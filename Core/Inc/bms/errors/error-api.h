/*!
 * \file error-api.h
 * \date 2024-08-24
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Simple wrapper for the error handler generated code
 */

#ifndef ERROR_API_H
#define ERROR_API_H

#include "error.h"

#ifdef CONF_ERROR_MODULE_ENABLE

/*!
 * \brief Initialization of the internal error handler structure
 * \param reset A callback function that performs a system reset, used for critical errors. It is used for can communication errors to reset the cellboard and try to recover from the error.
 *
 * \retval ERROR_RC_OK if the initialization function correctly executed
 * \retval ERROR_RC_UNKNOWN if the errorlib returned error
 * \retval ERROR_RC_NULL_POINTER if reset is not a valid function pointer
 */
enum ErrorReturnCode error_api_init(system_reset_callback reset);

/*!
 * \brief Increments the error counter on a specific instance of an error group.
 * If an error expires the function fills the payload with the given error and enables the error-sending task.
 *
 * \bug Consecutive errors overwrite the payload, queue implementation has to be taken into consideration.
 *
 * \attention If the can error group is given as input to this function it will reset the micro.
 *
 * \param group The group of the error to be incremented
 * \param instance The instance of the error to be incremented
 *
 * \retval ERROR_RC_UNKNOWN The errorlib returned error
 * \retval ERROR_RC_OK The function executed correctly
 */
enum ErrorReturnCode error_api_set(enum ErrorGroup group, error_instance_t instance);

/*!
 * \brief Resets the error counter for a specific instance of an error group.
 *
 * \param group The group of the error to be reset
 * \param instance The instance of the error to be reset
 *
 * \retval ERROR_RC_UNKNOWN The errorlib returned error
 * \retval ERROR_RC_OK The function executed correctly
 */
enum ErrorReturnCode error_api_reset(enum ErrorGroup group, error_instance_t instance);

/*!
 * \brief Get the number of expired errors
 *
 * \return size_t The number of expired errors
 */
size_t error_api_get_expired(void);

/*!
 * \brief Get the information of the first expired error
 *
 * \return ErrorInfo The information of the first expired error
 */
ErrorInfo error_api_get_expired_info(void);

// TODO: update libcan
// /*!
//  * \brief Get a pointer to the CAN payload of cellboard error
//  *
//  * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
//  *
//  * \return bms_cellboard_error_converted_t*  A pointer to the payload
//  */
// bms_cellboard_error_converted_t *error_api_get_error_canlib_payload(size_t *byte_size);

#else // CONF_ERROR_MODULE_ENABLE

#define error_api_init() (ERROR_RC_OK)
#define error_api_set(group, instance) (ERROR_RC_OK)
#define error_api_reset(group, instance) (ERROR_RC_OK)
#define error_api_get_expired() (0U)
#define error_api_get_error_canlib_payload(byte_size) (NULL)
#define error_api_get_expired_info() ((ErrorInfo){ 0U })

#endif // CONF_ERROR_MODULE_ENABLE

#endif // ERROR_API_H
