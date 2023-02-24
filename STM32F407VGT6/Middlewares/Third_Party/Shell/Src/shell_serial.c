/**
  ******************************************************************************
  * File Name          : shell_serial.c
  * Description        : shell task thread
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "shell_serial.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include "app_azure_rtos.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define SHELL_BUF_SIZE              128
#define SHELL_UART                  BSP_UART2

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* serial helle omessage */
static const char* wellcom =
    SHELL_NEW_LINE
    "ThreadX-based Serial Terminal Shell" SHELL_NEW_LINE
    "Copyright (c) 2021, Author: xwiron, Email: xwiron@aliyun.com" SHELL_NEW_LINE
    "Type \"help\" to view a list of registered commands" SHELL_NEW_LINE
    SHELL_NEW_LINE;

/* serial shell context */
static shell_context_t shell_serial_context;
struct shell_command_sets_t shell_serial_command_sets;
struct shell_history_queue_t shell_serial_history;

/* Private function prototypes -----------------------------------------------*/

static void* shell_malloc(size_t size)
{
    return tx_malloc(size);
}

static void shell_free(void* ptr)
{
    tx_free(ptr);
}

static int shell_serial_printf(const char* fmt, ...)
{
    va_list args;
    int32_t len;
    char* buf;

    buf = shell_malloc(SHELL_BUF_SIZE + 1);

    if(buf == NULL)
        return 0;

    va_start(args, fmt);
    len = vsnprintf(buf, SHELL_BUF_SIZE, fmt, args);
    va_end(args);

    buf[len] = '\0';

   //bsp_uart_poll_tx(SHELL_UART, (uint8_t*)buf, len, TX_WAIT_FOREVER);

    shell_free(buf);

    return len;
}


/*----------------------------------------------------------------------------*/
static int shell_serial_puts(const char* str)
{
    int len;

    len = strlen(str);

   // bsp_uart_poll_tx(SHELL_UART, (uint8_t*)str, len, TX_WAIT_FOREVER);

    return len;
}

/*----------------------------------------------------------------------------*/
static int shell_serial_putchar(char ch)
{
    //bsp_uart_poll_tx(SHELL_UART, (uint8_t*)&ch, 1, TX_WAIT_FOREVER);

    return ch;
}

/*----------------------------------------------------------------------------*/
static int shell_serial_getchar(void)
{
    char data = EOF;

    //bsp_uart_it_rx(SHELL_UART, (uint8_t*)&data, 1, TX_WAIT_FOREVER);

    return data;
}

/*---------------------------------------------------------------------------*/
void shell_serial(struct shell_command_set_t* set)
{
    shell_context_t* shell = &shell_serial_context;
    shell->name            = "root";
    shell->shell_getchar   = shell_serial_getchar;
    shell->shell_putchar   = shell_serial_putchar;
    shell->shell_puts      = shell_serial_puts;
    shell->shell_printf    = shell_serial_printf;
    shell->shell_malloc    = shell_malloc;
    shell->shell_free      = shell_free;

    shell_init(shell, &shell_serial_command_sets, &shell_serial_history);

    if(set)
    {
        shell_command_set_register(shell->command_sets, set);
    }

    shell_serial_puts(SHELL_NEW_LINE "Shell init done, Press any key to continue..." SHELL_NEW_LINE);

    shell_serial_getchar();
    shell_serial_puts(wellcom);
    shell_output_prompt(shell);

    for(;;)
    {
        int data = shell_serial_getchar();

        if(data != EOF)
        {
            char* buf = shell_malloc(SHELL_BUF_SIZE + 1); // +1 '\0'

            if(buf != NULL)
            {
                buf[0] = data;
                buf[1] = '\0';

                shell_getline(shell, buf, SHELL_BUF_SIZE);

                shell_input(shell, buf);

                shell_free(buf);
            }
            else
            {
                /* shell malloc error */
                shell_serial_printf("Shell malloc menmory error.");
            }
        }

    }
}
