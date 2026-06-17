/*!
 * \file bms-manager-api.h
 * \date 2024-05-07
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief API for the BMS manager module
 */

#ifndef BMS_MANAGER_API_H
#define BMS_MANAGER_API_H

#include "bms-manager.h"
#include "cellboard-conf.h"
#include "cellboard-def.h"

#ifdef CONF_BMS_MANAGER_MODULE_ENABLE

/*!
 * \brief Initialize the bms manager internal handler structure
 *
 * \param send A pointer to the callback used to send data via SPI (can be NULL)
 * \param send_receive A pointer to the callback used to send and receive data via SPI
 *
 * \retval BMS_MANAGER_RC_NULL_POINTER if the send/receive callback pointer is NULL
 * \retval BMS_MANAGER_RC_OK if the handler is initialized correctly
 */
enum BmsManagerReturnCode bms_manager_api_init(bms_manager_send_callback_t send, bms_manager_send_receive_callback_t send_receive);

/*!
 * \brief Routine that handles the communication with the BMS monitor
 *
 * \details This function should be called periodically with a certain interval
 *
 * \retval BMS_MANAGER_RC_OK
 */
enum BmsManagerReturnCode bms_manager_api_routine(void);

/*!
 * \brief Write the configuration registers of the BMS monitor
 *
 * \attention This function does not ensure that the data is correctly stored inside the LTCs
 * to check if the registers are updated correctly a read command has to be performed
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_write_configuration(void);

/*!
 * \brief Read the configuration registers from the BMS monitor
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_DECODE_ERROR if there was an error while decoding the received data
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_read_configuration(void);

/*!
 * \brief Start the cells voltage ADC conversion
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_start_volt_conversion(void);

/*!
 * \brief Start the discharge resistors temperatures ADC conversion
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_start_temp_conversion(void);

/*!
 * \brief Start the open wire ADC conversion
 *
 * \param pull_up Pull-up/pull-down option to select
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_start_open_wire_conversion(enum BmsManagerOpenWireOperation pull_up);

/*!
 * \brief Check if the started ADC conversion has ended
 *
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_poll_conversion_status(void);

/*!
 * \brief Read the cells voltages from the BMS monitor
 *
 * \param reg The register to read from
 *
 * \retval BMS_MANAGER_RC_DECODE_ERROR if there was an error while decoding the data
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_read_voltages(enum BmsManagerVoltageRegister reg);

/*!
 * \brief Read the discharge resistors temperatures from the LTCs
 *
 * \param reg The register to read from
 *
 * \retval BMS_MANAGER_RC_DECODE_ERROR if there was an error while decoding the data
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_read_temperatures(enum BmsManagerTemperatureRegister reg);

/*!
 * \brief Read the cells voltages after the open wire conversion from the LTCs
 *
 * \param reg The register to read from
 * \param op The type of operation completed before the readings (Pull-up/Pull-down)
 *
 * \retval BMS_MANAGER_RC_DECODE_ERROR if there was an error while decoding the data
 * \retval BMS_MANAGER_RC_ENCODE_ERROR if there was an error while encoding the command
 * \retval BMS_MANAGER_RC_COMMUNICATION_ERROR if there is an error during the transmission of the data
 * \retval BMS_MANAGER_RC_BUSY if the peripherial is busy
 * \retval BMS_MANAGER_RC_ERROR if an unkown error happens
 * \retval BMS_MANAGER_RC_OK otherwise
 */
enum BmsManagerReturnCode bms_manager_api_read_open_wire_voltages(enum BmsManagerVoltageRegister reg, enum BmsManagerOpenWireOperation pull_up_operation);

/*!
 * \brief Check for open wires
 *
 * \details To check for open wire the delta between the converted voltages values
 * is calculated for all the 12 cells excluded the first, then the open wire
 * is detected if:
 *     - The first pull up voltage value is 0.0000 (an epsilon is used to avoid float precision errors)
 *     - The last pull-down voltage value is 0.0000 (same as above)
 *     - At least one delta voltage value is below the -400 mV threshold
 *
 * \returns bit_flag32 A bitmask where the n-th bit represent the n-th cell (up to 32) that has an open wire
 */
bit_flag32 bms_manager_api_check_open_wire(void);

/*!
 * \brief Set the cells to discharge
 *
 * \param cells The bitmask where the n-th bit represent the n-th cell (up to 32)
 *
 * \retval BMS_MANAGER_RC_OK if the operation was successful
 */
enum BmsManagerReturnCode bms_manager_api_set_discharge_cells(bit_flag32 cells);

/*!
 * \brief Get the cells that are being currently discharged
 *
 * \returns bit_flag32 The bitmask where the n-th bit represent the n-th cell (up to 32)
 */
bit_flag32 bms_manager_api_get_discharge_cells(void);

#ifdef CONF_BMS_MANAGER_STRINGS_ENABLE

/*!
 * \brief Get a formatted string representation of the LTC6811 configuration
 *
 * \param config The configuration of the chip
 * \param out[out] The output string
 * \param size The maximum size of the output string
 *
 * \returns int The number of byte written inside the string or -1 on error
 */
int bms_manager_api_get_config_string(
    const Ltc6811Cfgr config,
    char *const out,
    const size_t size);

/*!
 * \brief Get a formatted string representation of the requested config for a single LTC
 *
 * \param ltc The chip to get the config from
 * \param out[out] The output string
 * \param size The maximum size of the output string
 *
 * \returns int The number of byte written inside the string or -1 on error
 */
int bms_manager_api_get_requested_config_string(
    const size_t ltc,
    char *const out,
    const size_t size);

/*!
 * \brief Get a formatted string representation of the actual config for a single LTC
 *
 * \param ltc The chip to get the config from
 * \param out[out] The output string
 * \param size The maximum size of the output string
 *
 * \returns int The number of byte written inside the string or -1 on error
 */
int bms_manager_api_get_actual_config_string(
    const size_t ltc,
    char *const out,
    const size_t size);

#else // CONF_BMS_MANAGER_STRINGS_ENABLE

#define bms_manager_api_get_config_string(config, out, size) (0)
#define bms_manager_api_get_requested_config_string(config, out, size) (0)
#define bms_manager_api_get_actual_config_string(config, out, size) (0)

#endif // CONF_BMS_MANAGER_STRINGS_ENABLE

#else // CONF_BMS_MANAGER_MODULE_ENABLE

#define bms_manager_api_init(send, send_receive) (BMS_MANAGER_RC_OK)
#define bms_manager_api_routine() (BMS_MANAGER_RC_OK)
#define bms_manager_api_write_configuration() (BMS_MANAGER_RC_OK)
#define bms_manager_api_read_configuration() (BMS_MANAGER_RC_OK)
#define bms_manager_api_start_volt_conversion() (BMS_MANAGER_RC_OK)
#define bms_manager_api_start_temp_conversion() (BMS_MANAGER_RC_OK)
#define bms_manager_api_start_open_wire_conversion() (BMS_MANAGER_RC_OK)
#define bms_manager_api_poll_conversion_status() (BMS_MANAGER_RC_OK)
#define bms_manager_api_read_voltages(reg) (BMS_MANAGER_RC_OK)
#define bms_manager_api_read_temperatures(reg, op) (BMS_MANAGER_RC_OK)
#define bms_manager_api_check_open_wire() (0U)
#define bms_manager_api_set_discharge_cells(cells) (BMS_MANAGER_RC_OK)
#define bms_manager_api_get_discharge_cells() (0U)

#endif // CONF_BMS_MANAGER_MODULE_ENABLE

#endif // BMS_MANAGER_API_H
