/**
  ******************************************************************************
  * @file    task_gui.h
  * @brief   This file contains all the function prototypes for
  *          the task_gui.c file
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
#ifndef __GUI_TASK_H__
#define __GUI_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_azure_rtos.h"

/* Exported variables --------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Exported Functions Prototypes ---------------------------------------------*/
void gui_thread_entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif

#endif /*__GUI_TASK_H__*/

