/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_key.h
  * @brief          : hardware underlying driver initialization
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_KEY_H_
#define _BSP_KEY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "multi_button.h"

/* Exported defines ----------------------------------------------------------*/
typedef enum KEY_STATE_TYPE
{
	KEY_NO_STATUS,

	KEY1_DOWN,
	KEY1_LONG,
    
	KEY2_DOWN,
	KEY2_LONG,

	KEY3_DOWN,
	KEY3_LONG,

	KEY4_DOWN,
	KEY4_LONG,

	KEY5_DOWN,
	KEY5_LONG,    

	KEY6_DOWN,
	KEY6_LONG,
    
}key_state_t;

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
uint32_t Key_Scan(void);
    
#ifdef __cplusplus
}
#endif

#endif /* _BSP_KEY_H_ */
