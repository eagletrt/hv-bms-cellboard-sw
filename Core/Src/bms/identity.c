/**
 * @file identity.c
 * @date 2024-04-26
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Module that stores the information about the identity of the current cellboard
 * running this code
 */

#include "identity.h"

static CellboardId cellboard_id;
static time build_time;
static char * build_time_str = __DATE__" "__TIME__;

static bms_cellboard_version_converted_t version_can_payload;

void identity_init(CellboardId id) {
    cellboard_id = id;

    // TODO: Build time at compile time
    // Get build time
    struct tm tm = { 0 };
    // Ignore warnings from this line
    if (strptime(build_time_str, "%b %d %Y %H:%M:%S", &tm) != NULL)
        build_time = mktime(&tm);

    // Update canlib payload info
    version_can_payload.cellboard_id = id;
    version_can_payload.component_build_time = build_time >> 3U; // Remove 3 bits to keep size inside the allowed range
    version_can_payload.canlib_build_time = CANLIB_BUILD_TIME;
}

CellboardId identity_get_cellboard_id(void) {
    return cellboard_id;
}

time identity_get_build_time(void) {
    return build_time;
}

char * identity_get_string_build_time(void) {
    return build_time_str;
}

bms_cellboard_version_converted_t * identity_get_can_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(version_can_payload);

    return &version_can_payload;
}
