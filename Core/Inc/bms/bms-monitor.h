/**
 * @file bms-monitor.h
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Handling function for the BMS monitor
 */

#include <stddef.h>
#include <stdint.h>

#include "cellboard-conf.h"

/**
 * @brief Return code for the BMS monitor module functions
 *
 * @details
 *     - BMS_MONITOR_OK the function executed succesfully
 *     - BMS_MONITOR_NULL_POINTER a NULL pointer was given to a function
 *     - BMS_MONITOR_ENCODE_ERROR some data could not be encoded correctly
 *     - BMS_MONITOR_BUSY the monitor is busy making ADC conversions
 */
typedef enum {
    BMS_MONITOR_OK,
    BMS_MONITOR_NULL_POINTER,
    BMS_MONITOR_ENCODE_ERROR,
    BMS_MONITOR_BUSY
} BmsMonitorReturnCode;

/**
 * @brief Callback used to send data via SPI
 *
 * @param data A pointer to the data to send
 * @param size The length of the data in bytes
 */
typedef void (* bms_monitor_send_callback)(uint8_t * data, size_t size);

/**
 * @brief Callback used to send and receive data via SPI
 *
 * @param data A pointer to the data to send
 * @param out[out] A pointer where the received data is stored
 * @param size The length of the sent data in bytes
 * @param out_size The length of the received data in bytes
 */
typedef void (* bms_monitor_send_receive_callback)(
    uint8_t * data,
    uint8_t * out,
    size_t size,
    size_t out_size
);


#ifdef CONF_BMS_MONITOR_MODULE_ENABLE

/**
 * @brief Initialize the bms monitor internal handler structure
 *
 * @param send A pointer to the callback used to send data via SPI (can be NULL)
 * @param send_receive A pointer to the callback used to send and receive data via SPI
 *
 * @return BmsMonitorReturnCode
 *     - BMS_MONITOR_NULL_POINTER if the send/receive callback pointer is NULL
 *     - BMS_MONITOR_OK otherwise
 */
BmsMonitorReturnCode bms_monitor_init(
    bms_monitor_send_callback send,
    bms_monitor_send_receive_callback send_receive
);

/**
 * @brief Start the ADC conversion for the cell voltages
 *
 * @return BmsMonitorReturnCode
 *     - BMS_MONITOR_BUSY the monitor is busy performing conversions
 *     - BMS_MONITOR_ENCODE_ERROR if there is an error while encoding the data to send
 *     - BMS_MONITOR_OK otherwise
 */
BmsMonitorReturnCode bms_monitor_start_volt_conversion(void);

/**
 * @brief Start the ADC conversion for the GPIO
 *
 * @return BmsMonitorReturnCode
 *     - BMS_MONITOR_BUSY the monitor is busy performing conversions
 *     - BMS_MONITOR_ENCODE_ERROR if there is an error while encoding the data to send
 *     - BMS_MONITOR_OK otherwise
 */
BmsMonitorReturnCode bms_monitor_start_gpio_conversion(void);

/**
 * @brief Start the ADC conversion for the cell voltages and GPIO simultaneously
 *
 * @return BmsMonitorReturnCode
 *     - BMS_MONITOR_BUSY the monitor is busy performing conversions
 *     - BMS_MONITOR_ENCODE_ERROR if there is an error while encoding the data to send
 *     - BMS_MONITOR_OK otherwise
 */
BmsMonitorReturnCode bms_monitor_start_volt_and_gpio_conversion(void);

#else  // CONF_BMS_MONITOR_MODULE_ENABLE

#define bms_monitor_init() (BMS_MONITOR_OK)
#define bms_monitor_start_volt_conversion (BMS_MONITOR_OK)
#define bms_monitor_start_gpio_conversion (BMS_MONITOR_OK)
#define bms_monitor_start_volt_and_gpio_conversion (BMS_MONITOR_OK)

#endif // CONF_BMS_MONITOR_MODULE_ENABLE
