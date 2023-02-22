/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_uart.h
  * @brief          : hardware underlying driver initialization
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_UART_H_
#define _BSP_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int BSP_Uart_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_UART_H_ */
