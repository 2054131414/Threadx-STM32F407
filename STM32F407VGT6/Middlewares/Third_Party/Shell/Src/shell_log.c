/**
  ******************************************************************************
  * @file    debug_log.c
  * @author  Iron
  * @date    2021-01-31
  * @version v1.0
  * @brief   debug log c file
  */

/** @addtogroup DEGBU_LOG
  * @{
  */

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "shell_log.h"
#include "bsp.h"

#if DEBUG_LOG_ENABLE

#ifndef LOG_LOCAL_LEVEL
    #define LOG_LOCAL_LEVEL     DBG_LOG_DEBUG
#endif

/* private typedef -----------------------------------------------------------*/
/* private define ------------------------------------------------------------*/
#define DBG_BUF_SIZE              128
#define DBG_LOG_UART              BSP_UART2

/* private macro -------------------------------------------------------------*/
/* private variables ---------------------------------------------------------*/
static char log_buf[DBG_BUF_SIZE];

/* private function prototypes -----------------------------------------------*/
/* private functions ---------------------------------------------------------*/

uint32_t debug_log_timestamp(void)
{
    return HAL_GetTick();
}

int debug_log_puts(const char *str)
{
    int len;

    len = strlen(str);

    //bsp_uart_poll_tx(DBG_LOG_UART, (uint8_t *)str, len, HAL_MAX_DELAY);

    return len;
}

void debug_log(dbg_log_level_t level, const char *tag, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(log_buf, DBG_BUF_SIZE - 1, format, args);  // Note: rt_vsnprintf not surpport uint64_t float.
    va_end(args);

    debug_log_puts(log_buf);
}

void debug_assert_failed(const char *file, uint32_t line)
{
    debug_log_puts(LOG_COLOR_E);
    debug_log_puts("Assert failed at: ");
    debug_log_puts(file);
    debug_log(DBG_LOG_ERROR, "", ":%d\r\n", line);

    Error_Handler(__FILE__, __LINE__);
}

void debug_print_hex(const uint8_t *data, int32_t datalen)
{
    int32_t i, len;

    for (i = 0, len = 0; i < datalen && len < (DBG_BUF_SIZE - 1); i++)
    {
        len += snprintf(&log_buf[len], DBG_BUF_SIZE - len, "%02X ", data[i]);
    }

    log_buf[len] = '\0';

    debug_log_puts(log_buf);

    debug_log_puts(_LINE_END_);
}

#endif // DEBUG_LOG_ENABLE
