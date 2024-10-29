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
#include "can.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bmsCommands.h"
#include "canCommunication.h"
#include "structs.h"
#include "flashStorage.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

BmsStatus status = { .cellVoltages = { 0 }, 		// Stores all cell voltages
		.minVoltage = 50000,		// Set higher than maximum possible voltage
		.maxVoltage = 0,			// Set lower than minimum possible voltage
		.packCurrent = 0,					// Initialise at 0mA
		.packCurrentCounter = 0,			// Initialise at 0As
		.cellTemperatures = { 1, 2, 3, 4, 5, 6, 7, 8 },	// Stores all cell temperatures
		.maxTemp = 0,						// Initialise at 0
		.maxTemp = 5000,						// Initialise at 500 degrees
		.stmTemperature = 0,				// Internal STM thermistor
		.passiveBalancingActive = 0,// Passive balancing is not active on startup
		.cellPassiveBalancingFlags = 0,		// No cells are balancing on startup
		.numCellsBalancing = 0,			// Number of cells currently balancing
		.cellOverVoltage = 0,				// Reset Over Voltage Error
		.cellUnderVoltage = 0,				// Reset Under Voltage Error
		.cellShutdownTemperature = 0,		// Reset Shutdown Temperature Error
		.cellWarningTemperature = 0,		// Reset Warning Temperature Error
		.pbIcWatchdogTimeout = 0,		// Reset PB IC Watchdog Timeout Error
		};

// Initialize an instance of BmsConfig
BmsConfig config = { .numberOfCells = 18,// BMS is designed to have 18 cells on it
		.numberOfThermistors = 8,// There are 8 thermistors wired to the STM ADCs
		.overVoltageMV = 42000,// HV-LiPo (M025) are 4.35V, LiCoO2 (<2024) are 4.2V
		.underVoltageMV = 32000,			// All LiPo are 3.2V
		.overVoltageMV = 42000,			// All LiPo are 4.2V
		.shutdownTemperature = 60,	// Rules dictate max accumulator temp is 60C
		.warningTemperature = 50,			// Chosen semi-arbitrarily
		.passiveBalanceEnabled = 0,			// Disable by default
		.passiveBalanceThreshold = 20,		// 2mV
		};

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
	HAL_Delay(250); // Delay To ensure Motec & rest of car ready
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM14_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	// Janky Startup Blinking
	for (uint8_t blinks = 0; blinks < 5; blinks++) {
		HAL_GPIO_WritePin(CAN_OK_GPIO_Port, CAN_OK_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin, GPIO_PIN_RESET);
		HAL_Delay(100); // Delay To ensure Motec & rest of car ready
		HAL_GPIO_WritePin(CAN_OK_GPIO_Port, CAN_OK_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin, GPIO_PIN_SET);
		HAL_Delay(100); // Delay To ensure Motec & rest of car ready
	}
	// Ensure they're off
	HAL_GPIO_WritePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CAN_OK_GPIO_Port, CAN_OK_Pin, GPIO_PIN_RESET);

  BMS_InitConfigRegisters();
	CAN_SendBmsConfiguration();
	CAN_SendBalanceConfiguration();

	// Load previous energy values
	FLASH_CheckForSegmentCapacity();

	// Begin Timers
	HAL_TIM_Base_Start_IT(&htim1);  // Get BMS Voltages
	HAL_TIM_Base_Start_IT(&htim3);  // Send CAN Voltages
	HAL_TIM_Base_Start_IT(&htim14); // Get BMS Temperatures && Other shit
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1) {
		// Check if 1 second has passed, if so save energies to flash
//		uint32_t currentTime = HAL_GetTick();
//		if (currentTime - lastSaveTime >= 1000) {
//			writeToFlash(cellRemainingEnergy, 20);
//			lastSaveTime = currentTime;
//		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
	while (1) {
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
