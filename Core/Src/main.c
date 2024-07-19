/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fdcan.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "fsm.h"
#include "post.h"
#include "error.h"

#include "stm32g4xx_it.h"

#include "temp.h"

#include "bms-monitor-fsm.h"
#include "bms-manager.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void system_reset(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_FDCAN1_Init();
  MX_SPI3_Init();
  MX_USART2_UART_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  /**
   * Start the timer used to increment the timebase internal counter
   */
  HAL_TIM_Base_Start_IT(&HTIM_TIMEBASE);

  fsm_state_t fsm_state = FSM_STATE_INIT;

  // Prepare data for the POST procedure
  PostInitData init_data = {
      .system_reset = system_reset,
      .cs_enter = it_cs_enter,
      .cs_exit = it_cs_exit,
      .can_send = can_send,
      .spi_send = spi_send,
      .spi_send_receive = spi_send_and_receive,
      .led_set = gpio_led_set_state,
      .led_toggle = gpio_led_toggle_state,
      .gpio_set_address = gpio_set_mux_address,
      .adc_start = adc_temperature_start_conversion
  };
  
  // Read the cellboard ID from the ADC
  init_data.id = gpio_get_cellboard_id();

  fsm_state = fsm_run_state(fsm_state, &init_data);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t t = 0;
  size_t cnt = 1;
  size_t ltc = 0;

  // bms_manager_set_discharge_cells(0b100);
  while (1)
  {
    fsm_state = fsm_run_state(fsm_state, NULL);

    // Ltc6811Cfgr * requested = bms_manager_get_requested_config();
    // Ltc6811Cfgr * actual = bms_manager_get_actual_config();

    if (HAL_GetTick() - t >= 500) {
        // char msg[64] = { 0 };
        // sprintf(msg, "%d %d\r\n", poll_err_count, read_err_count);
        // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30); 

        // requested[0].GPIO = 0b11111;
        // requested[1].GPIO = 0b11111;
        // // if (cnt++ < 15) {
        // requested[ltc].DCC = cnt;
        // requested[ltc].DCTO = LTC6811_DCTO_120MIN;
        // cnt *= 2U;
        // if (cnt >= 1 << 13) {
        //     requested[ltc].DCC = 0;
        //     requested[ltc].DCTO = LTC6811_DCTO_OFF;
        //     ltc = !ltc;
        //     cnt = 1;
        // }
        // }
        // else {
        //     requested[0].DCC = 0;
        //     requested[0].DCTO = LTC6811_DCTO_OFF;
        // }
 
        // char msg[64] = { 0 };
        // sprintf(msg, "%u\r\n", fsm_get_status());
        // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);
 

     //    char msg[512] = { 0 };
     //    raw_volt_t ** values = bms_manager_get_pup();
     //    for (size_t i = 0; i < 2; ++i) {
     //        sprintf(msg, i ? "PUP: " : "PUD: ");
     //        for (size_t j = 0; j < CELLBOARD_SEGMENT_SERIES_COUNT; ++j) {
     //            sprintf(msg + strlen(msg), "%.3f ", values[i][j] / 10000.f);
     //        }
     //        sprintf(msg + strlen(msg), "\r\n");
     //        HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);
     //    }
     //    sprintf(msg, "\r\n");
     //    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);
     // 
        {
            char msg[512] = "\033[H";
            raw_volt_t * values = volt_get_values();
            for (size_t i = 0; i < CELLBOARD_SEGMENT_SERIES_COUNT; ++i) {
                sprintf(msg + strlen(msg), "%d. %.3f\r\n", i, values[i] / 10000.f);
            }
            // sprintf(msg + strlen(msg), "\r\n\r\n");
            HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);
        }
        // {
        //     char msg[8000] = "\033[H";
        //     raw_temp_t * values = temp_get_values();
        //     for (size_t ch = 0; ch < CELLBOARD_SEGMENT_TEMP_CHANNEL_COUNT; ++ch) {
        //         for (size_t sensor = 0; sensor < CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT; ++sensor) {
        //             size_t index = ch * CELLBOARD_SEGMENT_TEMP_SENSOR_PER_CHANNEL_COUNT + sensor;
        //             double value = values[index] / 4096.0 * 3.3;
        //             // sprintf(msg + strlen(msg), "%3.3f ", value);
        //
        //             celsius_t temp = temp_value_to_celsius(value);
        //             sprintf(msg + strlen(msg), "%4.2f ", temp);
        //         }
        //         sprintf(msg + strlen(msg), "@\r\n");
        //     }
        //     // sprintf(msg + strlen(msg), "\r\n\r\n");
        //     HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);
        // }
        // char msg[512] = { 0 };
        // raw_temp_t * values = temp_get_discharge_values();
        // for (size_t i = 0; i < CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT; ++i) {
        //     sprintf(msg + strlen(msg), "%d. %hu\r\n", i, values[i]);
        // }
        // sprintf(msg + strlen(msg), "\r\n\r\n");
        // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 30);

        // char msg[512] = "Requested:\r\n";
    //     for (size_t i = 0; i < 2; ++i) {
    //         sprintf(msg + strlen(msg), " - ADCOPT: %hu\r\n", requested[i].ADCOPT);
    //         sprintf(msg + strlen(msg), " - DTEN: %hu\r\n", requested[i].DTEN);
    //         sprintf(msg + strlen(msg), " - REFON: %hu\r\n", requested[i].REFON);
    //         sprintf(msg + strlen(msg), " - GPIO: %hu\r\n", requested[i].GPIO);
    //         sprintf(msg + strlen(msg), " - VUV: %hu\r\n", requested[i].VUV);
    //         sprintf(msg + strlen(msg), " - VOV: %hu\r\n", requested[i].VOV);
    //         sprintf(msg + strlen(msg), " - DCC: %hu\r\n", requested[i].DCC);
    //         sprintf(msg + strlen(msg), " - DCTO: %hu\r\n", requested[i].DCTO);
    //         sprintf(msg + strlen(msg), "\r\n");
    //     }
    //     HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 50);
    //
        // sprintf(msg, "Actual:\r\n");
        // for (size_t i = 0; i < 2; ++i) {
        //     sprintf(msg + strlen(msg), " - ADCOPT: %hu\r\n", actual[i].ADCOPT);
        //     sprintf(msg + strlen(msg), " - DTEN: %hu\r\n", actual[i].DTEN);
        //     sprintf(msg + strlen(msg), " - REFON: %hu\r\n", actual[i].REFON);
        //     sprintf(msg + strlen(msg), " - GPIO: %hu\r\n", actual[i].GPIO);
        //     sprintf(msg + strlen(msg), " - VUV: %hu\r\n", actual[i].VUV);
        //     sprintf(msg + strlen(msg), " - VOV: %hu\r\n", actual[i].VOV);
        //     sprintf(msg + strlen(msg), " - DCC: %hu\r\n", actual[i].DCC);
        //     sprintf(msg + strlen(msg), " - DCTO: %hu\r\n", actual[i].DCTO);
        //     sprintf(msg + strlen(msg), "\r\n");
        // }
        // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 50);
    
         t = HAL_GetTick();
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /*
     * while(1) {
     *     fetenderi = HIGH;
     *     bccanti = HIGH;
     * }
     *
     * */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

inline void system_reset(void) {
    HAL_NVIC_SystemReset();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) { }

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
