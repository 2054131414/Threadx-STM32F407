/**
  ******************************************************************************
  * File Name          : task_main.c
  * Description        : main task thread
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
#include "main_task.h"

/* Private includes ----------------------------------------------------------*/
#include "module_init.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main thread entry
  * @param  thread input
  * @retval None
  */
void main_thread_entry(ULONG thread_input)
{
    /* 段注进入线程开启调度之之后 */
    TX_COMPONENTS_INIT();
}

