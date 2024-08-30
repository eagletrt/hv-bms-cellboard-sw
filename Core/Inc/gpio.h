/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include <stdbool.h>

#include "cellboard-def.h"
#include "led.h"

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief Set the state of the on-board LED
 *
 * @param state The state to set
 */
void gpio_led_set_state(const LedStatus state);

/**
 * @brief Toggle the current state of the LED
 */
void gpio_led_toggle_state(void);

/**
 * @brief Get the current cellboard identifier
 *
 * @return CellboardId The ID of the cellboard or -1 on error
 */
CellboardId gpio_get_cellboard_id(void);

/**
 * @brief Set the address of the multiplexer used to get the cells temperature measurements
 */
void gpio_set_mux_address(const uint8_t address);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

