/**
 * @file bms-manager.h
 * @date 2024-05-07
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Manager for the BMS monitor operations
 */

#include <stddef.h>
#include <stdint.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

/**
 * @brief Return code for the BMS manager module functions
 *
 * @details
 *     - BMS_MANAGER_OK the function executed succesfully
 *     - BMS_MANAGER_NULL_POINTER a NULL pointer was given to a function
 *     - BMS_MANAGER_ENCODE_ERROR some data could not be encoded correctly
 *     - BMS_MANAGER_DECODE_ERROR some data could not be decoded correctly
 *     - BMS_MANAGER_OPEN_WIRE an open wire is detected
 *     - BMS_MANAGER_BUSY the manager is busy making ADC conversions
 */
typedef enum {
    BMS_MANAGER_OK,
    BMS_MANAGER_NULL_POINTER,
    BMS_MANAGER_ENCODE_ERROR,
    BMS_MANAGER_DECODE_ERROR,
    BMS_MANAGER_OPEN_WIRE,
    BMS_MANAGER_BUSY
} BmsManagerReturnCode;

/**
 * @brief Callback used to send data via SPI
 *
 * @param data A pointer to the data to send
 * @param size The length of the data in bytes
 */
typedef void (* bms_manager_send_callback_t)(uint8_t * data, size_t size);

/**
 * @brief Callback used to send and receive data via SPI
 *
 * @param data A pointer to the data to send
 * @param out[out] A pointer where the received data is stored
 * @param size The length of the sent data in bytes
 * @param out_size The length of the received data in bytes
 */
typedef void (* bms_manager_send_receive_callback_t)(
    uint8_t * data,
    uint8_t * out,
    size_t size,
    size_t out_size
);


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
 * @brief Set the cells to discharge
 *
 * @param cells The bitmask where the n-th bit represent the n-th cell up to 32
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OK
 */
BmsManagerReturnCode bms_manager_set_discharge_cells(bit_flag32_t cells);

/**
 * @brief Run a single bms manager operation
 *
 * @details This function should be run with a certain interval, preferably 2ms
 * 
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_OK
 */
BmsManagerReturnCode bms_manager_run(void);

#else  // CONF_BMS_MANAGER_MODULE_ENABLE

#define bms_manager_init(send, send_receive) (BMS_MANAGER_OK)
#define bms_manager_run() (BMS_MANAGER_OK)

#endif // CONF_BMS_MANAGER_MODULE_ENABLE
