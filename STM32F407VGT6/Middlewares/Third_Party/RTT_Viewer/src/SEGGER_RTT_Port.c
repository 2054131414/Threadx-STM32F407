/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : SEGGER_RTT_Port.c
  * @brief          : RTT serial port printing secondary package
  ******************************************************************************
  * @attention      Open the cantaloupe before using your macro definition
  *
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "SEGGER_RTT_Port.h"

/**
 * @param  num:0-15
 * @param  sFormat
 * @return r
 */
#if RTT_USER_NUM_ENABLE

static int set_num=-1;
int SEGGER_RTT_printf_num(unsigned char num, const char * sFormat, ...)
{
    if (num > 15)
    {
        num = 0;
    }

    if (set_num!=num) {
        set_num = num;
        SEGGER_RTT_SetTerminal(set_num);
    }


    int r;
    va_list ParamList;

    va_start(ParamList, sFormat);
    r = SEGGER_RTT_vprintf(RTT_DBG_PORT, sFormat, &ParamList);
    va_end(ParamList);
    return r;

}

#endif
