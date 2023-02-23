/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : shell_ext.c
  * @brief          : shell extensions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 Letter.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __SHELL_EXT_H__
#define __SHELL_EXT_H__

/* Includes ------------------------------------------------------------------*/
#include "shell.h"


/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/**
 * @brief 数字类型
 *
 */
typedef enum
{
    NUM_TYPE_INT,                                           /**< 十进制整型 */
    NUM_TYPE_BIN,                                           /**< 二进制整型 */
    NUM_TYPE_OCT,                                           /**< 八进制整型 */
    NUM_TYPE_HEX,                                           /**< 十六进制整型 */
    NUM_TYPE_FLOAT                                          /**< 浮点型 */
} NUM_Type;

/* Exported functions --------------------------------------------------------*/
unsigned int shellExtParsePara(char* string);

int shellExtRun(shellFunction function, int argc, char* argv[]);

#endif
