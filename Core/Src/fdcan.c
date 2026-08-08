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

#include "eagletrt.h"
#include "eagletrt-api.h"

#include "can-communication.h"
#include "can-communication-api.h"

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void) {

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
    if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN FDCAN1_Init 2 */

    // TODO: Config filters
    FDCAN_FilterTypeDef f1 = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = 0,
        .FilterID2 = 0x7FF
    };
    HAL_FDCAN_ConfigFilter(&HCAN_BMS, &f1);
    HAL_FDCAN_ActivateNotification(&HCAN_BMS, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);

    // FDCAN_FilterTypeDef f2 = {
    //     .IdType = FDCAN_STANDARD_ID,
    //     .FilterIndex = 1,
    //     .FilterType = FDCAN_FILTER_RANGE,
    //     .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
    //     .FilterID1 = 0x550,
    //     .FilterID2 = 0x7FF
    // };
    // HAL_FDCAN_ConfigFilter(&HCAN_BMS, &f2);
    // HAL_FDCAN_ActivateNotification(&HCAN_BMS, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U);

    HAL_FDCAN_Start(&HCAN_BMS);

    /* USER CODE END FDCAN1_Init 2 */
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *fdcanHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (fdcanHandle->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspInit 0 */

        /* USER CODE END FDCAN1_MspInit 0 */

        /** Initializes the peripherals clocks
  */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
            Error_Handler();
        }

        /* FDCAN1 clock enable */
        __HAL_RCC_FDCAN_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
        GPIO_InitStruct.Pin = CAN_RX_Pin | CAN_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* FDCAN1 interrupt Init */
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE BEGIN FDCAN1_MspInit 1 */

        /* USER CODE END FDCAN1_MspInit 1 */
    }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *fdcanHandle) {

    if (fdcanHandle->Instance == FDCAN1) {
        /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

        /* USER CODE END FDCAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_FDCAN_CLK_DISABLE();

        /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
        HAL_GPIO_DeInit(GPIOA, CAN_RX_Pin | CAN_TX_Pin);

        /* FDCAN1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
        /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

        /* USER CODE END FDCAN1_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/*!
 * \brief Returns the native ST HAL CAN handler based on the network enum.
 * \param[in] network The target network track enum.
 * \return Pointer to the matched global CAN_HandleTypeDef, or \c NULL if invalid.
 */
EAGLETRT_STATIC_INLINE FDCAN_HandleTypeDef *prv_fdcan_get_handler(enum CanCommunicationNetwork network) {
    switch (network) {
        case CAN_COMMUNICATION_NETWORK_BMS:
            return &HCAN_BMS;
        default:
            return NULL;
    }
}

/*!
 * \brief Internal unified helper to write an abstract frame out to an ST HAL CAN peripheral.
 * \param[in] network The network track enum indicating which hardware peripheral to target.
 * \param[in] frame Pointer to the abstract frame structure containing the payload.
 *
 * \retval CAN_COMMUNICATION_RC_OK if the frame was sent successfully.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if a required pointer configuration is \c NULL.
 * \retval CAN_COMMUNICATION_RC_INVALID_LENGTH if the frame length exceeds CAN_COMMUNICATION_FRAME_DATA_SIZE.
 * \retval CAN_COMMUNICATION_RC_TRANSMISSION_ERROR if the native HAL layer rejects the transmission.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_fdcan_send_to_hardware(enum CanCommunicationNetwork network, const struct CanCommunicationFrame *frame) {
    FDCAN_HandleTypeDef *hcan = prv_fdcan_get_handler(network);

    if (hcan == NULL || frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }
    if (frame->length > CAN_COMMUNICATION_FRAME_DATA_SIZE) {
        return CAN_COMMUNICATION_RC_INVALID_LENGTH;
    }

    // Setup transmission header
    const FDCAN_TxHeaderTypeDef tx_header = {
        .Identifier = frame->id,
        .IdType = FDCAN_STANDARD_ID,
        .TxFrameType = FDCAN_DATA_FRAME,
        .DataLength = frame->length,
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
        .BitRateSwitch = FDCAN_BRS_OFF,
        .FDFormat = FDCAN_CLASSIC_CAN,
        .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
        .MessageMarker = 0U
    };
    // Send message
    if (HAL_FDCAN_AddMessageToTxFifoQ(&HCAN_BMS, &tx_header, frame->data) != HAL_OK) {
        return CAN_COMMUNICATION_RC_TRANSMISSION_ERROR;
    }
    return CAN_COMMUNICATION_RC_OK;
}

enum CanCommunicationReturnCode fdcan_send_bms(const struct CanCommunicationFrame *frame) {
    return prv_fdcan_send_to_hardware(CAN_COMMUNICATION_NETWORK_BMS, frame);
}

// TODO: Return and check errors
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if (hfdcan->Instance != HCAN_BMS.Instance) {
        return;
    }
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == RESET) {
        return;
    }

    FDCAN_RxHeaderTypeDef header = { 0 };
    struct CanCommunicationFrame frame = { 0 };
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, frame.data) == HAL_OK) {
        frame.id = header.Identifier;
        frame.length = header.DataLength;

        // Based on the handler, retrieve the selected network
        constexpr enum CanCommunicationNetwork network = CAN_COMMUNICATION_NETWORK_BMS;

        /* TODO: Handle return value of RX function */
        EAGLETRT_API_UNUSED(can_communication_api_add_to_rx(network, &frame));
    }
    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

// TODO: Return and check errors
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs) {
    UNUSED(hfdcan);
    UNUSED(RxFifo1ITs);
}

/* USER CODE END 1 */
