/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_led.h
  * @brief          : hardware underlying driver initialization
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_LED_H_
#define _BSP_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ----------------------------------------------------------*/
#define LED_OFF     0

#define LED_ON      1

#define LED_BLINK   2

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
int BSP_Led_Init(void);

int32_t led_on(int32_t led_id);

int32_t led_off(int32_t led_id);

int32_t led_blink(int32_t led_id, uint32_t ms);

int32_t led_ctr(int32_t led_id, int8_t led_state);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_LED_H_ */
