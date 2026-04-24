/**
 * \file identity-api.h
 * \date 2024-04-26
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief API for the module that stores the information about the identity of the current cellboard
 * running this code
 */

#ifndef IDENTITY_API_H
#define IDENTITY_API_H

#include "identity.h"

#ifdef CONF_IDENTITY_MODULE_ENABLE

/**
 * \brief Initialize all the info about the cellboard identity
 *
 * \param id The cellboard id
 * \retval IDENTITY_RC_OK the function executed succesfully
 * \retval IDENTITY_RC_INVALID_ID the given cellboard id is invalid
 */
enum IdentityReturnCode identity_api_init(const enum CellboardId id);

/**
 * \brief Get the cellboard idenfitier
 *
 * \return enum CellboardId The cellboard id
 */
enum CellboardId identity_api_get_cellboard_id(void);

/**
 * \brief Get the cellboard software build time in unix timestamp format (seconds)
 *
 * \return seconds The build time
 */
seconds identity_api_get_build_time(void);

/**
 * \brief Get a pointer to the canlib payload of the cellboard identity info
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \return bms_cellboard_version_converted_t* A pointer to the payload
 */
bms_cellboard_version_converted_t *identity_api_get_version_canlib_payload(size_t *const byte_size);

#else // CONF_IDENTITY_MODULE_ENABLE

#define identity_api_init(id) EAGLETRT_API_NOP()
#define identity_api_get_cellboard_id() (CELLBOARD_ID_0)
#define identity_api_get_build_time() (0U)
#define identity_api_get_version_canlib_payload(byte_size) (NULL)

#endif // CONF_IDENTITY_MODULE_ENABLE

#endif // IDENTITY_API_H
