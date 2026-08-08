/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
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
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "cellboard-def.h"

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc2;

/* USER CODE BEGIN Private defines */

/** @brief Redefinition for the ADC handlers */
#define HADC_TEMPS hadc2

/** @brief Total number of ADC channels used */
#define ADC_DMA_CHANNEL_COUNT (3U)

/** @brief ADC reference voltage in V */
#define ADC_VREF (3.3f)
/** @brief ADC resolution in bits */
#define ADC_RESOLUTION (12U)

/**
 * @brief Return code for the ADC module functions
 *
 * @details
 *     - ADC_OK the function executed succefully
 *     - ADC_NULL_POINTER a NULL pointer was given to a function
 *     - ADC_TIMEOUT the ADC converstion has taken too long to complete
 *     - ADC_START_ERROR the ADC cannot be started
 *     - ADC_POLL_ERROR there was an error while polling for the conversion status
 *     - ADC_STOP_ERROR the ADC cannot be stopped
 */
typedef enum {
    ADC_OK,
    ADC_NULL_POINTER,
    ADC_TIMEOUT,
    ADC_START_ERROR,
    ADC_POLL_ERROR,
    ADC_STOP_ERROR
} AdcReturnCode;

/* USER CODE END Private defines */

void MX_ADC2_Init(void);

/* USER CODE BEGIN Prototypes */

/** @brief Start the temperature ADC conversion using DMA */
void adc_temperature_start_conversion(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

