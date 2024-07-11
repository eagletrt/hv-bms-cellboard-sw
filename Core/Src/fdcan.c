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
  hfdcan1.Init.NominalPrescaler = 5;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 14;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 5;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 14;
  hfdcan1.Init.DataTimeSeg2 = 2;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  // TODO: Config filters
  HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);
  HAL_FDCAN_Start(&hfdcan1);

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
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
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

/**
 * @brief Get CAN DLC value from the payload size
 *
 * @param size The size of the payload in bytes
 * 
 * @return int32_t The DLC or negative error code
 */
int32_t _can_get_dlc_from_size(size_t size) {
    switch(size) {
        case 0U:
            return FDCAN_DLC_BYTES_0;
        case 1U:
            return FDCAN_DLC_BYTES_1;
        case 2U:
            return FDCAN_DLC_BYTES_2;
        case 3U:
            return FDCAN_DLC_BYTES_3;
        case 4U:
            return FDCAN_DLC_BYTES_4;
        case 5U:
            return FDCAN_DLC_BYTES_5;
        case 6U:
            return FDCAN_DLC_BYTES_6;
        case 7U:
            return FDCAN_DLC_BYTES_7;
        case 8U:
            return FDCAN_DLC_BYTES_8;
        default:
            return -1;
    }
};

/**
 * @brief Get CAN TxFrameType value from the CanFrameType enum
 *
 * @param type The frame type enum value
 * 
 * @return int32_t The frame type or negative error code
 */
int32_t _can_get_frame_typename_from_frame_type(CanFrameType type) {
    switch (type) {
        case CAN_FRAME_TYPE_DATA:
            return FDCAN_DATA_FRAME;
        case CAN_FRAME_TYPE_REMOTE:
            return FDCAN_REMOTE_FRAME;
        default:
            return -1;
    }
}

/**
 * @brief Get the canFrameType enum value from the CAN TxFrameType
 *
 * @param typename The CAN frame type value
 * 
 * @return CanFrameType The frame type enum value or negative error code
 */
CanFrameType _can_get_frame_type_from_fram_typename(uint32_t typename) {
    switch (typename) {
        case FDCAN_DATA_FRAME:
            return CAN_FRAME_TYPE_DATA;
        case FDCAN_REMOTE_FRAME:
            return CAN_FRAME_TYPE_REMOTE;
        default:
            return -1;
    }
}

// TODO: Return and check errors
CanCommReturnCode can_send(
    can_id_t id,
    CanFrameType frame_type,
    const uint8_t * data,
    size_t size)
{
    if (id > CAN_COMM_ID_MASK)
        return CAN_COMM_INVALID_INDEX;

    // Get and check for data length
    int32_t dlc = _can_get_dlc_from_size(size);
    if (dlc < 0)
        return CAN_COMM_INVALID_PAYLOAD_SIZE;

    // Get and check the frame type
    int32_t type = _can_get_frame_typename_from_frame_type(frame_type);
    if (type < 0)
        return CAN_COMM_INVALID_FRAME_TYPE;
 
    // Setup transmission header
    const FDCAN_TxHeaderTypeDef header = {
        .Identifier = id,
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = type,
        .DataLength = dlc,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
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
    uint8_t data[CAN_COMM_MAX_PAYLOAD_BYTE_SIZE];
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, data) != HAL_OK)
        Error_Handler();
    
    CanFrameType frame_type = _can_get_frame_type_from_fram_typename(header.RxFrameType);
    if (frame_type < 0)
        return;

    // Update rx data
    can_comm_rx_add(
        bms_index_from_id(header.Identifier),
        frame_type,
        data,
        header.DataLength
    );
}

// TODO: Return and check errors
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef * hfdcan, uint32_t RxFifo1ITs) {
    UNUSED(hfdcan);
    UNUSED(RxFifo1ITs);
}

/* USER CODE END 1 */
