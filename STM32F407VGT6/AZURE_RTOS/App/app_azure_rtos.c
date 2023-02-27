
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
#include <stdarg.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "module_init.h"
#include "main_task.h"
#include "shell_task.h"
#include "gui_task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static TX_BYTE_POOL byte_pool;
static TX_THREAD mainthread;
static TX_THREAD shellthread;
static TX_THREAD guithread;
static TX_MUTEX tx_printf;
char  TX_MEM_POOL[TX_APP_MEM_POOL_SIZE];

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Define the initial system.
 * @param  first_unused_memory : Pointer to the first unused memory
 * @retval None
*/
VOID tx_application_define(VOID* first_unused_memory)
{
    UINT state;

    CHAR* pointer = TX_NULL;

    TX_THREAD_NOT_USED(first_unused_memory);

    /* Azure-RTOS components initialization. */
    TX_COMPONENTS_INIT();

    /* Create a byte memory pool from which to allocate the thread stacks. */
    state = tx_byte_pool_create(&byte_pool, "byte pool", (VOID*)TX_MEM_POOL, TX_APP_MEM_POOL_SIZE);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_POOL_ERROR;
    }

    /* Allocate the stack for main thread. */
    state = tx_byte_allocate(&byte_pool, (VOID**)&pointer, MAIN_THREAD_STACK_SIZE, TX_NO_WAIT);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_POOL_ERROR;
    }

    /* Create the main thread.  */
    state = tx_thread_create(&mainthread, "main thread", main_thread_entry, 0, pointer, MAIN_THREAD_STACK_SIZE, 1, 1, TX_NO_TIME_SLICE,
                             TX_AUTO_START);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_THREAD_ERROR;
    }
    
    /* Allocate the stack for gui thread. */
    state = tx_byte_allocate(&byte_pool, (VOID**)&pointer, GUI_THREAD_STACK_SIZE, TX_NO_WAIT);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_POOL_ERROR;
    }
    
    /* Create the gui thread.  */
    state = tx_thread_create(&guithread, "gui thread", gui_thread_entry, 0, pointer, GUI_THREAD_STACK_SIZE, 5, 5, TX_NO_TIME_SLICE,
                             TX_AUTO_START);
    
    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_THREAD_ERROR;
    }
    
    /* Allocate the stack for shell thread. */
    state = tx_byte_allocate(&byte_pool, (VOID**)&pointer, SHELL_THREAD_STACK_SIZE, TX_NO_WAIT);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_POOL_ERROR;
    }

    /* Create the shell thread.  */
    state = tx_thread_create(&shellthread, "shell thread", shell_thread_entry, 0, pointer, SHELL_THREAD_STACK_SIZE, 21, 21, TX_NO_TIME_SLICE,
                             TX_AUTO_START);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_THREAD_ERROR;
    }

    /* Create mutually exclusive semaphores */
    state = tx_mutex_create(&tx_printf, "tx printf", TX_NO_INHERIT);

    /* Check create state. */
    if(state != TX_SUCCESS)
    {
        state = TX_MUTEX_ERROR;
    }

}

/**
  * @brief  TX application printf
  * @param  The content of printf
  * @retval None
  */
void tx_kprintf(const char* fmt, ...)
{
    va_list v_args;
    size_t length;
    static char buf_str[TX_CONSOLEBUF_SIZE];

    va_start(v_args, fmt);
    length = vsnprintf(buf_str, sizeof(buf_str) - 1, fmt, v_args);
    if(length > TX_CONSOLEBUF_SIZE - 1)
        length = TX_CONSOLEBUF_SIZE - 1;

    tx_hw_console_output(buf_str);
    va_end(v_args);

    // tx_mutex_get(&tx_printf, TX_WAIT_FOREVER);
    // printf("%s", buf_str);
    // tx_mutex_put(&tx_printf);
}

/**
  * @brief  Application Delay function.
  * @param  Delay : number of ticks to wait
  * @retval None
  */
void TX_Delay(uint32_t Delay)
{
    UINT initial_time = tx_time_get();
    while((tx_time_get() - initial_time) < Delay);
}

/**
  * @brief  Request memory user secondary encapsulation.
  * @param  Request a memory size
  * @retval pointer
  */
void* tx_malloc(size_t size)
{
    void* pointer = NULL;
    UINT status;

    status = tx_byte_allocate(&byte_pool, (VOID**) &pointer, size, TX_NO_WAIT);

    if(status != TX_SUCCESS)
        return NULL;

    return pointer;
}

/**
  * @brief  Free up memory.
  * @param  ms the delay ms time
  * @retval tx state
  */
void tx_free(void* ptr)
{
    UINT status;

    status = tx_byte_release(ptr);

    if(status != TX_SUCCESS)
    {
        status = TX_POOL_ERROR;
    }
}


