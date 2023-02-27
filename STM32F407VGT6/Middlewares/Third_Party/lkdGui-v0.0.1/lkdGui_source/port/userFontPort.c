/**
  * @file   userFontPort.c
  * @author  guoweilkd
  * @version V0.0.0
  * @date    2018/04/18
  * @brief  lkdGui字体驱动接口，由移植者填充函数的具体内容。
  */

#include "lkdGui.h"
#include "rtthread.h"
#include "params_store.h"
#include "params_gui.h"
/* 字体控制实体 */
static lkdFont defaultFont;

#if 0
#define USER_CH_FONT_CHECK()                                \
if (dl_part == NULL)                                        \
{                                                           \
    dl_part = fal_partition_find(FAL_NAME_FONT_SECTION);    \
}                                                           \
if ((dl_part == NULL)||(paramsCheckChFont() != RT_EOK))     \
{                                                           \
    LOG_E("[%s] ch font check failed", __func__);           \
    return RT_ERROR;                                        \
}
#endif

/**
  *@brief 获取汉字字模
  *@param  gbk_h,gbk_l 汉字编码[H,L]
  *@param  pBuff 字模buff
  *@retval 0
  */
static uint8_t GetDfontData(uint8_t gbk_h, uint8_t gbk_l,uint8_t *pBuff)
{
    /* 添加用户代码 */
    uint32_t offset;
    if((gbk_h > 0xFE)||(gbk_h < 0x81))
    {
        // LOG_E("[%s] gbk_h(0x%02X) > 0xFE or gbk_l(0x%02X) < 0x81", __func__, gbk_h, gbk_l);
        return 0;
    }
    gbk_h -= 0x81;
    gbk_l -= 0x40;
    offset=((uint32_t)192 * gbk_h + gbk_l) * CH_FONT_SYS * 2;
    if (paramsCheckChFont() != RT_EOK)
    {
        // LOG_E("[%s] ch font check failed", __func__);
        return 0;
    }
    if (paramsGetFromFal(FAL_NAME_FONT_SECTION, offset, pBuff, CH_FONT_SYS*2) != RT_EOK)
    {
        return 0;
    }
    return 0;
}

/**
  *@brief 获取ASCII字模
  *@param  code1, 编码
  *@param  pBuff 字模buff
  *@retval 0
  */
static uint8_t GetSfontData(uint8_t code1, uint8_t *pBuff)
{
    /* 添加用户代码 */
    extern uint8_t GetFontASCII_6_12(uint8_t code1, uint8_t *pBuff);
    GetFontASCII_6_12(code1,pBuff);
    return 0;
}

/**
  *@brief 字体初始化
  *@param  None
  *@retval None
  */
void defaultFontInit(void)
{
    /* 根据字体要求做相应的修改 */

    /* 此buff的大小由最大字模大小决定 */
	static uint8_t dataBuff[CH_FONT_SYS*2];

	defaultFont.name = (uint8_t *)"汉字字模为12*12的GB2312,ASCII字模为12*6";
	defaultFont.dhigh = 12;
	defaultFont.dwide = 12;     //@ 这个是否与 CH_FONT_SYS 相关
	defaultFont.shigh = 12;
	defaultFont.swide = 6;
	defaultFont.pZmBuff = dataBuff;
	defaultFont.getDfont = GetDfontData;
	defaultFont.getSfont = GetSfontData;

    /* 设置为系统默认字体 */
	GuiFontSet(&defaultFont);
    GuiSetbackcolor(CWHITLE);
    GuiSetForecolor(CBLACK);
}
