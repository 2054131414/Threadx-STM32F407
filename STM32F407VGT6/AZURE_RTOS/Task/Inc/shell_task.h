/**
  ******************************************************************************
  * @file    shell_task.h
  * @brief   This file contains all the function prototypes for
  *          the shell_task.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHELL_TASK_H__
#define __SHELL_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_azure_rtos.h"

/* Exported variables --------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define SHELL_PORT huart5

/* Private typedef -----------------------------------------------------------*/

/* Exported Functions Prototypes ---------------------------------------------*/
void shell_thread_entry(ULONG thread_input);

char tx_hw_console_getchar(void);

void tx_hw_console_output(const char *str);

#ifdef __cplusplus
}
#endif

#endif /*__SHELL_TASK_H__*/

