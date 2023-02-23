/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : shell_port.c
  * @brief          : shell user export
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 Letter.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "shell_port.h"

/* Private includes ----------------------------------------------------------*/
#include "shell.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern SHELL_TypeDef shell;
char shell_buffer[512];

/* Private function prototypes -----------------------------------------------*/

void User_Shell_Write(const char ch)
{
	//调用STM32 HAL库 API 使用查询方式发送
	//HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFFFF);
}

//char User_Shell_Read()
//{
//    return 
//}

void User_Shell_Init(void)
{
    shell.write = User_Shell_Write;
	//shell.read  = User_Shell_Read；

    shellInit(&shell);
}
 
 