/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "st7789.h"
#include "display.h"

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
  MX_USART2_UART_Init();
  MX_SPI1_Init();

  /* USER CODE BEGIN 2 */
  ST7789_Fill_Color(BLACK); // Wipe any garbage
  ST7789_Init();
  Display_Init();

  // Status message test
  Display_SetStatus("Calibrating...");
  HAL_Delay(2000);
  Display_SetStatus("Ready");
  HAL_Delay(1000);
  Display_SetStatus("Resetting...");
  HAL_Delay(1000);
  Display_SetStatus("Error");
  HAL_Delay(1000);
  Display_ClearStatus();
  HAL_Delay(500);

  uint32_t step_count = 0;
  Display_UpdateSteps(step_count);
  Display_UpdatePace(PACE_STATIONARY);

  // Phase control
  #define PHASE_STATIONARY_MS  10000   // 15s slow
  #define PHASE_WALKING_MS     10000   // 15s medium 
  #define PHASE_RUNNING_MS     10000   // 15s fast 

  uint32_t phase_start = HAL_GetTick();
  uint8_t  phase = 0;  // 0=stationary, 1=walking, 2=running
  uint32_t step_interval_ms = 3000;  // start slow

  static uint32_t step_times[100];
  static uint8_t  step_count_window = 0;

  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
      uint32_t now = HAL_GetTick();

      // Switch phase based on elapsed time
      uint32_t phase_elapsed = now - phase_start;

      if (phase == 0 && phase_elapsed >= PHASE_STATIONARY_MS)
      {
          phase = 1;
          step_interval_ms = 600;  // ~100 spm = Walking
          phase_start = HAL_GetTick();
      }
      else if (phase == 1 && phase_elapsed >= PHASE_WALKING_MS)
      {
          phase = 2;
          step_interval_ms = 375;   // ~160 spm = Running
          phase_start = HAL_GetTick();
      }
      else if (phase == 2 && phase_elapsed >= PHASE_RUNNING_MS)
      {
          phase = 0;
          step_interval_ms = 3000;  // ~20 spm = Stationary
          phase_start = HAL_GetTick();
      }

      HAL_Delay(step_interval_ms);

      // Add new step timestamp
      step_count++;
      now = HAL_GetTick();

      if (step_count_window < 100)
          step_times[step_count_window++] = now;

      // Discard steps outside rolling 10s window
      uint8_t i = 0;
      while (i < step_count_window)
      {
          if ((now - step_times[i]) > 10000)
          {
              for (uint8_t j = i; j < step_count_window - 1; j++)
                  step_times[j] = step_times[j + 1];
              step_count_window--;
          }
          else i++;
      }

      // Calculate SPM — dynamic rolling window (actual span between first and last step)
      float spm = 0.0f;
      if (step_count_window >= 2)
      {
          uint32_t oldest = step_times[0];
          uint32_t newest = step_times[step_count_window - 1];
          uint32_t span_ms = newest - oldest;

          if (span_ms > 0)
              spm = (step_count_window - 1) * 60000.0f / span_ms;
      }

      // Classify using realistic thresholds
      uint8_t pace;
      if (spm < 40.0f)
          pace = PACE_STATIONARY;
      else if (spm <= 130.0f)
          pace = PACE_WALKING;
      else
          pace = PACE_RUNNING;

      Display_UpdateSteps(step_count);
      Display_UpdatePace(pace);
  /* USER CODE END 3 */
  }
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  while (1)
  {
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
