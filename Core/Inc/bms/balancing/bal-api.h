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

#include <stdint.h>
#include "bal.h"
#include "cellboard-def.h"

#ifdef CONF_BALANCING_MODULE_ENABLE

/*!
 * \brief Initialize the internal balancing handler structure
 *
 * \returns BAL_OK
 */
enum BalReturnCode bal_api_init(void);

/*!
 * \brief Handle received mainboard balancing set command
 *
 * \param[id] start True if balancing start is requested, false if balancing stop is requested
 * \param[id] target Voltage target to reach to stop balancing
 * \param[id] threshold Voltage delta from the target to keep when balancing
 */
void bal_api_balancing_set_handle(bool start, volt target, volt threshold);

/*!
 * \brief Check if the balancing is active
 *
 * \details The balancing is considered active even if it is not discharging
 *
 * \returns bool True if the balancing procedure is running, false otherwise
 */
bool bal_api_is_active(void);

/*!
 * \brief Check if the balancing is paused
 *
 * \returns bool True if the balancing procedure is paused, false otherwise
 */
bool bal_api_is_paused(void);

/*!
 * \brief Start the balancing mechanism
 *
 * \retval BAL_WATCHDOG_ERROR the interal watchdog could not be started
 * \retval BAL_OK if balancing is already active or if function executed correctly
 */
enum BalReturnCode bal_api_start(void);

/*!
 * \brief Stop the balancing mechanism
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_api_stop(void);

/*!
 * \brief Pause the discharge
 *
 * \details The balancing procedure can be paused only if active
 * \details The balancing procedure remains active even if it is paused
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_api_pause(void);

/*!
 * \brief Resume the discharge
 *
 * \details The balancing procedure can be resumed only if active
 *
 * \return enum BalReturnCode
 *     - BAL_OK
 */
enum BalReturnCode bal_api_resume(void);

/*!
 * \brief Get a pointer to the CAN payload of the Cellboard balancing status
 *
 * \param byte_size[out] A pointer where the size of the payload in bytes is
 * stored (can be NULL)
 *
 * \return Pointer of the payload
 */
union CanBmsMessages *bal_api_get_canlib_payload(size_t *byte_size);

#else // CONF_BALANCING_MODULE_ENABLE

#define bal_api_init() (BAL_OK)
#define bal_api_set_balancing_status_handle(payload) (NULL)
#define bal_api_set_balancing_status_handle (NULL)
#define bal_api_is_active() (false)
#define bal_api_is_paused() (false)
#define bal_api_start() (BAL_OK)
#define bal_api_stop() (BAL_OK)
#define bal_api_pause() (BAL_OK)
#define bal_api_resume() (BAL_OK)
#define bal_api_balancing_set_handle(start, target, threshold) (EAGLETRT_API_NOP())

#endif // CONF_BALANCING_MODULE_ENABLE

#endif // BAL_API_H
