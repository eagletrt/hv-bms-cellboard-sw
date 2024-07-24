/**
 * @file bms-manager.h
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Manager for the BMS monitor operations
 */

#ifndef BMS_MANAGER_H
#define BMS_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "volt.h"
#include "ltc6811.h"

/** @brief Thresholds used during the open wire check in mV */
#define BMS_MANAGER_OPEN_WIRE_THRESHOLD_MILLIVOLT ((millivolt_t)-400.f)
#define BMS_MANAGER_OPEN_WIRE_ZERO_MILLIVOLT ((millivolt_t)0.00005f)

/** @brief Thresholds used during the open wire check in mV * 10 */
#define BMS_MANAGER_OPEN_WIRE_THRESHOLD (-((int16_t)VOLT_MILLIVOLT_TO_VALUE(-(BMS_MANAGER_OPEN_WIRE_THRESHOLD_MILLIVOLT))))
#define BMS_MANAGER_OPEN_WIRE_ZERO VOLT_MILLIVOLT_TO_VALUE(BMS_MANAGER_OPEN_WIRE_ZERO_MILLIVOLT)

/**
 * @brief Return code for the BMS manager module functions
 *
 * @details
 *     - BMS_MANAGER_OK the function executed succesfully
 *     - BMS_MANAGER_NULL_POINTER a NULL pointer was given to a function
 *     - BMS_MANAGER_ENCODE_ERROR some data could not be encoded correctly
 *     - BMS_MANAGER_DECODE_ERROR some data could not be decoded correctly
 *     - BMS_MANAGER_OPEN_WIRE an open wire is detected
 *     - BMS_MANAGER_BUSY the manager or the peripheral is busy
 *     - BMS_MANAGER_COMMUNICATION_ERROR communiction error with the LTCs
 *     - BMS_MANAGER_ERROR generic error with unkown cause
 */
typedef enum {
    BMS_MANAGER_OK,
    BMS_MANAGER_NULL_POINTER,
    BMS_MANAGER_ENCODE_ERROR,
    BMS_MANAGER_DECODE_ERROR,
    BMS_MANAGER_OPEN_WIRE,
    BMS_MANAGER_BUSY,
    BMS_MANAGER_COMMUNICATION_ERROR,
    BMS_MANAGER_ERROR
} BmsManagerReturnCode;

/** @brief List of voltage registers */
typedef enum {
    BMS_MANAGER_VOLTAGE_REGISTER_A,
    BMS_MANAGER_VOLTAGE_REGISTER_B,
    BMS_MANAGER_VOLTAGE_REGISTER_C,
    BMS_MANAGER_VOLTAGE_REGISTER_D,
    BMS_MANAGER_VOLTAGE_REGISTER_COUNT
} BmsManagerVoltageRegister;

/** @brief List of temperatures registers */
typedef enum {
    BMS_MANAGER_TEMPERATURE_REGISTER_A,
    BMS_MANAGER_TEMPERATURE_REGISTER_B,
    BMS_MANAGER_TEMPERATURE_REGISTER_COUNT
} BmsManagerTemperatureRegister;

/** @brief List of open wire procedure operations */
typedef enum {
    BMS_MANAGER_OPEN_WIRE_OPERATION_PUD = LTC6811_PUP_INACTIVE,
    BMS_MANAGER_OPEN_WIRE_OPERATION_PUP = LTC6811_PUP_ACTIVE,
} BmsManagerOpenWireOperation;

/**
 * @brief Callback used to send data via SPI
 *
 * @param data A pointer to the data to send
 * @param size The length of the data in bytes
 *
 * @return BmsManagerReturnCode The result of the data transmission
 */
typedef BmsManagerReturnCode (* bms_manager_send_callback_t)(uint8_t * data, size_t size);

/**
 * @brief Callback used to send and receive data via SPI
 *
 * @param data A pointer to the data to send
 * @param out[out] A pointer where the received data is stored
 * @param size The length of the sent data in bytes
 * @param out_size The length of the received data in bytes
 *
 * @return BmsManagerReturnCode The result of the data transmission and reception
 */
typedef BmsManagerReturnCode (* bms_manager_send_receive_callback_t)(
    uint8_t * data,
    uint8_t * out,
    size_t size,
    size_t out_size
);

/**
 * @brief Type definitino for the BMS manager handler structure
 *
 * @attention This struct should not be used outside of this module
 *
 * @details The pup and pud arrays are used for the open wire check
 * @details The requested configuration should match the actual configuration
 *
 * @param send A pointer to the callback used to send the data via SPI
 * @param send_receive A pointer to the callback used to send and receive the data via SPI
 * @param chain The LTC handler structure
 * @param actual_config The actual configuration register read from the LTC
 * @param requested_config The requested configuration register of the LTC
 * @param pup An array of cells voltages read with pull-up active and inactive (see LTC6811_PUP)
 * @param communication_error_count A counter of the communication errors
 */
