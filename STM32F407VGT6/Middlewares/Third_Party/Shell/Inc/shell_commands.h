/**
  ******************************************************************************
  * File Name          : shell_command.h
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

#ifndef __SHELL_COMMANDS_H
#define __SHELL_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported includes ---------------------------------------------------------*/
#include "shell.h"
/*
    from linux: '/usr/include/x86_64-linux-gnu/sys/queue.h'
    man doc: http://www.manpagez.com/man/3/queue/
*/
#include "queue.h"

/* Exported types ------------------------------------------------------------*/

/* Command handling function type */
typedef char (shell_commands_func)(shell_context_t* shell, char* args);

/* Command structure */
struct shell_command_t
{
    const char* name;
    shell_commands_func* func;
    const char* help;
};

/*
    shell command set
*/
struct shell_command_set_t
{
    SLIST_ENTRY(shell_command_set_t) next;
    const struct shell_command_t* const commands;
};

SLIST_HEAD(shell_command_sets_t, shell_command_set_t);

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

int shell_commands_init(struct shell_command_sets_t* sets);

int shell_command_set_register(struct shell_command_sets_t* sets, struct shell_command_set_t* set);

int shell_command_set_deregister(struct shell_command_sets_t* sets, struct shell_command_set_t* set);

const struct shell_command_t* shell_command_lookup(struct shell_command_sets_t* sets, const char* name, int len);

const char* shell_commands_match(struct shell_command_sets_t* sets, char* buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_COMMANDS_H */
