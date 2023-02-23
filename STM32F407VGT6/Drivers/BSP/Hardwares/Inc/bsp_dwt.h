/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_dwt.h
  * @brief          : DWT delay counter module
  ******************************************************************************
  * @attention       Use related functions for inference
  *
  */
/* USER CODE END Header */
#ifndef _BSP_DWT_H_
#define _BSP_DWT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_port.h"
#include "tx_api.h"
#include "stm32f4xx.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Coretex-M3 DWT register address defines */
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004 
#define  DWT_CR      *(volatile unsigned int *)0xE0001000 
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC 
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004 

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

/* Exported functions --------------------------------------------------------*/

/* DWT delay functions */
int BSP_InitDWT(void);

void bsp_DelayUS(uint32_t _ulDelayTime);

void bsp_DelayMS(uint32_t _ulDelayTime);

/* RTOS app delay function */
void App_Delay(uint32_t Delay);

/* software delay */
void delay_200ns(uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_DWT_H_ */
