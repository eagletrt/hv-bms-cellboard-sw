/*!
 * \file bms-manager.h
 * \date 2024-05-07
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Manager for the BMS monitor operations
 */

#ifndef BMS_MANAGER_H
#define BMS_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "volt-api.h"
#include "ltc6811-1-api.h"

/*! \brief Thresholds used during the open wire check in V */
#define BMS_MANAGER_OPEN_WIRE_THRESHOLD_V (-0.400f)
#define BMS_MANAGER_OPEN_WIRE_ZERO_V (0.000005f)

/*! \brief Voltage reference of the LTCs ADC in V */
#define BMS_MANAGER_ADC_VREF (5.1f)

/*! \brief Resolution of the LTCs ADC in number of bits */
#define BMS_MANAGER_ADC_RESOLUTION (16)

/*!
 * \brief Convert the raw value read from the LTC to a voltage value in V
 *
 * \param value The raw value
 *
 * \return volt The converted voltage value in V
 */
// TODO: Move macro into the bms monitor library
#define BMS_MANAGER_RAW_VOLTAGE_TO_VOLT(value) ((value) * 0.0001f)

/*!
 * \brief Convert the raw value read from the GPIO of the LTCs to a voltage value in V
 *
 * \param value The raw value
 *
 * \return volt The converted voltage value in V
 */
// TODO: Move macro into the bms monitor library
#define BMS_MANAGER_RAW_GPIO_VALUE_TO_VOLT(value) ((value) * 0.0001f)

/*!
 * \brief Return code for the BMS manager module functions
 */
enum BmsManagerReturnCode {
    BMS_MANAGER_RC_OK,                  /*!< Function executed successfully */
    BMS_MANAGER_RC_NULL_POINTER,        /*!< A NULL pointer was given to a function */
    BMS_MANAGER_RC_ENCODE_ERROR,        /*!< Some data could not be encoded correctly */
    BMS_MANAGER_RC_DECODE_ERROR,        /*!< Some data could not be decoded correctly */
    BMS_MANAGER_RC_OPEN_WIRE,           /*!< An open wire is detected */
    BMS_MANAGER_RC_BUSY,                /*!< The manager or the peripheral is busy */
    BMS_MANAGER_RC_COMMUNICATION_ERROR, /*!< Communiction error with the LTCs */
    BMS_MANAGER_RC_ERROR                /*!< Generic error with unkown cause */
};

/*! \brief List of voltage registers */
enum BmsManagerVoltageRegister {
    BMS_MANAGER_VOLTAGE_REGISTER_A = LTC6811_1_CVAR,
    BMS_MANAGER_VOLTAGE_REGISTER_B = LTC6811_1_CVBR,
    BMS_MANAGER_VOLTAGE_REGISTER_C = LTC6811_1_CVCR,
    BMS_MANAGER_VOLTAGE_REGISTER_D = LTC6811_1_CVDR,
    BMS_MANAGER_VOLTAGE_REGISTER_COUNT = LTC6811_1_CVXR_COUNT
};

/*! \brief List of temperatures registers */
enum BmsManagerTemperatureRegister {
    BMS_MANAGER_TEMPERATURE_REGISTER_A = LTC6811_1_AVAR,
    BMS_MANAGER_TEMPERATURE_REGISTER_B = LTC6811_1_AVBR,
    BMS_MANAGER_TEMPERATURE_REGISTER_COUNT = LTC6811_1_AVXR_COUNT
};

/*! \brief List of open wire procedure operations */
enum BmsManagerOpenWireOperation {
    BMS_MANAGER_OPEN_WIRE_OPERATION_PUD = LTC6811_1_PUP_INACTIVE,
    BMS_MANAGER_OPEN_WIRE_OPERATION_PUP = LTC6811_1_PUP_ACTIVE,
};

/*!
 * \brief Callback used to send data via SPI
 *
 * \param data A pointer to the data to send
 * \param size The length of the data in bytes
 *
 * \returns enum BmsManagerReturnCode The result of the data transmission
 */
typedef enum BmsManagerReturnCode (*bms_manager_send_callback_t)(uint8_t *const data, const size_t size);

/*!
 * \brief Callback used to send and receive data via SPI
 *
 * \param data A pointer to the data to send
 * \param out[out] A pointer where the received data is stored
 * \param size The length of the sent data in bytes
 * \param out_size The length of the received data in bytes
 *
 * \return enum BmsManagerReturnCode The result of the data transmission and reception
 */
typedef enum BmsManagerReturnCode (*bms_manager_send_receive_callback_t)(
    uint8_t *const data,
    uint8_t *out,
    const size_t size,
    const size_t out_size);

/*!
 * \brief Type definition for the BMS manager handler structure
 *
 * \attention This struct should not be used outside of this module
 *
 * \details The pup and pud arrays are used for the open wire check
 * \details The requested configuration should match the actual configuration
 */
struct BmsManagerHandler {
    bms_manager_send_callback_t send;                 /*!< Callback used to send data via SPI */
    bms_manager_send_receive_callback_t send_receive; /*!< Callback used to send and receive data via SPI */

    struct Ltc68111Handler ltc_handler;                                /*!< LTC handler structure */
    struct Ltc68111Cfgr actual_config[CELLBOARD_SEGMENT_LTC_COUNT];    /*!< Actual configuration register read from the LTCs */
    struct Ltc68111Cfgr requested_config[CELLBOARD_SEGMENT_LTC_COUNT]; /*!< Requested configuration register of the LTCs */
    cells_volt pup[2U];                                                /*!< Array of cells voltages read with pull-up and pull-down used for the open-wire check (see LTC6811_1PUP) */
};

#endif // BMS_MANAGER_H
