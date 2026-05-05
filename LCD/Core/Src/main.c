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

/*
 * ============================================================
 * [LCD - DO NOT MODIFY]: This file is owned by the LCD Engineer.
 * Other team members must NOT edit this file.
 * ============================================================
 *
 * FILE: main.c
 * OWNER: LCD Engineer
 *
 * PURPOSE:
 *   Top-level entry point for the wearable step tracker firmware.
 *   Initialises all hardware peripherals, then runs the three
 *   startup stages in sequence before entering the infinite
 *   tracking loop.
 *
 * SYSTEM FLOW (power-on to running):
 *   1. HAL_Init()              — reset peripherals, configure Systick
 *   2. SystemClock_Config()    — set MCU clock to 16 MHz HSI
 *   3. MX_GPIO_Init()          — configure all GPIO pins
 *   4. MX_USART2_UART_Init()   — configure UART for debug output
 *   5. MX_SPI1_Init()          — configure SPI1 for the LCD
 *   6. ST7789_Init()           — bring up the 240x240 LCD display
 *   7. Stage_Calibration_Run() — Stage 1: 6-axis accelerometer calibration
 *   8. Stage_SelfTest_Run()    — Stage 2: ADXL335 self-test validation
 *   9. Stage_Tracker_Run()     — Stage 3: infinite step-tracking loop
 *
 * NOTE: Stage_Tracker_Run() never returns.  All code after it
 *       (Display_Init, status message tests, while(1)) is dead
 *       code that will never execute in normal operation.
 *
 * HOW OTHER ENGINEERS INTERACT WITH THIS FILE:
 *   - Do NOT modify this file.
 *   - If you need to run your own initialisation before the
 *     stages start, coordinate with the LCD Engineer.
 *   - Integrate your subsystem logic inside your own .c file
 *     by implementing the hooks described in each stage's .h file.
 * ============================================================
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include "display.h"
#include "stage_calibration.h"
#include "stage_selftest.h"
#include "stage_tracker.h"

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
  HAL_Init();  /* Must be the very first call — sets up the HAL tick timer and resets all peripherals */

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();  /* Sets HSI oscillator to 16 MHz, no PLL — see SystemClock_Config() below */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();          /* Configure all GPIO pins (button, LED, LCD control pins) per main.h */
  MX_USART2_UART_Init();   /* Configure USART2 on PA2/PA3 for serial debug — not actively used by firmware */
  MX_SPI1_Init();          /* Configure SPI1 — used exclusively to communicate with the ST7789 LCD */

  /* USER CODE BEGIN 2 */

  /* --- LCD Hardware Initialisation --- */
  ST7789_Fill_Color(BLACK); /* Clear any garbage pixels left over from reset before init runs */
  ST7789_Init();            /* Send full ST7789 init sequence: reset, COLMOD (16-bit), rotation, gamma */

  /* --- Stage 1: Accelerometer Calibration --- */
  /* Blocking call. Guides the user through 6 directional steps (+X, -X, +Y, -Y, +Z, -Z).
   * Each step shows a prompt on the LCD and waits for a button press.
   * [HOOK - Static Calibration Engineer]: Your ADC sampling code runs inside
   *   stage_calibration.c after each button press. See stage_calibration.h for details. */
  Stage_Calibration_Run();

  /* --- Stage 2: ADXL335 Self-Test --- */
  /* Blocking call. Reads baseline and self-test accelerometer voltages, compares deltas
   * against expected values (X:-325mV, Y:+325mV, Z:+550mV ±100mV).
   * If the test fails the device does a full hardware reset (NVIC_SystemReset) and
   * restarts from Stage 1.
   * [HOOK - Self-Test Engineer]: Replace the dummy float values and pass/fail logic
   *   inside stage_selftest.c with real ADC readings. See stage_selftest.h for details. */
  Stage_SelfTest_Run();

  /* --- Stage 3: Step Tracker --- */
  /* Infinite loop — this call NEVER returns.
   * Continuously reads step events, calculates steps-per-minute (SPM),
   * classifies pace (STATIONARY / WALKING / RUNNING), and updates the LCD.
   * [HOOK - Step Tracker Engineer]: Replace the simulation phase block inside
   *   stage_tracker.c with real accelerometer step detection. See stage_tracker.h. */
  Stage_Tracker_Run();

  /* ============================================================
   * DEAD CODE BELOW — Stage_Tracker_Run() never returns.
   * Everything from here to the end of main() is unreachable in
   * normal operation.  It is kept for reference / future use only.
   * ============================================================ */
  Display_Init();


  /* DEAD CODE: Status message test sequence — used during LCD development to verify
   * that Display_SetStatus() renders and clears correctly.  Unreachable at runtime. */
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

  /* USER CODE END 2 */

  /* DEAD CODE: Infinite loop — unreachable because Stage_Tracker_Run() loops forever above.
   * Kept as a safety catch in case Stage_Tracker_Run() is ever refactored to return. */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /*
     * [HOOK - Step Tracker Engineer]: If Stage_Tracker_Run() is ever refactored
     * to be non-blocking, your periodic step-check and display-update calls
     * would go here instead.
     */
  /* USER CODE END 3 */
 }
}

/**
  * @brief System Clock Configuration
  * @retval None
  *
  * Configures the MCU to run at 16 MHz using the internal HSI oscillator.
  * No PLL is used (PLLState = NONE), keeping power consumption low.
  * HCLK = SYSCLK = 16 MHz.  APB1 is divided by 2 → 8 MHz peripheral clock.
  * Voltage scaling set to Range 1 (full speed) to support 16 MHz operation.
  *
  * [LCD - DO NOT MODIFY]: Clock speed affects SPI timing for the LCD.
  * Changing this without also adjusting SPI prescaler in MX_SPI1_Init()
  * may corrupt the display.
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);  /* Range 1 = up to 64 MHz */

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
  *
  * Called automatically by HAL functions (e.g. HAL_RCC_OscConfig) when they
  * fail.  Disables all interrupts and spins forever — effectively freezes the
  * device.  The LED_GREEN pin (PA5) will stop toggling, which can help
  * identify a hard-fault during debug.
  *
  * [LCD - DO NOT MODIFY]: Do not remove or bypass this handler.
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();  /* Mask all maskable interrupts — no more Systick, no UART ISR */
  while (1)
  {
    /* Hang here forever so a debugger can catch the call stack */
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
