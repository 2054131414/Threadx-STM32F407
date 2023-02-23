/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_w25qxx.c
  * @brief          : Flash driver module
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_w25qxx.h"

/* Private includes ----------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "module_init.h"
#include "bsp.h"
#include "sfud.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SPI1_CLK_Pin       GPIO_PIN_3
#define SPI1_CLK_GPIO_Port GPIOB

#define SPI1_CS_Pin        GPIO_PIN_6
#define SPI1_CS_GPIO_Port  GPIOB

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* Private function prototypes -----------------------------------------------*/


/**
 * @brief  SPI init function
 * @param  none
 * @return none
*/
void MX_SPI1_Init(void)
{
    hspi1.Instance               = SPI1;
    hspi1.Init.Mode              = SPI_MODE_MASTER;
    hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi1.Init.NSS               = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial     = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  Config the hardware resources
 * @param  none
 * @return none
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance==SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin       = SPI1_CLK_Pin|GPIO_PIN_4|GPIO_PIN_5;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    if(spiHandle->Instance==SPI1)
    {
        __HAL_RCC_SPI1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, SPI1_CLK_Pin|GPIO_PIN_4|GPIO_PIN_5);
    }
}


/**
 * @brief SPI Nor flash informations
*/
sfud_flash sfud_norflash0 =
{
    .name     = "norflash",
    .spi.name = "SPI1",
    .chip     = { "W25Q64FV", SFUD_MF_ID_WINBOND, 0x40, 0x17, 8L * 1024L * 1024L, SFUD_WM_PAGE_256B, 4096, 0x20 }
};

/**
 * @brief  SFUD use spi flash init
 * @param  None
 * @return 0->success -1->fail
 * @note   This function must be used with sfud together
*/
int sfud_flash_init(void)
{
    /* SFUD initialize */
    if (sfud_device_init(&sfud_norflash0) == SFUD_SUCCESS)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
//TX_INIT_BOARD_EXPORT(sfud_flash_init);

