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

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Shell thread entry
  * @param  thread input
  * @retval None
  */
void shell_thread_entry(ULONG thread_input)
{
    tx_kprintf("start main thread %d ...\r\n", thread_input);
    
    while(1)
    {
       tx_thread_sleep(100); 
    }
}

