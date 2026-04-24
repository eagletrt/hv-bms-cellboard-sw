/**
 * \file identity.h
 * \date 2024-04-26
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Module that stores the information about the identity of the current cellboard
 * running this code
 */

#ifndef IDENTITY_H
#define IDENTITY_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"
#include "eagletrt.h"
#include "bms_network.h"

/**
 * \brief Definition of the string containing the latest build time
 *
 * \details The build time string is in the following format %b %d %Y %H:%M:%S
 * -> month day year hours:minutes:seconds (e.g. Apr 28 2024 11:45:15)
 */
#ifndef IDENTITY_BUILD_TIME_STRING
#define IDENTITY_BUILD_TIME_STRING (__DATE__ " "__TIME__)
#endif // IDENTITY_BUILD_TIME_STRING

/**
 * \brief Identity module handler structure
 *
 * \attention This structure should not be used outside of this function
 *
 * \param cellboard_id The identifier of the current cellboard
 * \param build_time The unix timestamp of the latest build time
 * \param version_can_payload The payload for the canlib message containing the version info
 */
struct IdentityHandler {
    enum CellboardId cellboard_id;
    seconds build_time;

    bms_cellboard_version_converted_t version_can_payload;
};

/**
 * \brief Return code for the identity module functions
 *
 * \details
 *     - IDENTITY_RC_OK the function executed succesfully
 *     - IDENTITY_RC_INVALID_ID the given cellboard id is invalid
 */
enum IdentityReturnCode {
    IDENTITY_RC_OK,
    IDENTITY_RC_INVALID_ID

};

#ifdef CONF_IDENTITY_MODULE_ENABLE

/**
 * \brief Initialize all the info about the cellboard identity
 *
 * \param id The cellboard id
 * \retval IDENTITY_RC_OK the function executed succesfully
 * \retval IDENTITY_RC_INVALID_ID the given cellboard id is invalid
 */
enum IdentityReturnCode identity_init(const enum CellboardId id);

/**
 * \brief Get the cellboard idenfitier
 *
 * \return enum CellboardId The cellboard id
 */
enum CellboardId identity_get_cellboard_id(void);

/**
 * \brief Get the cellboard software build time in unix timestamp format (seconds)
 *
 * \return seconds The build time
 */
seconds identity_get_build_time(void);

/**
 * \brief Get a pointer to the canlib payload of the cellboard identity info
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * \return bms_cellboard_version_converted_t* A pointer to the payload
 */
bms_cellboard_version_converted_t *identity_get_version_canlib_payload(size_t *const byte_size);

#else // CONF_IDENTITY_MODULE_ENABLE

#define identity_init(id) EAGLETRT_API_NOP()
#define identity_get_cellboard_id() (CELLBOARD_ID_0)
#define identity_get_build_time() (0U)
#define identity_get_version_canlib_payload(byte_size) (NULL)

#endif // CONF_IDENTITY_MODULE_ENABLE

#endif // IDENTITY_H
