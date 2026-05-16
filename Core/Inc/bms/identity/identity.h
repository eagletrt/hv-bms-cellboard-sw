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

#include "cellboard-def.h"
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

#endif // IDENTITY_H
