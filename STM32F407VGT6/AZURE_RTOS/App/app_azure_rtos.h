
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.h
  * @author  MCD Application Team
  * @brief   azure_rtos application header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#ifndef APP_AZURE_RTOS_H
#define APP_AZURE_RTOS_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_azure_rtos_config.h"
#include "tx_api.h"
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void tx_kprintf(const char *fmt, ...);

void *tx_malloc(size_t size);

void tx_free(void *ptr);

void TX_Delay(uint32_t Delay);

/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* APP_AZURE_RTOS_H */
