/**
 * @file identity.h
 * @date 2024-04-26
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Module that stores the information about the identity of the current cellboard
 * running this code
 */

#ifndef IDENTITY_H
#define IDENTITY_H

/*** !!! Needed for strptime function !!! ***/
#define _GNU_SOURCE
#define __USE_XOPEN
#define _XOPEN_SOURCE
/********************************************/

#include <stddef.h>

#include "cellboard-def.h"

#include "bms_network.h"

/**
 * @brief Definition of the string containing the latest build time
 *
 * @details The build time string is in the following format %b %d %Y %H:%M:%S
 * -> month day year hours:minutes:seconds (e.g. Apr 28 2024 11:45:15)
 */
#define IDENTITY_BUILD_TIME_STR (__DATE__" "__TIME__)

/**
 * @brief Initialize all the info about the cellboard identity
 *
 * @param id The cellboard id
 */
void identity_init(CellboardId id);

/**
 * @brief Get the cellboard idenfitier
 *
 * @return CellboardId The cellboard id
 */
CellboardId identity_get_cellboard_id(void);

/**
 * @brief Get the cellboard software build time in unix timestamp format (seconds)
 *
 * @return seconds_t The build time
 */
seconds_t identity_get_build_time(void);

/**
 * @brief Get a pointer to the canlib payload of the cellboard identity info
 *
 * @param byte_size[out] A pointer where the size of the payload in bytes is stored (can be NULL)
 *
 * @return bms_cellboard_version_converted_t* A pointer to the payload
 */
bms_cellboard_version_converted_t * identity_get_canlib_payload(size_t * byte_size);

#endif  // IDENTITY_H
