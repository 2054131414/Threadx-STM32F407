/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_lcd.c
  * @author  mingliang.sui
  * @brief   hardware underlying driver initialization
  ******************************************************************************
  * @attention
  * The function must be called before the scheduler is started
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp_lcd.h"

/* Private includes ----------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define CS_H 	 	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,GPIO_PIN_SET);
#define CS_L 		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,GPIO_PIN_RESET);

#define SDA_H 	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_SET);
#define SDA_L 	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);

#define SCLK_H 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET);
#define SCLK_L 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);

#define LED_H 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
#define LED_L 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);

#define RS_H  	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13 , GPIO_PIN_SET);
#define RS_L  	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13 , GPIO_PIN_RESET);

#define RST_H 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12, GPIO_PIN_SET);
#define RST_L 	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12, GPIO_PIN_RESET);

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/**
  *@brief  引脚初始化
  *@param  None
  *@retval None
  */
static void LcdGpioConfig(void);
/**
 * @brief 发送字节数据
 * @param data 数据值
 */
static void lcd_send_data(unsigned char data);
static void lcd_send_data(unsigned char data);
/* 清屏 */
void Clear_screen(void)
{
    uint8_t i, page;
    /* @//@ 这里是否与 GUIYMAX 相关 */
    for(page = 0xb0; page < 0xb8; page++)
    {
        WriteLcdCommand(0x00);
        WriteLcdCommand(page);
        WriteLcdCommand(0x00);
        WriteLcdCommand(0x10);/* 列地址，高低字节两次写入，从第0列开始 */
        /* @//@ 这里是否与 GUIXMAX 相关 */
        for(i = 0; i < 128; i++)
        {
            WriteLcdData(0);
        }
    }
}
/* lcd初始化 */
void LcdInit(void)
{
    /* 初始化IO管脚 */
    LcdGpioConfig();
    LED_H;
    RST_L; /* 低电平复位 */
    HAL_Delay(50);
    RST_H; /* 高电平 */
    HAL_Delay(50);
    WriteLcdCommand(0xe2); /* 软复位 */
    HAL_Delay(5);
    WriteLcdCommand(0x2c); /* 升压步骤1 */
    HAL_Delay(5);
    WriteLcdCommand(0x2e); /* 升压步骤2 */
    HAL_Delay(5);
    WriteLcdCommand(0x2f); /* 升压步骤3 */
    HAL_Delay(5);
    WriteLcdCommand(0x23); /* 粗调对比度，可设置范围0x20~0x27 */
    WriteLcdCommand(0x81); /* 微调对比度 */
    WriteLcdCommand(0x1A); /* */
    WriteLcdCommand(0xa2); /* 1/9 偏压比 */
    WriteLcdCommand(0xc0); /* 行扫描序列：从上到下 */
    WriteLcdCommand(0xa0); /* 列扫描序列：从左到右 */
    WriteLcdCommand(0x0F); /* 起始行第0行开始 */
    WriteLcdCommand(0xA4); /* 全显所有点阵 */
    WriteLcdCommand(0xaf); /* 开显示 */
    HAL_Delay(10);
    WriteLcdCommand(0x30);
    HAL_Delay(5);
    WriteLcdCommand(0x30);
    HAL_Delay(10);
    WriteLcdCommand(0x0c);
    Clear_screen(); /* 清屏 */
}

/* 写命令 */
void WriteLcdCommand(uint8_t cmd)
{
    CS_L;
    RS_L;
    lcd_send_data(cmd); /* 发送数据*/
    CS_H;
}

/* 写书数据 */
void WriteLcdData(uint8_t data)
{
    CS_L;
    RS_H;
    lcd_send_data(data); /* 发送数据 */
    CS_H;
}

/* 设置起始行 */
void SetLcdRow(uint8_t row)
{
    WriteLcdCommand(CMD_ROWADDRH | ((row & 0xF0) >> 4));
    WriteLcdCommand(CMD_ROWADDRL | (row & 0x0F));
}

/* 设置起始列 */
void SetLcdCol(uint8_t col)
{
    WriteLcdCommand(CMD_COLADDRH | ((col & 0x70) >> 4));
    WriteLcdCommand(CMD_COLADDRL | (col & 0x0F));
}

/* 开显示 */
void Uc1698OpenLcdDisplay(void)
{
    CS_L;
    WriteLcdCommand(OPEN_LCDDISPLAY);
    CS_H;
    LED_H;
}

/* 关显示 */
void Uc1698CloseLcdDisplay(void)
{
    CS_L;
    WriteLcdCommand(CLOSE_LCDDISPLAY);
    CS_H;
    LED_L;
}
/* 引脚初始化 */
static void LcdGpioConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LCD_SDA_Pin;
    HAL_GPIO_Init(LCD_SDA_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_CS_Pin;
    HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_RST_Pin;
    HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_SCLK_Pin;
    HAL_GPIO_Init(LCD_SCLK_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_LED_Pin;
    HAL_GPIO_Init(LCD_LED_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_RS_Pin;
    HAL_GPIO_Init(LCD_RS_GPIO_Port, &GPIO_InitStruct);
    LED_H;
}

/* 发送字节数据 */
static void lcd_send_data(unsigned char data)
{
    unsigned char buf = data;

    for(int i = 0; i < 8; i++)
    {
        if((buf & 0x80))
        {
            SDA_H;
        }
        else
        {
            SDA_L;
        }
        SCLK_L;
        __NOP();
        SCLK_H;
        buf = buf << 1;
    }
}
