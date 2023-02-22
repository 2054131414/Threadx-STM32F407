/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp.h
  * @brief          : hardware underlying driver initialization
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_H_
#define _BSP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void System_Init(void);
void Error_Handler(char* file, uint32_t line);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_H_ */
