/**
  * @file   lcdDriverPort.c
  * @author  guoweilkd
  * @version V0.0.0
  * @date    2018/04/18
  * @brief  lkdGui液晶显示屏驱动接口，由移植者填充函数的具体内容。
  */

#include "lkdGui.h"
#include "bsp_lcd.h"

/* Gui显示缓冲区 */
static uint8_t  lkdGuiBuff[GUIXMAX][(GUIYMAX+7)>>3];
static uint8_t  lkdGuiBuff_bak[GUIXMAX][(GUIYMAX+7)>>3];

/**
  *@brief 将Gui缓冲区数据显示到lcd上
  *@param  None
  *@retval None
  */
void GuiUpdateDisplayAll(void)
{
  /* 添加用户代码 */
}

/**
  *@brief 将Gui指定缓冲区数据显示到lcd上
  *@param  beginx,beginy,endx,endy 坐标
  *@retval None
  */
void GuiRangeUpdateDisplay(lkdCoord beginx, lkdCoord beginy,lkdCoord endx, lkdCoord endy)
{
	/* 添加用户代码 */
  uint8_t i, n;
  //@ /* 这里与 GUIYMAX 相关 */
  for(i=0; i< 8; i++)
  {
    /* 没有改变的点不用重复显示 */
    for(n=0; n < GUIXMAX; n++)
    {
      if (lkdGuiBuff[n][i] != lkdGuiBuff_bak[n][i])
      {
        break;
      }
    }
    if (n == GUIXMAX)   continue;

    WriteLcdCommand(0xb0+i);    /*设置页地址（0~7）*/
    WriteLcdCommand(0x00);      /*设置显示位置—列低地址*/
    WriteLcdCommand(0x10);      /*设置显示位置—列高地址*/
    //@ /* 这里与 GUIXMAX 相关 */
    for(n=0; n < GUIXMAX; n++)
    {
      WriteLcdData(lkdGuiBuff[n][i]);
      lkdGuiBuff_bak[n][i] = lkdGuiBuff[n][i];
    }
  }
}

/**
  *@brief 向缓冲区画点
  *@param  x,y 坐标
  *@param  color 颜色 <目前只有黑:CBLACK 白:CWHITLE>
  *@retval None
  */
void GuiDrawPoint(lkdCoord x, lkdCoord y, lkdColour color)
{
  /* 添加用户代码 */
  uint8_t i, n;
  //@ /* 这里与 GUIYMAX 相关 */
  for(i=0; i< 8; i++)
  {
    /* 没有改变的点不用重复显示 */
    for(n=0; n < GUIXMAX; n++)
    {
        if (lkdGuiBuff[n][i] != lkdGuiBuff_bak[n][i])
        {
            break;
        }
    }
    if (n == GUIXMAX)   continue;

    WriteLcdCommand(0xb0+i);    /*设置页地址（0~7）*/
    WriteLcdCommand(0x00);      /*设置显示位置—列低地址*/
    WriteLcdCommand(0x10);      /*设置显示位置—列高地址*/
    //@ /* 这里与 GUIXMAX 相关 */
    for(n=0; n < GUIXMAX; n++)
    {
      WriteLcdData(lkdGuiBuff[n][i]);
      lkdGuiBuff_bak[n][i] = lkdGuiBuff[n][i];
    }
  }
}

/**
  *@brief 读取当前显示的点
  *@param  x,y 坐标
  *@param  color 颜色 <目前只有黑:CBLACK 白:CWHITLE>
  *@retval None
  */
void  GuiReadPoint(lkdCoord x, lkdCoord y, lkdColour *pColor)
{
  /* 添加用户代码 */
  uint8_t pos,bx,temp=0;
  //@ 这里使用宏来代替 /* 这里与 GUIYMAX 和 GUIYMAX 相关 */
  if(x>127||y>63)return;//超出范围了.
  x = 127-x;
  y = 63-y;
  pos=7-y/8;
  bx=y%8;
  temp=1<<(7-bx);
  if(pColor)lkdGuiBuff[x][pos]|=temp;
  else lkdGuiBuff[x][pos]&=~temp;  
}

/**
  *@brief 关显示
  *@param  None
  *@retval None
  */
void CloseLcdDisplay(void)
{
  /* 添加用户代码 */
  Uc1698CloseLcdDisplay();
}

/**
  *@brief 开显示
  *@param  None
  *@retval None
  */
void OpenLcdDisplay(void)
{
  /* 添加用户代码 */
  Uc1698OpenLcdDisplay();
}