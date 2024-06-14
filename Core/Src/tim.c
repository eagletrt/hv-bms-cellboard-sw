/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

#include "timer_utils.h"
#include "error.h"

/* USER CODE END 0 */

TIM_HandleTypeDef htim6;

/* TIM6 init function */
void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspInit 0 */

  /* USER CODE END TIM6_MspInit 0 */
    /* TIM6 clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();
  /* USER CODE BEGIN TIM6_MspInit 1 */

  /* USER CODE END TIM6_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspDeInit 0 */

  /* USER CODE END TIM6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();
  /* USER CODE BEGIN TIM6_MspDeInit 1 */

  /* USER CODE END TIM6_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief Callback that updates the error timer based on a timestamp and a timeout
 *
 * @details The autoreload register value is set to the difference between
 * the current time and the timestamp plus the timeout
 *
 * @param timestamp The time in which the error was set
 * @param timeout The error timeout
 */
void error_update_timer_callback(uint32_t timestamp, uint16_t timeout) {
    HAL_TIM_Base_Stop_IT(&TIM_ERROR);
    int32_t t = HAL_GetTick();
    int32_t dt = ((int32_t)timestamp - t) + (int32_t)timeout;
    if (dt < 0)
        dt = 0;
    __HAL_TIM_SET_COUNTER(&TIM_ERROR, 0);
    __HAL_TIM_SET_AUTORELOAD(&TIM_ERROR, TIM_MS_TO_TICKS(&TIM_ERROR, dt));
    __HAL_TIM_CLEAR_FLAG(&TIM_ERROR, TIM_IT_CC1);
    HAL_TIM_Base_Start_IT(&TIM_ERROR);
}

/**
 * @brief Callback that stops the error timer
 */
void error_stop_timer_callback(void) {
    HAL_TIM_Base_Stop_IT(&TIM_ERROR);
}

/**
 * @brief Timer period elapsed callback
 *
 * @details This function is called when a timer autoreload register reaches
 * the maximum value
 *
 * @param htim A pointer to the timer handler structure
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
    if (htim->Instance == TIM_ERROR.Instance) {
        // Stop the timer and expire the error
        HAL_TIM_Base_Stop_IT(&TIM_ERROR);
        error_expire();
    }
}

/* USER CODE END 1 */
