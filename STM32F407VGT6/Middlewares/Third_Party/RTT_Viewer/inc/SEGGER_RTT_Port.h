/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : SEGGER_RTT_Port.h
  * @brief          : RTT serial port printing secondary package
  ******************************************************************************
  * @attention      Open the cantaloupe before using your macro definition
  *
  */
/* USER CODE END Header */
#ifndef _SEGGER_RTT_PORT_H_
#define _SEGGER_RTT_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "SEGGER_RTT.h"

/* Exported defines ----------------------------------------------------------*/
#if RTT_DBG_ENABLE
    /* Select Promise Interface */
    #define DEBUG_NUM(num)  SEGGER_RTT_SetTerminal(num);
    /* Initialize the debug module */
    #define DEBUG_INIT()    SEGGER_RTT_Init()
    /* RTT Terminal number */
    #define RTT_DBG_PORT        0
    #define LOG_PROTO(num,type,color,format,...)                        \
            SEGGER_RTT_printf_num(num,"%s%s"format"\r\n%s",             \
                            color,                                      \
                            type,                                       \
                            ##__VA_ARGS__,                              \
                            RTT_CTRL_RESET)
    #define LOG_PROTO_1(num,type,color,format,...)                      \
            SEGGER_RTT_printf_num(num,"%s%s"format"%s",                 \
                            ##__VA_ARGS__,                              \
                            RTT_CTRL_RESET)
    /* clear the screen*/
    #define log_clear()     SEGGER_RTT_WriteString(RTT_DBG_PORT, "  "RTT_CTRL_CLEAR)
    /* no color log output */
    #define log_debug(num,format,...)   LOG_PROTO(num,"D:","",format,##__VA_ARGS__)
    /* color format log output */
    #define log_info(num,format,...)    LOG_PROTO(num,"I:", RTT_CTRL_TEXT_BRIGHT_GREEN , format, ##__VA_ARGS__)
    #define log_warn(num,format,...)    LOG_PROTO(num,"W:", RTT_CTRL_TEXT_BRIGHT_YELLOW, format, ##__VA_ARGS__)
    #define log_error(num,format,...)   LOG_PROTO(num,"E:", RTT_CTRL_TEXT_BRIGHT_RED   , format, ##__VA_ARGS__)
    #define log_printf(num,format,...)  LOG_PROTO_1(num,"","",format,##__VA_ARGS__)
    enum log_lever_typr
    {
        Log_log,      //base date
        Log_Debug,    //debug
        Log_mem,      //memory
        Log_context,  //kernel
        log_thread,   //thread
        log_com,      //com
        log_diaodu    //schedule
    };


#else
    #define DEBUG_NUM
    #define DEBUG_INIT()
    #define log_clear()
    #define log_debug
    #define log_info
    #define log_warn
    #define log_error
#endif

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int SEGGER_RTT_printf_num(unsigned char num, const char * sFormat, ...);

long list_thread(void);

#ifdef __cplusplus
}
#endif

#endif /* _SEGGER_RTT_PORT_H_ */
