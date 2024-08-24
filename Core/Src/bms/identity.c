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

/**
 * @brief Identity module handler structure
 *
 * @param cellboard_id The identifier of the current cellboard
 * @param build_time The unix timestamp of the latest build time
 * @param can_payload The payload of the canlib message containing the cellboard version
 */
_STATIC struct {
    CellboardId cellboard_id;
    seconds_t build_time;

    bms_cellboard_version_converted_t can_payload;
} hidentity;

void identity_init(CellboardId id) {
    memset(&hidentity, 0U, sizeof(hidentity));

    hidentity.cellboard_id = id;

    // TODO: Build time at compile time
    // Get build time
    struct tm tm = { 0 };
    // Ignore warnings from this line
    if (strptime(IDENTITY_BUILD_TIME_STR, "%b %d %Y %H:%M:%S", &tm) != NULL)
        hidentity.build_time = mktime(&tm);

    // Update canlib payload info
    hidentity.can_payload.cellboard_id = id;
    hidentity.can_payload.component_build_time = hidentity.build_time >> 3U; // Remove 3 bits to keep size inside the allowed range
    hidentity.can_payload.canlib_build_time = CANLIB_BUILD_TIME;
}

CellboardId identity_get_cellboard_id(void) {
    return hidentity.cellboard_id;
}

seconds_t identity_get_build_time(void) {
    return hidentity.build_time;
}

bms_cellboard_version_converted_t * identity_get_canlib_payload(size_t * byte_size) {
    if (byte_size != NULL)
        *byte_size = sizeof(hidentity.can_payload);

    return &hidentity.can_payload;
}
