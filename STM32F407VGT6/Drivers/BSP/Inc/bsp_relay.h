/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_relay.h
  * @brief          : Peripheral relay initialization
  ******************************************************************************
  * @attention       This function call in BSP
  *
  */
/* USER CODE END Header */
#ifndef _BSP_RELAY_H_
#define _BSP_RELAY_H_

#ifdef __cplusplus
extern "C"{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Exported defines ----------------------------------------------------------*/
#define RELAY_OFF     0
#define RELAY_ON      1
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int32_t BSP_Relay_Init(void);

int32_t relay_on(int32_t relay_id);

int32_t relay_off(int32_t relay_id);

int32_t relay_ctr(int32_t relay_id, int8_t relay_state);

#ifdef __cplusplus
}
#endif 

#endif /* _BSP_RELAY_H_ */ 
