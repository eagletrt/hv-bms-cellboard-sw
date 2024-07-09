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

extern ADC_HandleTypeDef hadc1;

extern ADC_HandleTypeDef hadc2;

/* USER CODE BEGIN Private defines */

/** @brief Redefinition for the ADC handlers */
#define HADC_TEMPS_1 hadc1
#define HADC_TEMPS_2 hadc2

/** @brief Conversion timeout for the ADC conversion in ms */
#define ADC_CONVERSION_TIMEOUT (10U)

/** @brief ADC reference voltage in mV */
#define ADC_VREF ((millivolt_t)3300.0f)
/** @brief ADC resolution in bits */
#define ADC_RESOLUTION (12U)
/**
 * @brief Voltage range that correspond to a certian cellboard index in mV
 *
 * @details The +1 at the end is added to avoid overflow with the last value
 */
#define ADC_INDEX_VOLTAGE_RANGE ((millivolt_t)((ADC_VREF / CELLBOARD_COUNT) + 1.0f))

/**
 * @brief Convert a 12-bit ADC value to a voltage in mV
 *
 * @param value The value to convert
 *
 * @return millivolt_t The converted voltage value
 */
#define ADC_VALUE_TO_VOLTAGE(value) ((millivolt_t)((value) / ((1 << ADC_RESOLUTION) - 1.0f) * ADC_VREF))

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

void MX_ADC1_Init(void);
void MX_ADC2_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

