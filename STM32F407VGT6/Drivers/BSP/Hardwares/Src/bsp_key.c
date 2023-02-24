/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  mingliang.sui
  * @brief   hardware underlying driver initialization
  ******************************************************************************
  * @attention
  * The function must be called before the scheduler is started
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_key.h"

/* Private includes ----------------------------------------------------------*/
#include "module_init.h"
#include "main.h"
#include "tx_api.h"

#include "bsp_led.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum Button_ID_TYPE
{
	button1_id,
	button2_id,
	button3_id,
	button4_id,
	button5_id,
	button6_id,
}button_id_t;

/* Private define ------------------------------------------------------------*/
// #define KEY6_Pin GPIO_PIN_2
// #define KEY6_GPIO_Port GPIOE

// #define KEY5_Pin GPIO_PIN_3
// #define KEY5_GPIO_Port GPIOE

// #define KEY4_Pin GPIO_PIN_4
// #define KEY4_GPIO_Port GPIOE

// #define KEY3_Pin GPIO_PIN_5
// #define KEY3_GPIO_Port GPIOE

// #define KEY2_Pin GPIO_PIN_6
// #define KEY2_GPIO_Port GPIOE

// #define KEY1_Pin GPIO_PIN_13
// #define KEY1_GPIO_Port GPIOC

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
struct Button btn1;
struct Button btn2;
struct Button btn3;
struct Button btn4;
struct Button btn5;
struct Button btn6;

/* Private function prototypes -----------------------------------------------*/

void BTN1_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN1_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

void BTN2_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN2_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

void BTN3_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN3_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

void BTN4_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN4_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

void BTN5_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN5_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

void BTN6_PRESS_DOWN_Handler(void* btn)
{
    led_on(0);
	//do something...
}
void BTN6S_PRESS_UP_Handler(void* btn)
{
    led_off(0);
	//do something...
}

uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case button1_id:
			return HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
		case button2_id:
			return HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin);
		case button3_id:
			return HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin);
		case button4_id:
			return HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin);
		case button5_id:
			return HAL_GPIO_ReadPin(KEY5_GPIO_Port, KEY5_Pin);
		case button6_id:
			return HAL_GPIO_ReadPin(KEY6_GPIO_Port, KEY6_Pin);
		default:
			return 0;
	}
}

int BSP_Key_Init(void)
{
    button_init(&btn1, read_button_GPIO, 1, button1_id);
	button_init(&btn2, read_button_GPIO, 1, button2_id);
    button_init(&btn3, read_button_GPIO, 1, button3_id);
	button_init(&btn4, read_button_GPIO, 1, button4_id);
    button_init(&btn5, read_button_GPIO, 1, button5_id);
	button_init(&btn6, read_button_GPIO, 1, button6_id);
    
	button_attach(&btn1, PRESS_DOWN, BTN1_PRESS_DOWN_Handler);
	button_attach(&btn1, PRESS_UP,   BTN1_PRESS_UP_Handler);

	button_attach(&btn2, PRESS_DOWN, BTN2_PRESS_DOWN_Handler);
	button_attach(&btn2, PRESS_UP,   BTN2_PRESS_UP_Handler);

	button_attach(&btn3, PRESS_DOWN, BTN2_PRESS_DOWN_Handler);
	button_attach(&btn3, PRESS_UP,   BTN2_PRESS_UP_Handler);    
    
	button_attach(&btn4, PRESS_DOWN, BTN2_PRESS_DOWN_Handler);
	button_attach(&btn4, PRESS_UP,   BTN2_PRESS_UP_Handler);    

	button_attach(&btn5, PRESS_DOWN, BTN2_PRESS_DOWN_Handler);
	button_attach(&btn5, PRESS_UP,   BTN2_PRESS_UP_Handler);    

	button_attach(&btn6, PRESS_DOWN, BTN2_PRESS_DOWN_Handler);
	button_attach(&btn6, PRESS_UP,   BTN2_PRESS_UP_Handler);

	button_start(&btn1);
	button_start(&btn2);
	button_start(&btn3);
	button_start(&btn4);
	button_start(&btn5);
	button_start(&btn6);
    
    return 0;
}
TX_INIT_DEVICE_EXPORT(BSP_Key_Init);
