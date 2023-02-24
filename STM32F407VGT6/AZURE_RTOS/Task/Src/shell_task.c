/**
  ******************************************************************************
  * File Name          : shell_task.c
  * Description        : shell task thread
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

/* Private includes ----------------------------------------------------------*/
#include "shell_task.h"

/* Private includes ----------------------------------------------------------*/
#include "nr_micro_shell.h"
#include "app_azure_rtos.h"
#include "bsp_uart.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

void tx_hw_console_output(const char *str)
{
		size_t i = 0, size = 0;
		char a = '\r';
	
		size = strlen(str);
	
		for (i = 0; i < size; i++)
		{
				if (*(str + i) == '\n')
				{
						HAL_UART_Transmit(&SHELL_PORT, (uint8_t *)&a, 1, 1);
				}
				HAL_UART_Transmit(&SHELL_PORT, (uint8_t *)(str + i), 1, 1);
		}
}

char tx_hw_console_getchar(void)
{
		int ch = -1;
		
		if (__HAL_UART_GET_FLAG(&SHELL_PORT, UART_FLAG_RXNE) != RESET)
		{
				ch = SHELL_PORT.Instance->DR & 0xff;
    }
    else
    {
        if(__HAL_UART_GET_FLAG(&SHELL_PORT, UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&SHELL_PORT);
        }
        tx_thread_mdelay(10);
    }
		return ch;
}

/**
  * @brief  Nr shell get char
  * @param  None
  * @retval Console str
  */
static int nr_shell_getchar(void)
{
		return tx_hw_console_getchar();
}

/**
  * @brief  Shell thread entry
  * @param  thread input
  * @retval None
  */
void shell_thread_entry(ULONG thread_input)
{
    char ch;
    
		shell_init();
	
    while(1)
    {
        ch = nr_shell_getchar();
			  
			  if (ch == '\r')
					shell('\n');
    }
}

