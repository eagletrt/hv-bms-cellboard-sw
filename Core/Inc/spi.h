/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "cellboard-def.h"
#include "cellboard-conf.h"

#include "bms-manager.h"

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN Private defines */

/** @brief Alias for the SPI handler structure */
#define HSPI_LTC hspi3

/**
 * @brief Possible states of the SPI Chip Select pin
 * 
 * @details The pin logic is inverted
 */
#define SPI_CS_SET GPIO_PIN_RESET
#define SPI_CS_RESET GPIO_PIN_SET

/* USER CODE END Private defines */

void MX_SPI3_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief Send data via SPI
 *
 * @param data A pointer to the data to send
 * @param size The length of the data in bytes
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode spi_send(uint8_t * const data, const size_t size);

/**
 * @brief Send and receive data via SPI
 *
 * @param data A pointer to the data to send
 * @param out[out] A pointer where the received data is stored
 * @param size The length of the sent data in bytes
 * @param out_size The length of the received data in bytes
 *
 * @return BmsManagerReturnCode
 *     - BMS_MANAGER_COMMUNICATION_ERROR if there is an error during the transmission or reception of the data
 *     - BMS_MANAGER_BUSY if the peripherial is busy
 *     - BMS_MANAGER_ERROR if an unkown error happens
 *     - BMS_MANAGER_OK otherwise
 */
BmsManagerReturnCode spi_send_and_receive(
    uint8_t * const data,
    uint8_t * const out,
    const size_t size,
    const size_t out_size
);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

