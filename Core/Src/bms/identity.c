/**
 * @file identity.c
 * @date 2024-04-26
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Module that stores the information about the identity of the current board
 */

#include "identity.h"

#include <time.h>
#include <string.h>

_STATIC _IdentityHandler hidentity;

void identity_init(const CellboardId id) {
    memset(&hidentity, 0U, sizeof(hidentity));

    hidentity.cellboard_id = id;

    // TODO: Build time at compile time
    // Get build time
    struct tm tm = { 0 };
    // Ignore warnings from this line
    if (strptime(IDENTITY_BUILD_TIME_STR, "%b %d %Y %H:%M:%S", &tm) != NULL)
        hidentity.build_time = mktime(&tm);

    // Update canlib payload info
    hidentity.version_can_payload.cellboard_id = (bms_cellboard_version_cellboard_id)id;
    hidentity.version_can_payload.component_build_time = hidentity.build_time >> 3U; // Remove 3 bits to keep size inside the allowed range
    hidentity.version_can_payload.canlib_build_time = CANLIB_BUILD_TIME;
}

CellboardId identity_get_cellboard_id(void) {
    return hidentity.cellboard_id;
}

seconds_t identity_get_build_time(void) {
    return hidentity.build_time;
}

bms_cellboard_version_converted_t * identity_get_version_canlib_payload(size_t * const byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hidentity.version_can_payload);
    return &hidentity.version_can_payload;
}
