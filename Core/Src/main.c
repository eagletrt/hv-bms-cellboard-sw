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

#include "stm32g4xx_it.h"
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

#ifdef CONF_DEMO_ENABLE

_STATIC void demo() {
    usart_log("\033[H");

    const cells_volt_t * const volt_values = volt_get_values();

    usart_log("=== VOLT VALUES ===\r\n");

    usart_log("   ");
    for (size_t i = 0U; i < 6U; ++i) 
        usart_log("%5d", i);

    usart_log("\r\n");
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_SERIES_COUNT / 6U; ++i) {
        usart_log("%d  ", i);
        for (size_t j = 0U; j < 6U; ++j) { 
            usart_log("%5.02f V ", (*volt_values)[i * 6U + j]);
        }
        usart_log("\r\n");
    }
    usart_log("\r\n\r\n");

    const cells_temp_t * const temp_values = temp_get_values();

    usart_log("=== TEMPERATURE VALUES ===\r\n");

    for (size_t i = 0U; i < 8U; ++i) 
        usart_log("  %7d", i);

    usart_log("\r\n");
    for (size_t i = 0U; i < CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT / 8U; ++i) {
        usart_log("%d  ", i);
        for (size_t j = 0U; j < 8U; ++j) {
            usart_log("%9.02f C", (*temp_values)[i * 8U + j]);
        }
        usart_log("\r\n");
    }
    usart_log("\r\n\r\n");

    // const raw_temp_t * discharge_temp_values = temp_get_values();
    //
    // usart_log("=== DISCHARGE TEMPERATURE VALUES ===\n");
    // for(size_t i = 0U; i < CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT; ++i) {
    //     usart_log("%d\n", discharge_temp_values[i]);
    // }
    // usart_log("\n\n");

    static bit_flag32_t cells = 1U;
    static uint32_t t = 0U;

    if (HAL_GetTick() - t >= 250U) {
        bms_manager_set_discharge_cells(cells);
        cells = (cells << 1U) & 0xFFFFFF;
        if (cells == 0U)
            cells = 1U;
        t = HAL_GetTick();
    }
}

#endif

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
  while (1)
  {
    fsm_state = fsm_run_state(fsm_state, NULL);

#ifdef CONF_DEMO_ENABLE
    _STATIC uint32_t t = 0U;
    if (HAL_GetTick() - t >= 250U) {
        demo();
        t = HAL_GetTick();
    }
#endif
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

void system_reset(void) {
    HAL_NVIC_SystemReset();
}

#ifdef CONF_FULL_ASSERT_ENABLE

/**
 * @brief Debug function called when an assertion fails
 *
 * @param file The file where the assert failed
 * @param line The line where the assert failed
 */
void cellboard_assert_failed(const char * file, const int line) {
    CELLBOARD_UNUSED(file);
    CELLBOARD_UNUSED(line);
}

#endif // CONF_FULL_ASSERT_ENABLE

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
