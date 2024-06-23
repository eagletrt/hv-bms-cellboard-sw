/**
 * @file identity.c
 * @date 2024-04-26
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Module that stores the information about the identity of the current cellboard
 * running this code
 */

#include "identity.h"

#include <time.h>

#define BUILD_TIME_STR (__DATE__" "__TIME__)

// WARNING: this structure should not be used outside of this file
_STATIC _IdentityHandler hidentity;

void identity_init(CellboardId id) {
    hidentity.cellboard_id = id;

    // TODO: Build time at compile time
    // Get build time
    struct tm tm = { 0 };
    // Ignore warnings from this line
    if (strptime(BUILD_TIME_STR, "%b %d %Y %H:%M:%S", &tm) != NULL)
        hidentity.build_time = mktime(&tm);

    // Update canlib payload info
    hidentity.can_payload.cellboard_id = id;
    hidentity.can_payload.component_build_time = hidentity.build_time >> 3U; // Remove 3 bits to keep size inside the allowed range
    hidentity.can_payload.canlib_build_time = CANLIB_BUILD_TIME;
}

CellboardId identity_get_cellboard_id(void) {
    return hidentity.cellboard_id;
}

milliseconds_t identity_get_build_time(void) {
    return hidentity.build_time;
}

char * identity_get_string_build_time(void) {
    return BUILD_TIME_STR;
}

bms_cellboard_version_converted_t * identity_get_can_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hidentity.can_payload);

    return &hidentity.can_payload;
}
