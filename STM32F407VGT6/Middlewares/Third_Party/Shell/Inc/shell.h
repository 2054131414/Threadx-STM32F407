/**
  ******************************************************************************
  * File Name          : shell.c
  * Description        : shell task thread
  ******************************************************************************
  * @attention
  * VT100 控制码: \033[0m     // 关闭所有属性      \033[4m     // 下划线
  *               \033[1m     // 设置为高亮        \033[5m     // 闪烁
  *               \033[7m     // 反显              \033[8m     // 消隐
  *               \033[nA     // 光标上移 n 行     \033[nB     // 光标下移 n 行
  *               \033[nC     // 光标右移 n 行     \033[nD     // 光标左移 n 行
  *               \033[y;xH   // 设置光标位置      \033[2J     // 清屏
  *               \033[K      // 清除从光标到行尾的内容
  *               \033[s      // 保存光标位置      \033[?25l   // 隐藏光标
  *               \033[u      // 恢复光标位置      \033[?25h   // 显示光标
  *
  *             \033[30m – \033[37m 为设置前景色   \033[40m – \033[47m 为设置背景色
  *             30: 黑色                           40: 黑色
  *             31: 红色                           41: 红色
  *             32: 绿色                           42: 绿色
  *             33: 黄色                           43: 黄色
  *             34: 蓝色                           44: 蓝色
  *             35: 紫色                           45: 紫色
  *             36: 青色                           46: 青色
  *             37: 白色                           47: 白色    
  *
  *
  ******************************************************************************
  */

#ifndef __SHELL_H
#define __SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported includes ---------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "shell_log.h"

/* Exported define ------------------------------------------------------------*/
/* End of line */
#define SHELL_NEW_LINE   "\r\n"

/* Helper macros to parse arguments */
#define SHELL_ARGS_INIT(args, next_args) (next_args) = (args);

#define SHELL_ARGS_NEXT(args, next_args)            \
    do {                                            \
        (args) = (next_args);                       \
        if((args) != NULL) {                        \
            if(*(args) == '\0') {                   \
                (args) = NULL;                      \
            } else {                                \
                (next_args) = strchr((args), ' ');  \
                if((next_args) != NULL) {           \
                    *(next_args) = '\0';            \
                    (next_args)++;                  \
                }                                   \
            }                                       \
        } else {                                    \
            (next_args) = NULL;                     \
        }                                           \
    } while(0)

/* Exported types ------------------------------------------------------------*/
struct shell_command_sets_t;

struct shell_history_queue_t;

typedef struct _shell_context_t
{
    const char* name;

    int (*shell_getchar)(void);
    int (*shell_putchar)(char ch);
    int (*shell_puts)(const char* str);
    int (*shell_printf)(const char* fmt, ...);
    void* (*shell_malloc)(size_t size);
    void (*shell_free)(void* ptr);

    struct shell_command_sets_t* command_sets;
    struct shell_history_queue_t* history;
} shell_context_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

void shell_output_prompt(shell_context_t* shell);

int shell_getline(shell_context_t* shell, char* buf, size_t bufsize);

int32_t shell_input(shell_context_t* shell, const char* cmd);

void shell_init(shell_context_t* shell, struct shell_command_sets_t* command_sets, struct shell_history_queue_t* history_queue);

#ifdef __cplusplus
}
#endif

#endif /* __SHELL_H */
