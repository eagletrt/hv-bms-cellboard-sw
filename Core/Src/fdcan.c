/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */

#include "can-comm.h"
#include "bms_network.h"

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 16;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 2;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clocks
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = CAN_RX_Pin|CAN_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, CAN_RX_Pin|CAN_TX_Pin);

  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

static uint32_t can_size_to_dlc[] = {
    [0] = FDCAN_DLC_BYTES_0,
    [1] = FDCAN_DLC_BYTES_1,
    [2] = FDCAN_DLC_BYTES_2,
    [3] = FDCAN_DLC_BYTES_3,
    [4] = FDCAN_DLC_BYTES_4,
    [5] = FDCAN_DLC_BYTES_5,
    [6] = FDCAN_DLC_BYTES_6,
    [7] = FDCAN_DLC_BYTES_7,
    [8] = FDCAN_DLC_BYTES_8
};

// TODO: Return and check errors
void can_send(can_id id, const uint8_t * data, size_t size) {
    if (id >= bms_MESSAGE_COUNT)
        return CAN_COMM_INVALID_INDEX;
    if (size > CELLBOARD_CAN_MAX_PAYLOAD_BYTE_SIZE)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;

    // Setup transmission header
    const FDCAN_TxHeaderTypeDef header = {
        .Identifier = id,
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = can_size_to_dlc[size],
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_FD_CAN,
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
        .MessageMarker = 0U
    };

    // Send message
    if (HAL_FDCAN_AddMessageToTxFifoQ(&HCAN_BMS, &header, data) != HAL_OK)
        return CAN_COMM_TRANSMISSION_ERROR;
    return CAN_COMM_OK;
}

// TODO: Return and check errors
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef * hfdcan, uint32_t RxFifo0ITs) {
    if (hfdcan->Instance != HCAN_BMS.Instance)
        return;
    // if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == RESET)
    //     return;

    FDCAN_RxHeaderTypeDef header;
    uint8_t data[CELLBOARD_CAN_MAX_PAYLOAD_BYTE_SIZE];
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, data) != HAL_OK)
        Error_Handler();

    // Update rx data
    can_comm_rx_add(bms_index_from_id(header.Identifier), data, header.DataLength);
}

// TODO: Return and check errors
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef * hfdcan, uint32_t RxFifo1ITs) {
    UNUSED(hfdcan);
    UNUSED(RxFifo1ITs);
}

/* USER CODE END 1 */
