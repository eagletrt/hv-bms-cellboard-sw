/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
#define MUX_OUT0_Pin GPIO_PIN_0
#define MUX_OUT0_GPIO_Port GPIOA
#define MUX_OUT1_Pin GPIO_PIN_1
#define MUX_OUT1_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define MUX_OUT2_Pin GPIO_PIN_4
#define MUX_OUT2_GPIO_Port GPIOA
#define ID_SELECTOR_0_Pin GPIO_PIN_5
#define ID_SELECTOR_0_GPIO_Port GPIOA
#define ID_SELECTOR_1_Pin GPIO_PIN_6
#define ID_SELECTOR_1_GPIO_Port GPIOA
#define ID_SELECTOR_2_Pin GPIO_PIN_7
#define ID_SELECTOR_2_GPIO_Port GPIOA
#define MUX_A0_Pin GPIO_PIN_8
#define MUX_A0_GPIO_Port GPIOA
#define MUX_A1_Pin GPIO_PIN_9
#define MUX_A1_GPIO_Port GPIOA
#define MUX_A2_Pin GPIO_PIN_10
#define MUX_A2_GPIO_Port GPIOA
#define CAN_RX_Pin GPIO_PIN_11
#define CAN_RX_GPIO_Port GPIOA
#define CAN_TX_Pin GPIO_PIN_12
#define CAN_TX_GPIO_Port GPIOA
#define MUX_A3_Pin GPIO_PIN_15
#define MUX_A3_GPIO_Port GPIOA
#define LTC_SCK_Pin GPIO_PIN_3
#define LTC_SCK_GPIO_Port GPIOB
#define LTC_MISO_Pin GPIO_PIN_4
#define LTC_MISO_GPIO_Port GPIOB
#define LTC_MOSI_Pin GPIO_PIN_5
#define LTC_MOSI_GPIO_Port GPIOB
#define LTC_CS_Pin GPIO_PIN_6
#define LTC_CS_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
