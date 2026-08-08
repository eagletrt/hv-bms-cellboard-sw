/*!
 * \file identity-api.c
 * \date 2024-04-26
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Module that stores the information about the identity of the current board
 */

#include "identity.h"

#include <time.h>
#include <string.h>
#include "eagletrt.h"

#ifdef CONF_IDENTITY_MODULE_ENABLE

EAGLETRT_STATIC struct IdentityHandler identity_handler;

enum IdentityReturnCode identity_api_init(const enum CellboardId cellboard_id) {
    memset(&identity_handler, 0U, sizeof(identity_handler));
    if (cellboard_id >= CELLBOARD_ID_COUNT) {
        return IDENTITY_RC_INVALID_ID;
    }

    identity_handler.cellboard_id = cellboard_id;

    // Get build time
    struct tm current_time = { 0 };
    // Ignore warnings from this line
    if (strptime(IDENTITY_BUILD_TIME_STRING, "%b %d %Y %H:%M:%S", &current_time) != NULL) {
        identity_handler.build_time = mktime(&current_time);
    }

    // TODO: update libcan
    // Update canlib payload info
    // identity_handler.version_can_payload.cellboard_id = (bms_cellboard_version_cellboard_id)cellboard_id;
    // identity_handler.version_can_payload.component_build_time = identity_handler.build_time >> 3U; // Remove 3 bits to keep size inside the allowed range
    // identity_handler.version_can_payload.canlib_build_time = CANLIB_BUILD_TIME;
    return IDENTITY_RC_OK;
}

enum CellboardId identity_api_get_cellboard_id(void) {
    return identity_handler.cellboard_id;
}

seconds identity_api_get_build_time(void) {
    return identity_handler.build_time;
}

// TODO: update libcan
// bms_cellboard_version_converted_t *identity_api_get_version_canlib_payload(size_t *const byte_size) {
//     if (byte_size != NULL) {
//         *byte_size = sizeof(identity_handler.version_can_payload);
//     }
//     return &identity_handler.version_can_payload;
// }

#ifdef CONF_IDENTITY_STRINGS_ENABLE

EAGLETRT_STATIC char *identity_module_name = "identity";

#endif // CONF_IDENTITY_STRINGS_ENABLE

#endif // CONF_IDENTITY_MODULE_ENABLE
