/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_lcd.h
  * @brief          : hardware underlying driver initialization
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_LCD_H_
#define _BSP_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ----------------------------------------------------------*/
#define OPEN_LCDDISPLAY 		0xAD     /* 开显示 */
#define CLOSE_LCDDISPLAY 	  0xAC     /* 关显示 */
#define CMD_ROWADDRH        0x70     /*  */
#define CMD_ROWADDRL        0x60     /*  */
#define CMD_COLADDRH        0x10     /*  */
#define CMD_COLADDRL        0x00     /*  */

#define BR 			            0X01     // Set LCD Bias Ratio:1/10 bias
#define PC1                 0X03     // power control set as internal power
#define TC                  0x00     // set temperate compensation as 0%
#define PM                  192      // Set Vbias Potentiometer
#define LC_210              0X05     // set LCD Control
#define LC_43               0x03
#define LC_5                0X01
#define LC_76               0x01
#define LC_8                0X00
#define NIV                 0X10     // n-line inversion
#define CSF                 0X02     // 23:enable FRC,PWM,LRM sequence
#define WPC0                0x25     // Starting Column Address
#define WPP0                0        // Starting Row Address
#define WPC1                0x5A     // Ending Column Address
#define WPP1                159      // Ending Row Address
#define AC                  0x01     // Address  Control
#define CEN                 159      // COM scanning end (last COM with full line cycle,0 based index)

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 *@brief LCD初始化
 *@param  None
 *@retval None
 */
void LcdInit(void);

/**
 *@brief 设置起始行
 *@param  row 行
 *@retval None
 */
void SetLcdRow(uint8_t row);

/**
 *@brief 设置起始列
 *@param  col 列地址
 *@retval None
 */
void SetLcdCol(uint8_t col);

/**
 *@brief 写数据
 *@param  data 数据
 *@retval None
 */
void WriteLcdData(uint8_t data);

/**
 *@brief 写命令
 *@param  cmd 命令
 *@retval None
 */
void WriteLcdCommand(uint8_t cmd);

/**
 *@brief 开显示
 *@param  None
 *@retval None
 */
void Uc1698OpenLcdDisplay(void);

/**
 *@brief 关显示
 *@param  None
 *@retval None
 */
void Uc1698CloseLcdDisplay(void);

/**
 * 清屏
 */
void Clear_screen(void);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_LCD_H_ */
