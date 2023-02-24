/**
  ******************************************************************************
  * File Name          : shell_history.c
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

#ifndef __SHELL_HISTORY_H
#define __SHELL_HISTORY_H

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

/* Exported define -----------------------------------------------------------*/

#define SHELL_HISTORY_NUM   5

/* Exported types ------------------------------------------------------------*/

/*
 * Shell history element
 */
struct shell_history_t
{
    TAILQ_ENTRY(shell_history_t) entries;
    char *commands;
};

/*
 * List definitions.
 */
TAILQ_HEAD(shell_history_head_t, shell_history_t);

/*
 * Shell history context
 */
struct shell_history_queue_t
{
    struct shell_history_head_t head;
    struct shell_history_t *current;
};

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

int shell_history_init(struct shell_history_queue_t *history);

void shell_history_reset(shell_context_t *shell);

int shell_history_append(shell_context_t *shell, const char *cmd, int len);

int shell_history_last(shell_context_t *shell, char *buf, int bufsize);

int shell_history_next(shell_context_t *shell, char *buf, int bufsize);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPLATE_H */