typedef struct {
    bms_manager_send_callback_t send;
    bms_manager_send_receive_callback_t send_receive;

    Ltc6811Chain chain;
    Ltc6811Cfgr actual_config[CELLBOARD_SEGMENT_LTC_COUNT];
    Ltc6811Cfgr requested_config[CELLBOARD_SEGMENT_LTC_COUNT];
    raw_volt_t pup[2U][CELLBOARD_SEGMENT_SERIES_COUNT];

    size_t communication_error_count;
} _BmsManagerHandler;

#ifdef CONF_BMS_MANAGER_MODULE_ENABLE

/**
 * @brief Initialize the bms manager internal handler structure
 *
 * @param send A pointer to the callback used to send data via SPI (can be NULL)
 * @param send_receive A pointer to the callback used to send and receive data via SPI
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_NULL_POINTER if the send/receive callback pointer is NULL
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_init(
    bms_manager_send_callback_t send,
    bms_manager_send_receive_callback_t send_receive
);

/**
 * @brief Routine that handles the communication with the BMS monitor
 *
 * @details This function should be called periodically with a certain interval
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OK
 */
BmsManagerReturnCode bms_manager_routine(void);

/**
 * @brief Write the configuration registers of the BMS monitor
 *
 * @attention This function does not ensure that the data is correctly stored inside the LTCs
 * to check if the registers are updated correctly a read command has to be performed
 * 
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR error while encoding the command
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_write_configuration(void);

/**
 * @brief Read the configuration registers from the BMS monitor
 * 
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the received data
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_read_configuration(void);

/**
 * @brief Start the cells voltage ADC conversion
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_start_volt_conversion(void);

/**
 * @brief Start the discharge resistors temperatures ADC conversion
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_start_temp_conversion(void);

/**
 * @brief Start the open wire ADC conversion
 *
 * @param pull_up Pull-up/pull-down option to select
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_start_open_wire_conversion(Ltc6811Pup pull_up);

/**
 * @brief Check if the started ADC conversion has ended
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_poll_conversion_status(void);

/**
 * @brief Read the cells voltages from the BMS monitor
 *
 * @param reg The register to read from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_read_voltages(BmsManagerVoltageRegister reg);

/**
 * @brief Read the discharge resistors temperatures from the LTCs
 *
 * @param reg The register to read from
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_read_temperatures(BmsManagerTemperatureRegister reg);

/**
 * @brief Read the cells voltages after the open wire conversion from the LTCs
 *
 * @param reg The register to read from
 * @param op The type of operation completed before the readings (Pull-up/Pull-down)
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_ENCODE_ERROR if there was an error while encoding the command
 *     - BMS_MANAGER_DECODE_ERROR if there was an error while decoding the data
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_read_open_wire_voltages(BmsManagerVoltageRegister reg, BmsManagerOpenWireOperation op);

/**
 * @brief Check for open wires
 *
 * @details To check for open wire the delta between the converted voltages values
 * is calculated for all the 12 cells excluded the first, then the open wire
 * is detected if:
 *     - The first pull up voltage value is 0.0000 (an epsilon is used to avoid float precision errors)
 *     - The last pull-down voltage value is 0.0000 (same as above)
 *     - At least one delta voltage value is below the -400 mV threshold
 *     
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OPEN_WIRE if an open wire is detected
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode bms_manager_check_open_wire(void);

/**
 * @brief Set the cells to discharge
 *
 * @param cells The bitmask where the n-th bit represent the n-th cell (up to 32)
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OK
 */
BmsManagerReturnCode bms_manager_set_discharge_cells(bit_flag32_t cells);

/**
 * @brief Get the cells that are being currently discharged
 *
 * @return bit_flag32_t The bitmask where the n-th bit represent the n-th cell (up to 32)
 */
bit_flag32_t bms_manager_get_discharge_cells(void);

#else  // CONF_BMS_MANAGER_MODULE_ENABLE

#define bms_manager_init(send, send_receive) (BMS_MANAGER_OK)
#define bms_manager_routine() (BMS_MANAGER_OK)
#define bms_manager_write_configuration() (BMS_MANAGER_OK)
#define bms_manager_read_configuration() (BMS_MANAGER_OK)
#define bms_manager_start_volt_conversion() (BMS_MANAGER_OK)
#define bms_manager_start_temp_conversion() (BMS_MANAGER_OK)
#define bms_manager_start_open_wire_conversion() (BMS_MANAGER_OK)
#define bms_manager_poll_conversion_status() (BMS_MANAGER_OK)
#define bms_manager_read_voltages(reg) (BMS_MANAGER_OK)
#define bms_manager_read_temperatures(reg, op) (BMS_MANAGER_OK)
#define bms_manager_check_open_wire() (BMS_MANAGER_OK)
#define bms_manager_set_discharge_cells(cells) (BMS_MANAGER_OK)
#define bms_manager_get_discharge_cells() (0U)

#endif // CONF_BMS_MANAGER_MODULE_ENABLE

#endif // BMS_MANAGER_H
