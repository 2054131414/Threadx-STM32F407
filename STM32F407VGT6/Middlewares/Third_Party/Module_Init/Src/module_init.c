/* USER CODE BEGIN Header */
/**
 * ****************************************************************************
 * @file    auto_init.c 
 * @brief   System initialization starts
 * ****************************************************************************
 * @attention
 * 
 * Segment registration, automatic initialization.
 * When the user uses it, 
 * it is enough to put the interface function in a different location
 * 
 * 
 * Change Logs:
 * Date             Author          Notes
 * 2022-11-24       Mingliang.sui   First version
 * 2023-01-20       Mingliang.sui   Compatible with AC6 compilers
 * 2023-02-20       Mingliang.sui   Rename the functions name 
 *
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "module_init.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

/**
 * @brief  Traverse empty functions
 * @param  None
 * @return Zero
 * @note   Registration is achieved by iterating through the empty function below
*/
static int _tx_start(void)
{
    return 0;
}
TX_INIT_EXPORT(_tx_start, "0");

static int _tx_board_start(void)
{
    return 0;
}
TX_INIT_EXPORT(_tx_board_start, "0.end");

static int _tx_board_end(void)
{
    return 0;
}
TX_INIT_EXPORT(_tx_board_end, "1.end");

static int _tx_end(void)
{
    return 0;
}
TX_INIT_EXPORT(_tx_end, "6.end");

/**
 * @brief  Components Initialization for board
 * @param  None
 * @retval None
*/
void TX_COMPONENTS_BOARD_INIT(void)
{
    volatile const initcall_t *tx_ptr;

    for (tx_ptr = &_init_call__tx_board_start; tx_ptr < &_init_call__tx_board_end; tx_ptr++)
    {
        (*tx_ptr)();
    }
}

/**
 * @brief  Components Initialization
 * @param  None
 * @retval None
*/
void TX_COMPONENTS_INIT(void)
{
    volatile const initcall_t *tx_ptr;

    for (tx_ptr = &_init_call__tx_board_end; tx_ptr < &_init_call__tx_end; tx_ptr ++)
    {
        (*tx_ptr)();
    }
}
