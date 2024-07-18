/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through
        * the Code Generation settings)
     PA0   ------> SharedAnalog_PA0
     PA1   ------> SharedAnalog_PA1
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MUX_A0_Pin|MUX_A1_Pin|MUX_A2_Pin|MUX_A3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LTC_CS_Pin|LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NRST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = MUX_OUT0_Pin|MUX_OUT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = ID_SELECTOR_0_Pin|ID_SELECTOR_1_Pin|ID_SELECTOR_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = MUX_A0_Pin|MUX_A1_Pin|MUX_A2_Pin|MUX_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = LTC_CS_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

void gpio_led_set_state(LedStatus state) {
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (GPIO_PinState)state);
}

void gpio_led_toggle_state(void) {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

CellboardId gpio_get_cellboard_id(void) {
    CellboardId id = 0;
    /*
     * Get the individual bits of the cellboard identifier
     * The bits are inverted because it is Aris fault
     */
    bool bit_0 = !(bool)HAL_GPIO_ReadPin(ID_SELECTOR_0_GPIO_Port, ID_SELECTOR_0_Pin);
    bool bit_1 = !(bool)HAL_GPIO_ReadPin(ID_SELECTOR_1_GPIO_Port, ID_SELECTOR_1_Pin);
    bool bit_2 = !(bool)HAL_GPIO_ReadPin(ID_SELECTOR_2_GPIO_Port, ID_SELECTOR_2_Pin);

    // Set the identifier using the bits
    id = CELLBOARD_BIT_TOGGLE_IF(id, bit_0, 0U);
    id = CELLBOARD_BIT_TOGGLE_IF(id, bit_1, 1U);
    id = CELLBOARD_BIT_TOGGLE_IF(id, bit_2, 2U);

    if (id >= CELLBOARD_ID_COUNT)
        id = CELLBOARD_ID_5;
    return id;
}

void gpio_set_mux_address(uint8_t address) {
    if (address >= CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT)
        return;
    
    // Get the address bits
    bool b0 = CELLBOARD_BIT_GET(address, 0U);
    bool b1 = CELLBOARD_BIT_GET(address, 1U);
    bool b2 = CELLBOARD_BIT_GET(address, 2U);
    bool b3 = CELLBOARD_BIT_GET(address, 3U);

    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (GPIO_PinState)b0);
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (GPIO_PinState)b1);
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (GPIO_PinState)b2);
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (GPIO_PinState)b3);
}

/* USER CODE END 2 */
