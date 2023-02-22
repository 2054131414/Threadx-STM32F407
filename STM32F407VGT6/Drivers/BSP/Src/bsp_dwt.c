/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_dwt.c
  * @brief          : DWT delay counter module
  ******************************************************************************
  * @attention       Use related functions for inference
  *
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_dwt.h"
#include "module_init.h"

/**
 * @brief  software delay
 * @param  n - the times to delay
 * @return none
*/
void delay_200ns(uint32_t n)
{
    uint32_t k;

    while(n--)
    {
        for(k = 0; k < 6; k++)
        {
            __NOP();
        }
    }
}

/**
 * @brief Coretex-M4 dwt counter initialization
*/
int BSP_InitDWT(void)
{
    DEM_CR         |= (unsigned int)DEM_CR_TRCENA;
    DWT_CYCCNT      = (unsigned int)0u;
    DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;

    return 0;
}
TX_INIT_BOARD_EXPORT(BSP_InitDWT);

/**
 * @brief  bsp dwt delay ms
 * @param  _ulDelayTime
 * @return none
*/
void bsp_DelayMS(uint32_t _ulDelayTime)
{
    bsp_DelayUS(1000 * _ulDelayTime);
}

/**
 * @brief  bsp dwt delay us
 * @param  _ulDelayTime
 * @return none
*/
void bsp_DelayUS(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
    uint32_t tStart;

    tStart = DWT_CYCCNT;
    tCnt = 0;
    tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);

    while(tCnt < tDelayCnt)
    {
        tCnt = DWT_CYCCNT - tStart;
    }
}

/**
 * @brief  bsp delay DWT
 * @param  _ulDelayTime
 * @return none
*/
void bsp_DelayDWT(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
    uint32_t tStart;

    tCnt = 0;
    tDelayCnt = _ulDelayTime;
    tStart = DWT_CYCCNT;

    while(tCnt < tDelayCnt)
    {
        tCnt = DWT_CYCCNT - tStart;
    }
}
