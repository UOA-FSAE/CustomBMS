/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NTC_STM_MEAS_9_Pin GPIO_PIN_0
#define NTC_STM_MEAS_9_GPIO_Port GPIOA
#define AB_C1_EN_Pin GPIO_PIN_1
#define AB_C1_EN_GPIO_Port GPIOA
#define AB_DRIVE_EN_Pin GPIO_PIN_2
#define AB_DRIVE_EN_GPIO_Port GPIOA
#define AB_C2_EN_Pin GPIO_PIN_3
#define AB_C2_EN_GPIO_Port GPIOA
#define AB_C3_EN_Pin GPIO_PIN_4
#define AB_C3_EN_GPIO_Port GPIOA
#define AB_C4_EN_Pin GPIO_PIN_5
#define AB_C4_EN_GPIO_Port GPIOA
#define AB_C5_EN_Pin GPIO_PIN_6
#define AB_C5_EN_GPIO_Port GPIOA
#define AB_C6_EN_Pin GPIO_PIN_7
#define AB_C6_EN_GPIO_Port GPIOA
#define AB_C7_EN_Pin GPIO_PIN_0
#define AB_C7_EN_GPIO_Port GPIOB
#define STM_AUX_3_Pin GPIO_PIN_1
#define STM_AUX_3_GPIO_Port GPIOB
#define ABIC_FAULT_Pin GPIO_PIN_2
#define ABIC_FAULT_GPIO_Port GPIOB
#define AUX_SPI_SCK_Pin GPIO_PIN_10
#define AUX_SPI_SCK_GPIO_Port GPIOB
#define CAN_OK_Pin GPIO_PIN_12
#define CAN_OK_GPIO_Port GPIOB
#define CAN_ERROR_Pin GPIO_PIN_13
#define CAN_ERROR_GPIO_Port GPIOB
#define AUX_SPI_MISO_Pin GPIO_PIN_14
#define AUX_SPI_MISO_GPIO_Port GPIOB
#define AUX_SPI_MOSI_Pin GPIO_PIN_15
#define AUX_SPI_MOSI_GPIO_Port GPIOB
#define PBIC_TIMEOUT_Pin GPIO_PIN_8
#define PBIC_TIMEOUT_GPIO_Port GPIOA
#define STM_AUX_4_Pin GPIO_PIN_9
#define STM_AUX_4_GPIO_Port GPIOA
#define AB_EN_Pin GPIO_PIN_10
#define AB_EN_GPIO_Port GPIOA
#define STM_AUX_1_Pin GPIO_PIN_11
#define STM_AUX_1_GPIO_Port GPIOA
#define STM_AUX_2_Pin GPIO_PIN_12
#define STM_AUX_2_GPIO_Port GPIOA
#define BMS_CS_Pin GPIO_PIN_15
#define BMS_CS_GPIO_Port GPIOA
#define BMS_SPI_SCK_Pin GPIO_PIN_3
#define BMS_SPI_SCK_GPIO_Port GPIOB
#define BMS_SPI_MISO_Pin GPIO_PIN_4
#define BMS_SPI_MISO_GPIO_Port GPIOB
#define BMS_SPI_MOSI_Pin GPIO_PIN_5
#define BMS_SPI_MOSI_GPIO_Port GPIOB
#define BOOT0_Pin GPIO_PIN_11
#define BOOT0_GPIO_Port GPIOF

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
