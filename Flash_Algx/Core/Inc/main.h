/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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
#define DEBUG_USART2_TX_Pin GPIO_PIN_2
#define DEBUG_USART2_TX_GPIO_Port GPIOA
#define DEBUG_USART2_RX_Pin GPIO_PIN_3
#define DEBUG_USART2_RX_GPIO_Port GPIOA
#define SPI6_CS_FLASH0_Pin GPIO_PIN_15
#define SPI6_CS_FLASH0_GPIO_Port GPIOA
#define SPI6_CS_FLASH1_Pin GPIO_PIN_7
#define SPI6_CS_FLASH1_GPIO_Port GPIOD
#define SPI6_SCK_FLASH_Pin GPIO_PIN_3
#define SPI6_SCK_FLASH_GPIO_Port GPIOB
#define SPI6_MISO_FLASH_Pin GPIO_PIN_4
#define SPI6_MISO_FLASH_GPIO_Port GPIOB
#define SPI6_MOSI_FLASH_Pin GPIO_PIN_5
#define SPI6_MOSI_FLASH_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
