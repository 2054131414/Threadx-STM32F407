
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_azure_rtos.c
  * @author  MCD Application Team
  * @brief   azure_rtos application implementation file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_azure_rtos.h"

/* Private includes ----------------------------------------------------------*/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "module_init.h"
#include "main_task.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TX_BYTE_POOL byte_pool;
static TX_THREAD mainthread;
static TX_MUTEX tx_printf;
char mem_pool[TX_APP_MEM_POOL_SIZE];

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Define the initial system.
 * @param  first_unused_memory : Pointer to the first unused memory
 * @retval None
*/
VOID tx_application_define(VOID *first_unused_memory)
{
    UINT ret = TX_SUCCESS;
    
    CHAR *pointer = TX_NULL;
    
    TX_THREAD_NOT_USED(first_unused_memory);
    
    /* Create a byte memory pool from which to allocate the thread stacks.  */
    tx_byte_pool_create(&byte_pool, "byte pool", (VOID *)mem_pool, TX_APP_MEM_POOL_SIZE);
    
    /* Allocate the stack for thread 0.  */
    tx_byte_allocate(&byte_pool, (VOID **)&pointer, TX_APP_MEM_POOL_SIZE, TX_NO_WAIT);

    /* Create the main thread.  */
    tx_thread_create(&mainthread, "main thread", main_thread_entry, 0, pointer, TX_APP_MEM_POOL_SIZE, 1, 1, TX_NO_TIME_SLICE,
                     TX_AUTO_START);

    /* Create mutually exclusive semaphores */
    tx_mutex_create(&tx_printf,"tx printf",TX_NO_INHERIT);

}

/**
  * @brief  TX application printf
  * @param  The content of printf
  * @retval None
  */
void tx_kprintf(const char *fmt, ...)
{
  char buf_str[200 + 1];
  va_list v_args;
  
  va_start(v_args, fmt);
  (void)vsnprintf((char *)&buf_str[0], (size_t)sizeof(buf_str), (char const *)fmt, v_args);
  va_end(v_args);

  tx_mutex_get(&tx_printf, TX_WAIT_FOREVER);
  printf("%s", buf_str);
  tx_mutex_put(&tx_printf);
}

/**
  * @brief  Application Delay function.
  * @param  Delay : number of ticks to wait
  * @retval None
  */
void tx_delay_tick(uint32_t Delay)
{
  UINT initial_time = tx_time_get();
  while ((tx_time_get() - initial_time) < Delay);
}

