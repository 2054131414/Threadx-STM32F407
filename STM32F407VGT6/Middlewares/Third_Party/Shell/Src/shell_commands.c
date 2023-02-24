/**
  ******************************************************************************
  * File Name          : shell_command.c
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

/* Includes ------------------------------------------------------------------*/
#include "shell_commands.h"

/* Private includes ----------------------------------------------------------*/
#include "shell.h"
#include "shell_history.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
//static struct shell_command_head_t shell_command_set_head = SLIST_HEAD_INITIALIZER();
static struct shell_command_set_t builtin_shell_command_set;

/* Private function prototypes -----------------------------------------------*/

int shell_commands_init(struct shell_command_sets_t* sets)
{
    debug_assert(sets);

    /* Initialize the list. */
    SLIST_INIT(sets);

    /* Initialize the builtin shell commands list. */
    SLIST_INSERT_HEAD(sets, &builtin_shell_command_set, next);

    return 0;
}

int shell_command_set_register(struct shell_command_sets_t* sets, struct shell_command_set_t* set)
{
    struct shell_command_set_t* var = NULL;

    debug_assert(sets);

    if(SLIST_EMPTY(sets))
    {
        SLIST_INSERT_HEAD(sets, set, next);
    }
    else
    {
        SLIST_FOREACH(var, sets, next)
        {
            if(var->next.sle_next == NULL)
            {
                SLIST_INSERT_AFTER(var, set, next);
                break;
            }
        }
    }

    return 0;
}

int shell_command_set_deregister(struct shell_command_sets_t* sets, struct shell_command_set_t* set)
{
    debug_assert(sets);

    SLIST_REMOVE(sets, set, shell_command_set_t, next);

    return 0;
}

/* len = 0 表示全字匹配， len != 0 返回第一个匹配的命令 */
const struct shell_command_t* shell_command_lookup(struct shell_command_sets_t* sets, const char* name, int len)
{
    struct shell_command_set_t* set;
    const struct shell_command_t* cmd;

    debug_assert(sets);

    SLIST_FOREACH(set, sets, next)
    {
        for(cmd = set->commands; cmd->name != NULL; ++cmd)
        {
            if(len != 0 && !strncmp(cmd->name, name, len))
            {
                return cmd;
            }
            else if(!strcmp(cmd->name, name))
            {
                return cmd;
            }
        }
    }

    return NULL;
}

const char* shell_commands_match(struct shell_command_sets_t* sets, char* buf, int len)
{
    const struct shell_command_t* cmd;

    cmd = shell_command_lookup(sets, buf, len);

    return (cmd == NULL) ? NULL : cmd->name;
}

static char cmd_history(shell_context_t* shell, char* args)
{
    struct shell_history_head_t* phead;
    struct shell_history_t* elm;
    int index;

    debug_assert(shell);

    if(shell->history == NULL)
    {
        shell->shell_printf("Shell history not init." SHELL_NEW_LINE);
        return NULL;
    }

    shell->shell_printf("shell history queue:"  SHELL_NEW_LINE);

    phead = &shell->history->head;
    index = 0;

    TAILQ_FOREACH(elm, phead, entries)
    {
        index++;

        shell->shell_printf("%d: ", index);
        shell->shell_puts(elm->commands);
        shell->shell_puts(SHELL_NEW_LINE);
    }

    return NULL;
}

static char cmd_help(shell_context_t* shell, char* args)
{
    struct shell_command_sets_t* sets = shell->command_sets;
    struct shell_command_set_t* set;
    const struct shell_command_t* cmd;

    debug_assert(shell);
    debug_assert(shell->command_sets);

    shell->shell_printf("Available commands:"  SHELL_NEW_LINE);

    /* Note: we explicitly don't expend any code space to deal with shadowing */
    SLIST_FOREACH(set, sets, next)
    {
        for(cmd = set->commands; cmd->name != NULL; ++cmd)
        {
            shell->shell_printf("%s" SHELL_NEW_LINE, cmd->help);
        }
    }

    return NULL;
}

static char cmd_clear(shell_context_t* shell, char* args)
{
    debug_assert(shell);

    shell->shell_printf("\033[2J"); // VT100 终端控制码，清屏
    shell->shell_printf("\033[%d;%dH", 0, 0); // VT100 终端控制码，光标移到 y,x

    return NULL;
}

static char cmd_reboot(shell_context_t* shell, char* args)
{
    debug_assert(shell);

    shell->shell_printf("rebooting"  SHELL_NEW_LINE);

    //NVIC_SystemReset();

    return NULL;
}

const struct shell_command_t builtin_shell_commands[] =
{
    { "help",                 cmd_help,                 "'> help': Shows this help" },
    { "clear",                cmd_clear,                "'> clear': Clear the screen" },
    { "history",              cmd_history,               "'> history': List shell history input." },
    { "reboot",               cmd_reboot,               "'> reboot': Reboot the board by watchdog_reboot()" },
    { NULL, NULL, NULL },
};

static struct shell_command_set_t builtin_shell_command_set =
{
    .next     = NULL,
    .commands = builtin_shell_commands,
};
