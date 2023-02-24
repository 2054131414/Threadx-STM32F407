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

/* Private includes ----------------------------------------------------------*/
#include "shell_history.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

int shell_history_init(struct shell_history_queue_t *history)
{
    struct shell_history_head_t *phead;

    debug_assert(history);

    phead = &history->head;

    TAILQ_INIT(phead);

    history->current = TAILQ_FIRST(phead);

    return 0;
}

void shell_history_reset(shell_context_t *shell)
{
    struct shell_history_head_t *phead;

    if (shell->history != NULL)
    {
        phead = &shell->history->head;
        shell->history->current = TAILQ_FIRST(phead);   // 更新第一个记录
    }
}

int shell_history_append(shell_context_t *shell, const char *cmd, int len)
{
    struct shell_history_head_t *phead;
    struct shell_history_t *elm;
    int total;

    if (len <= 0 || shell->history == NULL)
        return 0;

    /* insert new elment */
    elm = shell->shell_malloc(sizeof(struct shell_history_t));

    if (elm == NULL)
        return 0;

    memset(elm, 0, sizeof(struct shell_history_t));

    elm->commands = shell->shell_malloc(len + 1);

    if (elm->commands == NULL)
    {
        shell->shell_free(elm);
        return 0;
    }

    strncpy(elm->commands, cmd, len);
    elm->commands[len] = '\0';
    phead = &shell->history->head;

    TAILQ_INSERT_HEAD(phead, elm, entries); // 插入数据
    shell->history->current = TAILQ_FIRST(phead);   // 更新第一个记录

    /* clear histroy */
    total = 0;

    TAILQ_FOREACH(elm, phead, entries)
    {
        total += 1;

        if (total > SHELL_HISTORY_NUM)
        {
            TAILQ_REMOVE(phead, elm, entries);

            shell->shell_free(elm->commands);
            shell->shell_free(elm);
        }
    }

    return total;
}

int shell_history_last(shell_context_t *shell, char *buf, int bufsize)
{
    struct shell_history_t *elm;

    debug_assert(shell->history);

    elm = shell->history->current;

    if (elm == NULL)
        return 0;

    strncpy(buf, elm->commands, bufsize);

    elm = TAILQ_NEXT(elm, entries);

    if (elm != NULL)
        shell->history->current = elm;

    return strlen(buf);
}

int shell_history_next(shell_context_t *shell, char *buf, int bufsize)
{
    struct shell_history_head_t *phead;
    struct shell_history_t *elm;

    debug_assert(shell->history);

    phead = &shell->history->head;
    elm = shell->history->current;

    if (elm == NULL)
        return 0;

    if (elm != TAILQ_FIRST(phead))
    {
        elm = TAILQ_PREV(elm, shell_history_head_t, entries);

        strncpy(buf, elm->commands, bufsize);

        shell->history->current = elm;
    }

    return strlen(buf);
}
