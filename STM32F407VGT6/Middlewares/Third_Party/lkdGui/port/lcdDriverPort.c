/**
  * @file   lcdDriverPort.c
  * @author  guoweilkd
  * @version V0.0.0
  * @date    2018/04/18
  * @brief  lkdGuiҺ����ʾ�������ӿڣ�����ֲ����亯���ľ������ݡ�
  */

#include "lkdGui.h"
#include "bsp_lcd.h"

/* Gui��ʾ������ */
static uint8_t  lkdGuiBuff[GUIXMAX][(GUIYMAX+7)>>3];
static uint8_t  lkdGuiBuff_bak[GUIXMAX][(GUIYMAX+7)>>3];

/**
  *@brief ��Gui������������ʾ��lcd��
  *@param  None
  *@retval None
  */
void GuiUpdateDisplayAll(void)
{
  /* ����û����� */
}

/**
  *@brief ��Guiָ��������������ʾ��lcd��
  *@param  beginx,beginy,endx,endy ����
  *@retval None
  */
void GuiRangeUpdateDisplay(lkdCoord beginx, lkdCoord beginy,lkdCoord endx, lkdCoord endy)
{
	/* ����û����� */
  uint8_t i, n;
  //@ /* ������ GUIYMAX ��� */
  for(i=0; i< 8; i++)
  {
    /* û�иı�ĵ㲻���ظ���ʾ */
    for(n=0; n < GUIXMAX; n++)
    {
      if (lkdGuiBuff[n][i] != lkdGuiBuff_bak[n][i])
      {
        break;
      }
    }
    if (n == GUIXMAX)   continue;

    WriteLcdCommand(0xb0+i);    /*����ҳ��ַ��0~7��*/
    WriteLcdCommand(0x00);      /*������ʾλ�á��е͵�ַ*/
    WriteLcdCommand(0x10);      /*������ʾλ�á��иߵ�ַ*/
    //@ /* ������ GUIXMAX ��� */
    for(n=0; n < GUIXMAX; n++)
    {
      WriteLcdData(lkdGuiBuff[n][i]);
      lkdGuiBuff_bak[n][i] = lkdGuiBuff[n][i];
    }
  }
}

/**
  *@brief �򻺳�������
  *@param  x,y ����
  *@param  color ��ɫ <Ŀǰֻ�к�:CBLACK ��:CWHITLE>
  *@retval None
  */
void GuiDrawPoint(lkdCoord x, lkdCoord y, lkdColour color)
{
  /* ����û����� */
  uint8_t i, n;
  //@ /* ������ GUIYMAX ��� */
  for(i=0; i< 8; i++)
  {
    /* û�иı�ĵ㲻���ظ���ʾ */
    for(n=0; n < GUIXMAX; n++)
    {
        if (lkdGuiBuff[n][i] != lkdGuiBuff_bak[n][i])
        {
            break;
        }
    }
    if (n == GUIXMAX)   continue;

    WriteLcdCommand(0xb0+i);    /*����ҳ��ַ��0~7��*/
    WriteLcdCommand(0x00);      /*������ʾλ�á��е͵�ַ*/
    WriteLcdCommand(0x10);      /*������ʾλ�á��иߵ�ַ*/
    //@ /* ������ GUIXMAX ��� */
    for(n=0; n < GUIXMAX; n++)
    {
      WriteLcdData(lkdGuiBuff[n][i]);
      lkdGuiBuff_bak[n][i] = lkdGuiBuff[n][i];
    }
  }
}

/**
  *@brief ��ȡ��ǰ��ʾ�ĵ�
  *@param  x,y ����
  *@param  color ��ɫ <Ŀǰֻ�к�:CBLACK ��:CWHITLE>
  *@retval None
  */
void  GuiReadPoint(lkdCoord x, lkdCoord y, lkdColour *pColor)
{
  /* ����û����� */
  uint8_t pos,bx,temp=0;
  //@ ����ʹ�ú������� /* ������ GUIYMAX �� GUIYMAX ��� */
  if(x>127||y>63)return;//������Χ��.
  x = 127-x;
  y = 63-y;
  pos=7-y/8;
  bx=y%8;
  temp=1<<(7-bx);
  if(pColor)lkdGuiBuff[x][pos]|=temp;
  else lkdGuiBuff[x][pos]&=~temp;  
}

/**
  *@brief ����ʾ
  *@param  None
  *@retval None
  */
void CloseLcdDisplay(void)
{
  /* ����û����� */
  Uc1698CloseLcdDisplay();
}

/**
  *@brief ����ʾ
  *@param  None
  *@retval None
  */
void OpenLcdDisplay(void)
{
  /* ����û����� */
  Uc1698OpenLcdDisplay();
}