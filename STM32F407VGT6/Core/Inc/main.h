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
#include "stm32f4xx_hal.h"

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

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY6_Pin GPIO_PIN_2
#define KEY6_GPIO_Port GPIOE
#define KEY5_Pin GPIO_PIN_3
#define KEY5_GPIO_Port GPIOE
#define KEY4_Pin GPIO_PIN_4
#define KEY4_GPIO_Port GPIOE
#define KEY3_Pin GPIO_PIN_5
#define KEY3_GPIO_Port GPIOE
#define KEY2_Pin GPIO_PIN_6
#define KEY2_GPIO_Port GPIOE
#define KEY1_Pin GPIO_PIN_13
#define KEY1_GPIO_Port GPIOC
#define LED_G2_Pin GPIO_PIN_0
#define LED_G2_GPIO_Port GPIOC
#define LED_G1_Pin GPIO_PIN_2
#define LED_G1_GPIO_Port GPIOC
#define LED_R_Pin GPIO_PIN_3
#define LED_R_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOD
#define LCD_RST_Pin GPIO_PIN_12
#define LCD_RST_GPIO_Port GPIOD
#define LCD_RS_Pin GPIO_PIN_13
#define LCD_RS_GPIO_Port GPIOD
#define LCD_SCK_Pin GPIO_PIN_14
#define LCD_SCK_GPIO_Port GPIOD
#define LCD_MOSI_Pin GPIO_PIN_8
#define LCD_MOSI_GPIO_Port GPIOC
#define LCD_LEDA_Pin GPIO_PIN_15
#define LCD_LEDA_GPIO_Port GPIOA
#define SPI1_CLK_Pin GPIO_PIN_3
#define SPI1_CLK_GPIO_Port GPIOB
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOB
#define RELAY_1_Pin GPIO_PIN_7
#define RELAY_1_GPIO_Port GPIOB
#define RELAY_2_Pin GPIO_PIN_8
#define RELAY_2_GPIO_Port GPIOB
#define RELAY_3_Pin GPIO_PIN_9
#define RELAY_3_GPIO_Port GPIOB
#define RELAY_4_Pin GPIO_PIN_0
#define RELAY_4_GPIO_Port GPIOE
#define RELAY_5_Pin GPIO_PIN_1
#define RELAY_5_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
