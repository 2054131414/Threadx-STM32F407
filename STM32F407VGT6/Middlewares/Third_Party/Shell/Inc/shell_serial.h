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

#ifndef __SHELL_SERIAL_H
#define __SHELL_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported includes ---------------------------------------------------------*/
#include "shell.h"
#include "shell_commands.h"
#include "shell_history.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void shell_serial(struct shell_command_set_t *set);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_SERIAL_H */
