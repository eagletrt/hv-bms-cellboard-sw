/*!
 * \file post-api.h
 * \date 2024-04-16
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#ifndef POST_API_H
#define POST_API_H

#include "post.h"

#ifdef CONF_POST_MODULE_ENABLE

/*!
 * \brief Run the power-on self test
 *
 * \details This function check if all the systems and peripherals work
 * as execpected, otherwise it returns an error code
 *
 * \param data The data needed by the POST module for initialization
 *
 * \retval POST_RC_INVALID_CELLBOARD_ID if the given cellboard id is not valid
 * \retval POST_RC_UNINITIALIZED if any of the modules cannot be initialized correctly
 * \retval POST_RC_NULL_POINTER if the given function pointers are NULL
 * \retval POST_RC_OK otherwise
 */
enum PostReturnCode post_api_run(struct PostInitData data);

#else // CONF_POST_MODULE_ENABLE

#define post_api_run(data) (POST_RC_OK)

#endif // CONF_POST_MODULE_ENABLE

#endif // POST_API_H
