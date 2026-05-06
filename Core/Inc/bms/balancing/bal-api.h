/*!
 * \file bal-api.h
 * \date 2024-04-17
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Balancing handling functions
 */

#ifndef BAL_API_H
#define BAL_API_H

#include "bal.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

/*!
 * \brief Initialize the internal balancing handler structure
 *
 * \returns BAL_OK
 */
enum BalReturnCode bal_init(void);

/*!
 * \brief Handler the information received inside the canlib payload
 *
 * \param payload A pointer to the CAN paylaod data
 * \retval BAL_NULL_POINTER if the payload pointer is NULL
 * \retval BAL_WATCHDOG_ERROR if there was an error during the watchdog reset operation
 * \retval BAL_OK if the function executed correctly
 */
enum BalReturnCode bal_set_balancing_status_handle(bms_cellboard_set_balancing_status_converted_t *const payload);

/*!
 * \brief Check if the balancing is active
 *
 * \details The balancing is considered active even if it is not discharging
 *
 * \returns bool True if the balancing procedure is running, false otherwise
 */
bool bal_is_active(void);

/*!
 * \brief Check if the balancing is paused
 *
 * \returns bool True if the balancing procedure is paused, false otherwise
 */
bool bal_is_paused(void);

/*!
 * \brief Start the balancing mechanism
 *
 * \retval BAL_WATCHDOG_ERROR the interal watchdog could not be started
 * \retval BAL_OK if balancing is already active or if function executed correctly
 */
enum BalReturnCode bal_start(void);

/*!
 * \brief Stop the balancing mechanism
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_stop(void);

/*!
 * \brief Pause the discharge
 *
 * \details The balancing procedure can be paused only if active
 * \details The balancing procedure remains active even if it is paused
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_pause(void);

/*!
 * \brief Resume the discharge
 *
 * \details The balancing procedure can be resumed only if active
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_resume(void);

/*!
 * \brief Get a pointer to the CAN payload of the balancing info
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is
 * stored (can be NULL)
 *
 * \return bms_cellboard_balancing_status_converted_t* A pointer to the payload
 */
bms_cellboard_balancing_status_converted_t *bal_get_status_canlib_payload(size_t *const byte_size);

#else // CONF_BALANCING_MODULE_ENABLE

#define bal_init() (BAL_OK)
#define bal_set_balancing_status_handle(payload) (NULL)
#define bal_set_balancing_status_handle (NULL)
#define bal_is_active() (false)
#define bal_is_paused() (false)
#define bal_start() (BAL_OK)
#define bal_stop() (BAL_OK)
#define bal_pause() (BAL_OK)
#define bal_resume() (BAL_OK)
#define bal_get_status_canlib_payload(byte_size) (NULL)

#endif // CONF_BALANCING_MODULE_ENABLE

#endif // BAL_API_H
