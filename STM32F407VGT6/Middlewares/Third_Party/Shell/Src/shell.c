/**
  ******************************************************************************
  * File Name          : shell.c
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
#include "shell.h"

/* Private includes ----------------------------------------------------------*/
#include "shell_commands.h"
#include "shell_history.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

void shell_output_prompt(shell_context_t* shell)
{
    shell->shell_printf("#%s> ", shell->name);
}

static void shell_update_line(shell_context_t* shell, char* buf, int index)
{
    if(index > 0)
    {
        shell->shell_printf("\033[%dD", index); // vt100: 光标移动输入首位
        shell->shell_puts("\033[K");    //  vt100: 清除从光标到行尾的内容
    }

    shell->shell_puts(buf);  // 显示命令行数据
}

int shell_getline(shell_context_t* shell, char* buf, size_t bufsize)
{
    int index, len;
    char* p;
    char ch;

    ch = buf[0];

    for(len = 0, index = 0; ; ch = shell->shell_getchar())
    {
        if(ch == '\r' || ch == '\n')
            break;  /* line end */

        if(ch == '\x03')  /* Ctrl + C */
        {
            index = 0;
            len = 0;
            break;
        }

        if((ch >= ' ') && (ch <= '~'))
        {
            if(len < bufsize)
            {
                if(index < len)
                {
                    for(p = &buf[len]; p >= &buf[index]; p--)
                    {
                        *p = *(p - 1);
                    }
                }

                buf[index++] = ch;
                buf[++len] = '\0';

                if(len == index)
                {
                    shell->shell_putchar(ch);
                }
                else
                {
                    shell->shell_puts(&buf[index - 1]); // 显示数据
                    shell->shell_printf("\033[%dD", len - index);   //  vt100: 光标左移 n 列
                }
            }

            continue;
        }

        if(ch == '\b' || ch == '\x7F')  /* xShell: '\b'/'\x08', putty: '\x7F'*/
        {
            if(index > 0)
            {
                if(index < len)
                {
                    for(p = &buf[index]; p < &buf[len]; p++)
                    {
                        *(p - 1) = *p;
                    }
                }

                index--;
                len--;
                buf[len] = '\0';

                shell->shell_puts("\033[D");    //  vt100: 光标左移 1 个字符
                shell->shell_puts("\033[K");    //  vt100: 清除从光标到行尾的内容
                shell->shell_puts(&buf[index]); // 显示数据

                if(index != len)
                {
                    shell->shell_printf("\033[%dD", len - index); // vt100: 光标移动插入位置
                }
            }

            shell_history_reset(shell);

            continue;
        }

        /* command match */
        if(ch == '\t')
        {
            const char* name;

            name = shell_commands_match(shell->command_sets, buf, len);

            if(name)
            {
                strncpy(buf, name, bufsize);
                shell_update_line(shell, buf, index);
                index = len = strlen(buf); // 更新 index, len
            }

            continue;
        }

        /* shell history */
        if(ch == '\x1B')  // 'ESC' '[' 'A'/'B'/'C'/'D'
        {
            ch = shell->shell_getchar();

            if(ch != '[')
                continue;

            ch = shell->shell_getchar();

            switch(ch)
            {
            case 'A': // up
                if(shell->history != NULL)
                {
                    buf[len] = '\0';

                    if(shell_history_last(shell, buf, bufsize) > 0)
                    {
                        shell_update_line(shell, buf, index);
                        index = len = strlen(buf); // 更新 index, len
                    }
                }
                break;

            case 'B': // down
                if(shell->history != NULL)
                {
                    buf[len] = '\0';

                    if(shell_history_next(shell, buf, bufsize) > 0)
                    {
                        shell_update_line(shell, buf, index);
                        index = len = strlen(buf); // 更新 index, len
                    }
                }
                break;

            case 'C': // right
                if(index < len)
                {
                    index++;
                    shell->shell_puts("\033[C"); // 光标右移1个字符
                }
                break;

            case 'D': // left
                if(index > 0)
                {
                    index--;
                    shell->shell_puts("\033[D"); // 光标左移1个字符
                }
                break;

            case '1': // home
                ch = shell->shell_getchar();

                if(ch == '~')  // '1B 5B 31 7E'
                {
                    shell->shell_printf("\033[%dD", index); // vt100: 光标移动输入首位
                    index = 0;
                }

                break;

            case '4': // end  '1B 5B 34 7E'
                ch = shell->shell_getchar();

                if(ch == '~')  // '1B 5B 31 7E'
                {
                    shell->shell_printf("\033[%dC", len - index); // vt100: 光标移动输入首位
                    index = len;
                }

                break;

            case '3': // Delete 删除右侧1个字符
                ch = shell->shell_getchar();

                if(ch == '~' && index < len)  // '1B 5B 33 7E'
                {
                    for(p = &buf[index]; p < &buf[len]; p++)
                    {
                        *p = *(p + 1);
                    }

                    len--;
                    buf[len] = '\0';

                    shell->shell_puts("\033[K");    //  vt100: 清除从光标到行尾的内容
                    shell->shell_puts(&buf[index]); // 显示数据

                    if(index != len)
                    {
                        shell->shell_printf("\033[%dD", len - index); // vt100: 光标移动插入位置
                    }

                    shell_history_reset(shell);
                }

                break;

            }
        }
    }

    buf[len] = '\0';
    shell->shell_puts(SHELL_NEW_LINE);

    shell_history_append(shell, buf, len);

    return len;
}

int32_t shell_input(shell_context_t* shell, const char* cmd)
{
    static char* args;
    static const struct shell_command_t* cmd_descr = NULL;

    /* Shave off any leading spaces. */
    while(*cmd == ' ')
    {
        cmd++;
    }

    /* Skip empty lines */
    if(*cmd != '\0')
    {
        /* Look for arguments */
        args = strchr(cmd, ' ');

        if(args != NULL)
        {
            *args = '\0';

            do
            {
                args++;
            }
            while(*args == ' ');
        }

        cmd_descr = shell_command_lookup(shell->command_sets, cmd, 0);

        if(cmd_descr != NULL && cmd_descr->func != NULL)
        {
            cmd_descr->func(shell, args);
        }
        else
        {
            shell->shell_printf("Command not found. Type 'help' for a list of commands" SHELL_NEW_LINE);
        }
    }

    shell_output_prompt(shell);

    return 0;
}

void shell_init(shell_context_t* shell, struct shell_command_sets_t* command_sets, struct shell_history_queue_t* history)
{
    debug_assert(shell);
    debug_assert(shell->shell_getchar);
    debug_assert(shell->shell_putchar);
    debug_assert(shell->shell_puts);
    debug_assert(shell->shell_printf);
    debug_assert(shell->shell_malloc);
    debug_assert(shell->shell_free);

    shell->command_sets = command_sets;
    shell->history = history;

    shell_commands_init(command_sets);

    if(history)
    {
        shell_history_init(history);
    }
}

