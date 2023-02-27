/**
  * @file   userAppPort.c
  * @author  guoweilkd
  * @version V0.0.0
  * @date    2019/01/17
  * @brief  lkdGui�û�Ӧ�ñ�д�ļ���
  * ��ʹ��GB2312�����ʽ�򿪴��ļ�������������������������
  * ��ʹ��GB2312�����ʽ�򿪴��ļ�������������������������
  * ��ʹ��GB2312�����ʽ�򿪴��ļ�������������������������
  * ��ʹ��GB2312�����ʽ�򿪴��ļ�������������������������
  * ��ʹ��GB2312�����ʽ�򿪴��ļ�������������������������
  * ˵�����ɰ汾�е�����״̬����û����ӽ��� Deprecated
  */

/************************************************/
/*                 ͷ�ļ�����                    */
/************************************************/
/* �������Ͷ���ͷ�ļ� */
#include "basic_os.h"

/* �²�����ͷ�ļ� */
#include "qrcode.h"

#include "params_feed_model.h"
#include "params_gateway.h"
#include "params_term.h"
#include "params_term_config.h"
#include "params_lora.h"
#include "params_sell.h"
#include "params_ota.h"
#include "params_gui.h"
#include "params_unit_hw_info.h"
#include "plat_date_time.h"
#include "plat_eventos.h"
#include "plat_node.h"

#include "app_term_event.h"
#include "app_ota.h"
#include "rs485_pack.h"
/* ��ģ������ͷ�ļ� */

/* ��ǰ��˽������ͷ�ļ� */
#include "lkdGui.h"
#include "UC1698Driver.h"

#define DBG_TAG "GUI"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <stdio.h>
/************************************************/
/*                 ����ģ��                                                         */
/************************************************/
/* �� */
#define MENUSTACK_NUM               8         //@ �� GUIWINMANAGE_NUM ���

#define GUI_ONE_LINE_MAX_SIZE       24

#define GUI_WAIT_TIMEOUT_CHECK()                                \
if (s_gui_wait_flag == 1)                                       \
{                                                               \
    if ((uint32_t)(MY_TICK_GET()-s_gui_wait_last_tick) > 1500)  \
    {                                                           \
        goto exit;                                              \
    }                                                           \
    return;                                                     \
}

/* ��Ӧs_gui_custom_flag�еĶ�Ӧλ */
/** 1-��ʾ�������ڽ��е�ַ��ͻ������ʾ; 0-��ʾû�е�ַ��ͻ������ʾ */
#define GUI_CUSTOM_FLAG_ADDR_COLLIDE         0

/*�������*/
static const My_Node_T *s_pt_key_node = NULL;
static int s_i_key_status = 0;
enum Key_Status_T
{
    KEY_UP_DOWN = 1,
    KEY_LEFT_DOWN,
    KEY_RIGHT_DOWN,
    KEY_OK_DOWN,
    KEY_DOWN_DOWN,
    KEY_CANCEL_DOWN,
};

/* �˵���� */
static lkdMenu HmiMenu;
/* �˵�ջ */
static MenuStack userMenuStack[MENUSTACK_NUM];

/* ��Ӧ�ó�ʼ����ɺ󣬸ñ�������֪ͨgui������Ӧ�ý���, 0:��ʾ�����������ȴ���1-��ʾ������Ӧ�þ������  */
static uint8_t s_gui_process_end = 0;
/* gui��ʼ����ɱ�־��0-δ��ʼ����ɣ�1-��ʼ����� */
static uint8_t s_gui_inited_end = 0;
/* gui��������Ҫ�����־ */
static uint8_t s_gui_saving = 0;
/* �����첽ʹ�� */
static uint16_t s_gui_term_addr = 1;
static uint8_t s_gui_wait_flag = 0;
static uint32_t s_gui_wait_last_tick = 0;
static uint8_t first_enter_flag = 0;

/* ��Щֵ����һЩ���� */
static int32_t s_gui_value = 0;
static uint8_t s_gui_value_0 = 0;
static uint8_t s_gui_value_1 = 0;
static uint8_t s_gui_value_2 = 0;
static uint8_t s_gui_value_3 = 0;

static double s_df_gui_value_0 = 0;
static double s_df_gui_value_1 = 0;

/* ÿһλ����һ����־ */
static uint32_t s_gui_custom_flag = 0;

/************************************************/
/*                 �ⲿ���ú�����������           */
/************************************************/
extern void GuiWinDraw(const lkdWin *pWindow);

/************************************************/
/*                 �ڲ�������������               */
/************************************************/
/** ������ */
static void _userWinHomeFunc(void *param);
/** ע�ᴰ�ڷ����� */
static void _userWinLoraRegFunc(void *param);
/** ��ά�봰�ڷ����� */
static void _userWinQrCodeFunc(void *param);
/** �������ն�ͨ�ųɹ��ʴ��ڷ����� */
static void _userCommSuccRateFunc(void *param);
/** ������Lora�ն�ͨ���ź�ǿ�ȴ��ڷ����� */
static void _userLoraRssiFunc(void *param);
/* ��ַ��ͻ���ڷ����� */
static void _userAddrCollideFunc(void *param);

/** �˵���������� */
static void _userMenuFun(void *param);

/** ��Ԫ�������÷����� */
static void _userUnitTypeSetFunc(void *param);
/** ��Ԫ�����÷����� */
static void _userUnitNumSetFunc(void *param);
/** ��Ԫ��λ���÷����� */
static void _userUnitTermRawSetFunc(void *param);

/** ��ιƽ̨ip�Ͷ˿����ý�������� */
static void _userServerInfoSetFunc(void *param);
/** ��ιƽ̨ѡ���������� */
static void _userMqttSelectSetFunc(void *param);
#if 0
/** �û�����ѡ���������� */
static void _userUpgradeModeSetFunc(void *param);
#endif

/** ��Ԫͨ���������ý�������� */
static void _userUnitCommTypeSetFunc(void *param);

/** ��ι�����·���ʽ���ý�������� */
static void _userUnitFeedVersionSetFunc(void *param);

/** ��Ԫ���Ϸ�ʽ���ý��������*/
static void _userUnitFoodModeSetFunc(void *param);

/** lora�ź�ǿ�Ȳ��Խ�������� */
static void _userLorsTestFunc(void *param);
#if 0
/** ��ι�������ò��·���������� */
static void _userFeedParamSetDefault(void *param);
#endif
/** ������ι��¼��ս��� */
static void _userTermRecodeClear(void *param);

/** �����ŵ����ý�������� */
static void _userLoraChannelSetFunc(void *param);

/** ���߷��͹������ý�������� */
static void _userLoraTxpowerSetFunc(void *param);

/** ������Ƶ�������ý�������� */
static void _userLoraSfSetFunc(void *param);

#if 0
/** ���߱��������ý�������� */
static void _userLoraCrSetFunc(void *param);

/** ���ߴ������ý�������� */
static void _userLoraBwSetFunc(void *param);

/** �����ŵ��Զ������������ý�������� */
static void _userLoraChAdjustSetFunc(void *param);

/** �����ŵ��Զ�������ʼ��������� */
static void _userLoraChAdjustStartFunc(void *param);
#endif

/** Loraͨ�Ų�����ʼ����������� */
static void _userLoraSetDefaultFunc(void *param);

#if 0
/** ���ö�ˮģʽ��������� */
static void _userConfigNoWaterFunc(void *param);

/** ��������ģʽʱ�����ý�������� */
static void _userConfigSellTimeSetFunc(void *param);

/** ��������ģʽ�������ý�������� */
static void _userConfigSellSwitchSetFunc(void *param);
#endif

/** ̽����ֵ���ý�������� */
static void _userConfigProbeAdcSetFunc(void *param);

/** ��̬��ۿ������ý�������� */
static void _userFoodClearSwitchSetFunc(void *param);

/** ��̬��ۼ��ʱ�����ý�������� */
static void _userFoodClearCheckTimeSetFunc(void *param);

/** ��̬���ʱ�����ý�������� */
static void _userFoodClearFoodTimeSetFunc(void *param);

/** ��̬��ˮʱ�����ý�������� */
static void _userFoodClearWaterTimeSetFunc(void *param);

/** ��̬�����ʼ�������ý�������� */
static void _userFoodClearStartAgeSetFunc(void *param);

/* ��̬��۽����������ý�������� */
static void _userFoodClearStopAgeSetFunc(void *param);

#if 0
/** ����ʱ�����ý�������� */
static void _userProtectTimeSetFunc(void *param);
#endif
/* ��ˮģʽ���ý�������� */
static void _userWaterModeSetFunc(void *param);
/* ��˸�̽�����������ý�������� */
static void _userProbeSenSetFunc(void *param);
/**һ�巧�������ý���*/
static void _userValveSpeedSetFunc(void *param);
#if 0
/* ���ٲ������ý�������� */
static void _userWaterSpeedDiffSetFunc(void *param);
#endif
/** ����У׼���ý�������� */
static void _userProtectPulseCalFunc(void *param);
/** ��ǿ�������ý�������� */
static void _userStrongFoodGearSetFunc(void *param);
/** ��ǿ��ˮ���ý�������� */
static void _userStrongWaterGearSetFunc(void *param);

/** �������ʱ������ */
static void _userMrFeedCleanSetFunc(void *param);
/** ���鴥����ʱʱ������ */
static void _userMrTouchDelaySetFunc(void *param);
/** ����ˮ�������� */
static void _userMrFeedWaterSetFunc(void *param);
/** ˮ����Ͽ������� */
static void _userWaterAlluvialSetFunc(void *param);

#if 0
/** ��ԪӲ����Ϣ������Ͳ���� */
static void _userUnitHwInfoSetFeeder(void *param);
#endif
/** ��ԪӲ����Ϣ�����ϲ����� */
static void _userUnitHwInfoSetTrough(void *param);
/** ��ԪӲ����Ϣ����̽������ */
static void _userUnitHwInfoSetProbeType(void *param);
#if 0
/** ��ԪӲ����Ϣ����̽��߶� */
static void _userUnitHwInfoSetProbeHigh(void *param);
/** ��ԪӲ����Ϣ���ô������� */
static void _userUnitHwInfoSetTouchType(void *param);
/** ��ԪӲ����Ϣ�����Ϲ޼�� */
static void _userUnitHwInfoSetFoodSpace(void *param);
/** ��ԪӲ����Ϣ����ˮԴ��� */
static void _userUnitHwInfoSetWaterSpace(void *param);
/** ��ԪӲ����Ϣ���õ������ */
static void _userUnitHwInfoSetMotorType(void *param);
#endif
/** ��ԪӲ����Ϣ����һ�巧���� */
static void _userUnitHwInfoSetFlowType(void *param);

/** ��ιģ���䷽���� */
static void _userFeedModelRecipeSet(void *param);
/** ��ιģ�ͺ��������� */
static void _userFeedModelPowderSet(void *param);
/** ��ιģ����ʪ������ */
static void _userFeedModelTempHumiSet(void *param);
/** ��ιģ�ͽ������� */
static void _userFeedModelHealthSet(void *param);
/** ��ιģ���������� */
static void _userFeedModelWeightSet(void *param);
/** ��ιģ�Ϳ������� */
static void _userFeedModelOpenSet(void *param);

/** �ַ���תΪ��ά�벢��ʾ���� */
static void _userMakeQrcode(uint8_t addr_x, uint8_t addr_y, uint8_t* qrstr);

/** ��������mqtt���� */
static int32_t _userMqttRestart(void);

/* ��������: min <= max */
static int _userGuiValueIncStep(int value, int min, int max, int step);
/* ��������: min <= max */
static int _userGuiValueDecStep(int value, int min, int max, int step);

/************************************************/
/*                 �ڲ����漯������               */
/************************************************/
/* ���� */
/** ������(����) */
const static lkdWin s_win_home                   = {0, 0, 128, 64, NULL,                   _userWinHomeFunc,      NULL};
/** ע����棺��ݼ��¼�����/����->ע�� */
const static lkdWin s_win_lora_reg               = {0, 0, 128, 64, (uint8_t*)"ע��",        _userWinLoraRegFunc, NULL};
/** ��ά����ʾ���棺��ݼ��Ͻ�����  */
const static lkdWin s_win_Qrcode                 = {0, 0, 128, 64, NULL,                   _userWinQrCodeFunc,    NULL};
/** �������ն�ͨ�ųɹ�����Ϣ���棺��ݼ��������/Lora->ͨ�� */
const static lkdWin s_win_comm_info              = {0, 0, 128, 64, (uint8_t*)"�ն�ͨ������",  _userCommSuccRateFunc, NULL};
/** ������lora�ն��ź�ǿ����Ϣ���棺��ݼ��Ҽ�����/Lora->ǿ�� */
const static lkdWin s_win_lora_rssi              = {0, 0, 128, 64, (uint8_t*)"Lora�ź�ǿ��", _userLoraRssiFunc,     NULL};

/** ��ַ��ͻ��ʾ���� */
const static lkdWin s_win_addr_collide           = {10, 5, 108, 54, (uint8_t*)"��ַ��ͻ",    _userAddrCollideFunc, NULL};

/** �˵����棺ȷ�ϼ����� */
const static lkdWin s_win_menu                   = {0, 0, 128, 64, NULL,                   _userMenuFun,          NULL};

/** �����������ý��棺����->���� */
const static lkdWin s_win_unit_type_set          = {0, 0, 128, 64, (uint8_t*)"������������", _userUnitTypeSetFunc, NULL};
/** ��Ԫ�����ý��棺����->��Ԫ�� */
const static lkdWin s_win_unit_num_set           = {0, 0, 128, 64, (uint8_t*)"��Ԫ����", _userUnitNumSetFunc, NULL};
/** ��Ԫ��λ���ý��棺����->��λ */
const static lkdWin s_win_term_raw_set           = {0, 0, 128, 64, NULL, _userUnitTermRawSetFunc, NULL};

/** ��ιƽ̨ip�Ͷ˿����ý��棺����->������ */
const static lkdWin s_win_server_info_set        = {0, 0, 128, 64, (uint8_t *)"������IP����", _userServerInfoSetFunc, NULL};
/** ��ιƽ̨ѡ����棺����->ƽ̨ */
const static lkdWin s_win_mqtt_select_set        = {0, 0, 128, 64, (uint8_t *)"ƽ̨ѡ��", _userMqttSelectSetFunc, NULL};
#if 0
/** �û�����ѡ����棺����->���� */
const static lkdWin s_win_upgrade_mode_set       = {0, 0, 128, 64, (uint8_t *)"�ն�����", _userUpgradeModeSetFunc, NULL};
#endif

/** ͨ��ģʽ���ý��棺ͨ�� */
const static lkdWin s_win_unit_comm_type_set     = {0, 0, 128, 64, (uint8_t *)"ͨѶģʽ", _userUnitCommTypeSetFunc , NULL};

/** ��ι�����·���ʽ���ý��棺���� */
const static lkdWin s_win_unit_feed_version_set  = {0, 0, 128, 64, (uint8_t *)"��ι�����·���ʽ", _userUnitFeedVersionSetFunc, NULL};

/** Lora�ź�ǿ�Ȳ��Խ��棺����->Lora���� */
const static lkdWin s_win_lora_test              = {0, 0, 128, 64, (uint8_t *)"Lora�ź�ǿ�Ȳ���", _userLorsTestFunc, NULL};
#if 0
/** ���������·����棺����->Ba���� */
const static lkdWin s_win_feed_param_set_default = {0, 0, 128, 64, (uint8_t *)"���������·�", _userFeedParamSetDefault, NULL};
/** ����->Re��¼ */
const static lkdWin Gui_Recode               = {0, 0, 128, 64, NULL, Gui_Recode_Fun, NULL};
#endif
/** ����->Re��� */
const static lkdWin s_win_term_recode_clear      = {0, 0, 128, 64, NULL, _userTermRecodeClear, NULL}; /*�洢�������ʼ��*/

/** ����->�ŵ� */
const static lkdWin s_win_lora_channel_set       = {0, 0, 128, 64, (uint8_t *)"�����ŵ�����", _userLoraChannelSetFunc, NULL};
/** ����->���� */
const static lkdWin s_win_lora_txpower_set       = {0, 0, 128, 64, (uint8_t *)"����ͨ�Ź���", _userLoraTxpowerSetFunc, NULL};
/** ����->��Ƶ */
const static lkdWin s_win_lora_sf_set            = {0, 0, 128, 64, (uint8_t *)"������Ƶ����", _userLoraSfSetFunc, NULL};
#if 0
/** ����->������ */
const static lkdWin s_win_lora_cr_set            = {0, 0, 128, 64, (uint8_t *)"���߱�����", _userLoraCrSetFunc, NULL};
/** ����->���� */
const static lkdWin s_win_lora_bw_set            = {0, 0, 128, 64, (uint8_t *)"���ߴ���", _userLoraBwSetFunc, NULL};
/** ����->�Զ� */
const static lkdWin s_win_lora_ch_adjust_set     = {0, 0, 128, 64, (uint8_t *)"�����ŵ��Զ���������", _userLoraChAdjustSetFunc, NULL};
/** ����->ִ�� */
const static lkdWin s_win_lora_ch_adjust_start   = {0, 0, 128, 64, (uint8_t *)"ִ�������ŵ��Զ�����", _userLoraChAdjustStartFunc, NULL};
#endif
/** Lora->��ʼ�� */
const static lkdWin s_win_lora_default           = {0, 0, 128, 64, (uint8_t *)"lora���ó�ʼ��", _userLoraSetDefaultFunc, NULL};

#if 0
/** ����->��ˮ */
const static lkdWin s_win_config_no_water        = {0, 0, 128, 64, (uint8_t *)"��ˮ����ģʽ", _userConfigNoWaterFunc , NULL};
/** ����->����->ʱ�� */
const static lkdWin s_win_sell_time_set          = {0, 0, 128, 64, (uint8_t *)"ʱ������", _userConfigSellTimeSetFunc, NULL};
/** ����->����->��λ */
const static lkdWin s_win_sell_switch_set        = {0, 0, 128, 64, (uint8_t *)"��λ����", _userConfigSellSwitchSetFunc , NULL};
#endif
/**����->��ֵ */
const static lkdWin s_win_probe_adc_set          = {0, 0, 128, 64, (uint8_t *)"����̽����ֵ����", _userConfigProbeAdcSetFunc , NULL};
/** ����->���->���� */
const static lkdWin s_win_food_clear_switch      = {0, 0, 128, 64, (uint8_t *)"��̬��ۿ���", _userFoodClearSwitchSetFunc, NULL};
/** ����->���->��� */
const static lkdWin s_win_food_clear_check_time  = {0, 0, 128, 64, (uint8_t *)"��̬��ۼ��ʱ��", _userFoodClearCheckTimeSetFunc, NULL};
/** ����->���->��� */
const static lkdWin s_win_food_clear_food_time   = {0, 0, 128, 64, (uint8_t *)"��̬������ʱ��", _userFoodClearFoodTimeSetFunc, NULL};
/** ����->���->��ˮ */
const static lkdWin s_win_food_clear_water_time  = {0, 0, 128, 64, (uint8_t *)"��̬�����ˮʱ��", _userFoodClearWaterTimeSetFunc, NULL};
/** ����->���->ʼ���� */
const static lkdWin s_win_food_clear_start_age   = {0, 0, 128, 64, (uint8_t *)"��̬�����ʼ����", _userFoodClearStartAgeSetFunc, NULL};
/** ����->���->������ */
const static lkdWin s_win_food_clear_stop_age    = {0, 0, 128, 64, (uint8_t *)"��̬��۽�������", _userFoodClearStopAgeSetFunc, NULL};
#if 0
/** ����->���� */
const static lkdWin s_win_protect_time           = {0, 0, 128, 64, (uint8_t *)"����ʱ�䵥λ����", _userProtectTimeSetFunc, NULL};
#endif
/** ����->��ˮ */
const static lkdWin s_win_water_mode_set         = {0, 0, 128, 64, (uint8_t *)"��ˮ��ʽ����", _userWaterModeSetFunc, NULL};
/** ����->̽��->������ */
const static lkdWin s_win_probe_sen_set          = {0, 0, 128, 64, (uint8_t *)"��˸�̽��������", _userProbeSenSetFunc, NULL};
/** ����->���� */
const static lkdWin s_win_pluse_cal              = {0, 0, 128, 64, (uint8_t *)"�Զ�Уˮ��ʽ", _userProtectPulseCalFunc, NULL};
/** ����->����->���� */
const static lkdWin s_win_Valve_Speed            = {0, 0, 128, 64, (uint8_t *)"һ�巧����", _userValveSpeedSetFunc, NULL};
#if 0
/** ����->����->���� */
const static lkdWin s_win_water_speed_diff       = {0, 0, 128, 64, (uint8_t *)"���첹��", _userWaterSpeedDiffSetFunc, NULL};
#endif
/** ����->ǿ��->���� */
const static lkdWin s_win_Strong_Food = {0, 0, 128, 64, (uint8_t *)"ǿ����������", _userStrongFoodGearSetFunc, NULL};
/** ����->ǿ��->��ˮ */
const static lkdWin s_win_Strong_Water = {0, 0, 128, 64, (uint8_t *)"ǿ����ˮ����", _userStrongWaterGearSetFunc, NULL};
/** ����->��ˮ˳�� */
const static lkdWin s_win_unit_food_mode_set     = {0, 0, 128, 64, (uint8_t *)"��ι���Ϸ�ʽ",_userUnitFoodModeSetFunc , NULL};
/** ����->����->��� */
const static lkdWin s_win_mr_feed_clean_set  = {0, 0, 128, 64, NULL, _userMrFeedCleanSetFunc, NULL};
/** ����->����->���� */
const static lkdWin s_win_mr_touch_delay_set = {0, 0, 128, 64, NULL, _userMrTouchDelaySetFunc, NULL};
/** ����->����->ˮ���� */
const static lkdWin s_win_mr_feed_water_set  = {0, 0, 128, 64, (uint8_t *)"������ˮ��ز���", _userMrFeedWaterSetFunc, NULL};
/** ����->ˮ�� */
const static lkdWin s_win_water_alluvial_set = {0, 0, 128, 64, (uint8_t *)"ˮ����Ͽ���", _userWaterAlluvialSetFunc, NULL};
#if 0
/* �豸->Ӳ��->���� */
const static lkdWin s_win_uint_hw_info_feeder_set      = {0, 0, 128, 64, (uint8_t *)"��������",   _userUnitHwInfoSetFeeder,    NULL};
#endif
/* �豸->Ӳ��->�ϲ� */
const static lkdWin s_win_uint_hw_info_trough_set      = {0, 0, 128, 64, (uint8_t *)"�ϲ�����",   _userUnitHwInfoSetTrough,    NULL};
/* �豸->Ӳ��->̽������ */
const static lkdWin s_win_uint_hw_info_probe_type_set  = {0, 0, 128, 64, (uint8_t *)"̽������",   _userUnitHwInfoSetProbeType, NULL};
#if 0
/* �豸->Ӳ��->̽��߶� */
const static lkdWin s_win_uint_hw_info_probe_high_set  = {0, 0, 128, 64, (uint8_t *)"̽��߶�",   _userUnitHwInfoSetProbeHigh, NULL};
/* �豸->Ӳ��->���� */
const static lkdWin s_win_uint_hw_info_touch_type_set  = {0, 0, 128, 64, (uint8_t *)"����������", _userUnitHwInfoSetTouchType, NULL};
/* �豸->Ӳ��->�Ͼ� */
const static lkdWin s_win_uint_hw_info_food_space_set  = {0, 0, 128, 64, (uint8_t *)"�Ϲ޼��",   _userUnitHwInfoSetFoodSpace, NULL};
/* �豸->Ӳ��->ˮ�� */
const static lkdWin s_win_uint_hw_info_water_space_set = {0, 0, 128, 64, (uint8_t *)"ˮԴ���",   _userUnitHwInfoSetWaterSpace, NULL};
/* �豸->Ӳ��->��� */
const static lkdWin s_win_uint_hw_info_motor_type_set  = {0, 0, 128, 64, (uint8_t *)"���",       _userUnitHwInfoSetMotorType, NULL};
#endif
/* �豸->Ӳ��->һ�巧 */
const static lkdWin s_win_uint_hw_info_flow_type_set   = {0, 0, 128, 64, (uint8_t *)"һ�巧",     _userUnitHwInfoSetFlowType,  NULL};
/* ģ��->�䷽ */
const static lkdWin s_win_feed_model_recipe_set   = {0, 0, 128, 64, (uint8_t *)"�䷽����",   _userFeedModelRecipeSet,  NULL};
/* ģ��->���� */
const static lkdWin s_win_feed_model_powder_set   = {0, 0, 128, 64, (uint8_t *)"����������", _userFeedModelPowderSet,  NULL};
/* ģ��->��ʪ�� */
const static lkdWin s_win_feed_model_temphumi_set = {0, 0, 128, 64, (uint8_t *)"��ʪ������", _userFeedModelTempHumiSet,  NULL};
/* ģ��->���� */
const static lkdWin s_win_feed_model_health_set   = {0, 0, 128, 64, (uint8_t *)"��������",   _userFeedModelHealthSet,  NULL};
/* ģ��->���� */
const static lkdWin s_win_feed_model_weight_set   = {0, 0, 128, 64, (uint8_t *)"��������",   _userFeedModelWeightSet,  NULL};
/* ģ��->���� */
const static lkdWin s_win_feed_model_open_set     = {0, 0, 128, 64, (uint8_t *)"ģ�Ϳ���",   _userFeedModelOpenSet,  NULL};

/** ��ʾ���� */
const static lkdWin s_win_saving                 = {21, 20, 88, 30, (uint8_t *)"������...", NULL, NULL};
const static lkdWin s_win_save_success           = {21, 20, 88, 30, (uint8_t *)"����ɹ�", NULL, NULL};
const static lkdWin s_win_set_complete           = {21, 20, 88, 30, (uint8_t *)"�������", NULL, NULL};
const static lkdWin s_win_set_failed             = {21, 20, 88, 30, (uint8_t *)"����ʧ��", NULL, NULL};
const static lkdWin s_win_set_ing                = {21, 20, 88, 30, (uint8_t *)"����������...", NULL, NULL};
const static lkdWin s_win_lora_initing           = {21, 20, 88, 30, (uint8_t *)"Lora��ʼ����...", NULL, NULL};
const static lkdWin s_win_gateway_update         = {10, 5, 108, 54, (uint8_t *)"�����������������", NULL, NULL};

/* �˵� */
/* ����->��� �����˵� */
const static lkdMenuNode Node0_9_4_6 = {0x946, (uint8_t *)"������", NULL, NULL, &s_win_food_clear_stop_age};
const static lkdMenuNode Node0_9_4_5 = {0x945, (uint8_t *)"ʼ����", &Node0_9_4_6, NULL, &s_win_food_clear_start_age};
const static lkdMenuNode Node0_9_4_4 = {0x944, (uint8_t *)"��ˮ", &Node0_9_4_5, NULL, &s_win_food_clear_water_time};
const static lkdMenuNode Node0_9_4_3 = {0x943, (uint8_t *)"���", &Node0_9_4_4, NULL, &s_win_food_clear_food_time};
const static lkdMenuNode Node0_9_4_2 = {0x942, (uint8_t *)"���", &Node0_9_4_3, NULL, &s_win_food_clear_check_time};
const static lkdMenuNode Node0_9_4_1 = {0x941, (uint8_t *)"����", &Node0_9_4_2, NULL, &s_win_food_clear_switch};

/* ����->���� �����˵� */
#if 0
const static lkdMenuNode Node0_9_2_2 = {0x922, (uint8_t *)"��λ", NULL, NULL, &s_win_sell_switch_set};
const static lkdMenuNode Node0_9_2_1 = {0x921, (uint8_t *)"ʱ��", &Node0_9_2_2, NULL, &s_win_sell_time_set};
#endif

/* ����->̽�� �����˵� */
const static lkdMenuNode Node0_9_7_1 = {0x971, (uint8_t *)"������", NULL, NULL, &s_win_probe_sen_set};

/* ����->ǿ�� �����˵� */
const static lkdMenuNode Node0_9_8_2 = {0x982, (uint8_t *)"��ˮ", NULL, NULL, &s_win_Strong_Water};
const static lkdMenuNode Node0_9_8_1 = {0x981, (uint8_t *)"����", &Node0_9_8_2, NULL, &s_win_Strong_Food};

/* ����->���� �����˵� */
#if 0
const static lkdMenuNode Node0_9_B_2 = {0x9B2, (uint8_t *)"����", NULL, NULL, &s_win_water_speed_diff};
const static lkdMenuNode Node0_9_B_1 = {0x9B1, (uint8_t *)"����", &Node0_9_B_2, NULL, &s_win_Valve_Speed};
#endif

/* ����->���� �����˵� */
const static lkdMenuNode Node0_9_C_3 = {0x9C3, (uint8_t *)"ˮ����", NULL,         NULL, &s_win_mr_feed_water_set};
const static lkdMenuNode Node0_9_C_2 = {0x9C2, (uint8_t *)"����",   &Node0_9_C_3, NULL, &s_win_mr_touch_delay_set};
const static lkdMenuNode Node0_9_C_1 = {0x9C1, (uint8_t *)"���",   &Node0_9_C_2, NULL, &s_win_mr_feed_clean_set};

/* �豸->Ӳ�� �����˵� */
#if 0
const static lkdMenuNode Node0_5_3_9 = {0x539, (uint8_t *)"ˮ��",     NULL,         NULL, &s_win_uint_hw_info_water_space_set};
const static lkdMenuNode Node0_5_3_8 = {0x538, (uint8_t *)"�Ͼ�",     &Node0_5_3_9, NULL, &s_win_uint_hw_info_food_space_set};
const static lkdMenuNode Node0_5_3_7 = {0x537, (uint8_t *)"����",     &Node0_5_3_8, NULL, &s_win_uint_hw_info_touch_type_set};
const static lkdMenuNode Node0_5_3_6 = {0x536, (uint8_t *)"̽��߶�", &Node0_5_3_7, NULL, &s_win_uint_hw_info_probe_high_set};
const static lkdMenuNode Node0_5_3_5 = {0x535, (uint8_t *)"���",     &Node0_5_3_6, NULL, &s_win_uint_hw_info_motor_type_set};
#endif
const static lkdMenuNode Node0_5_3_4 = {0x534, (uint8_t *)"һ�巧",   NULL, NULL, &s_win_uint_hw_info_flow_type_set};
const static lkdMenuNode Node0_5_3_3 = {0x533, (uint8_t *)"̽������", &Node0_5_3_4, NULL, &s_win_uint_hw_info_probe_type_set};
const static lkdMenuNode Node0_5_3_2 = {0x532, (uint8_t *)"�ϲ�",     &Node0_5_3_3, NULL, &s_win_uint_hw_info_trough_set};
#if 0
const static lkdMenuNode Node0_5_3_1 = {0x531, (uint8_t *)"����",     &Node0_5_3_2, NULL, &s_win_uint_hw_info_feeder_set};

/* �豸->���ư� �����˵� */
const static lkdMenuNode Node0_5_2_1 = {0x521, (uint8_t *)"����", NULL, NULL, &s_win_unit_food_mode_set};
#endif
/* ģ�� �����˵� */
const static lkdMenuNode Node0_A_6   = {0xA6, (uint8_t *)"����",   NULL, NULL, &s_win_feed_model_open_set};
const static lkdMenuNode Node0_A_5   = {0xA5, (uint8_t *)"����",   &Node0_A_6, NULL, &s_win_feed_model_weight_set};
const static lkdMenuNode Node0_A_4   = {0xA4, (uint8_t *)"����",   &Node0_A_5, NULL, &s_win_feed_model_health_set};
const static lkdMenuNode Node0_A_3   = {0xA3, (uint8_t *)"��ʪ��", &Node0_A_4, NULL, &s_win_feed_model_temphumi_set};
const static lkdMenuNode Node0_A_2   = {0xA2, (uint8_t *)"����",   &Node0_A_3, NULL, &s_win_feed_model_powder_set};
const static lkdMenuNode Node0_A_1   = {0xA1, (uint8_t *)"�䷽",   &Node0_A_2, NULL, &s_win_feed_model_recipe_set};

/* ���� �����˵� */
const static lkdMenuNode Node0_9_D   = {0x9D, (uint8_t *)"ˮ��", NULL,       NULL, &s_win_water_alluvial_set};
const static lkdMenuNode Node0_9_C   = {0x9C, (uint8_t *)"����", &Node0_9_D, &Node0_9_C_1, NULL};
#if 0
const static lkdMenuNode Node0_9_B   = {0x9B, (uint8_t *)"����", &Node0_9_C, &Node0_9_B_1, NULL};
#endif
const static lkdMenuNode Node0_9_B   = {0x9B, (uint8_t *)"����", &Node0_9_C, NULL, &s_win_Valve_Speed};
const static lkdMenuNode Node0_9_A   = {0x9A, (uint8_t *)"��ˮ˳��", &Node0_9_B, NULL, &s_win_unit_food_mode_set};
const static lkdMenuNode Node0_9_9   = {0x99, (uint8_t *)"Уˮ", &Node0_9_A, NULL, &s_win_pluse_cal};
const static lkdMenuNode Node0_9_8   = {0x98, (uint8_t *)"ǿ��", &Node0_9_9, &Node0_9_8_1, NULL};
const static lkdMenuNode Node0_9_7   = {0x97, (uint8_t *)"̽��", &Node0_9_8, &Node0_9_7_1, NULL};
const static lkdMenuNode Node0_9_6   = {0x96, (uint8_t *)"Сˮ��", &Node0_9_7, NULL, &s_win_water_mode_set};
#if 0
const static lkdMenuNode Node0_9_5   = {0x95, (uint8_t *)"����", &Node0_9_6, NULL, &s_win_protect_time};
#endif
const static lkdMenuNode Node0_9_4   = {0x94, (uint8_t *)"���", &Node0_9_6, &Node0_9_4_1, NULL};
const static lkdMenuNode Node0_9_3   = {0x93, (uint8_t *)"��ֵ", &Node0_9_4, NULL, &s_win_probe_adc_set};
#if 0
const static lkdMenuNode Node0_9_2   = {0x92, (uint8_t *)"����", &Node0_9_3, &Node0_9_2_1, NULL};
const static lkdMenuNode Node0_9_1   = {0x91, (uint8_t *)"��ˮ", &Node0_9_2, NULL, &s_win_config_no_water};
#endif

/* lora�����˵� */
#if 0
const static lkdMenuNode Node0_8_3 = {0x83, (uint8_t *)"��ʼ��", NULL, NULL, &s_win_lora_default};
const static lkdMenuNode Node0_8_2 = {0x82, (uint8_t *)"ǿ��", &Node0_8_3, NULL, &s_win_lora_rssi};
const static lkdMenuNode Node0_8_1 = {0x81, (uint8_t *)"ͨ��", &Node0_8_2, NULL, &s_win_comm_info};
#endif

/* ���߶����˵� */
#if 0
const static lkdMenuNode Node0_7_7 = {0x77, (uint8_t *)"ִ��", NULL, NULL, &s_win_lora_ch_adjust_start};
const static lkdMenuNode Node0_7_6 = {0x76, (uint8_t *)"�Զ�", &Node0_7_7, NULL, &s_win_lora_ch_adjust_set};
const static lkdMenuNode Node0_7_5 = {0x75, (uint8_t *)"����", &Node0_7_6, NULL, &s_win_lora_bw_set};
const static lkdMenuNode Node0_7_4 = {0x74, (uint8_t *)"������", &Node0_7_5, NULL, &s_win_lora_cr_set};
#endif
const static lkdMenuNode Node0_7_4 = {0x74, (uint8_t *)"��ʼ��", NULL, NULL, &s_win_lora_default};
const static lkdMenuNode Node0_7_3 = {0x73, (uint8_t *)"��Ƶ", &Node0_7_4, NULL, &s_win_lora_sf_set};
const static lkdMenuNode Node0_7_2 = {0x72, (uint8_t *)"����", &Node0_7_3, NULL, &s_win_lora_txpower_set};
const static lkdMenuNode Node0_7_1 = {0x71, (uint8_t *)"�ŵ�", &Node0_7_2, NULL, &s_win_lora_channel_set};

/* ���� �����˵� */
const static lkdMenuNode Node0_6_6 = {0x66, (uint8_t *)"Re���", NULL, NULL, &s_win_term_recode_clear};
#if 0
const static lkdMenuNode Node0_6_5 = {0x65, (uint8_t *)"Re��¼", &Node0_6_6, NULL, &Gui_Recode};
#endif
const static lkdMenuNode Node0_6_4 = {0x64, (uint8_t *)"ע��", &Node0_6_6, NULL, &s_win_lora_reg};
#if 0
const static lkdMenuNode Node0_6_3 = {0x63, (uint8_t *)"Ba����", &Node0_6_4, NULL, &s_win_feed_param_set_default};
const static lkdMenuNode Node0_6_2 = {0x62, (uint8_t *)"Ba����", &Node0_6_3, NULL, NULL};
#endif
const static lkdMenuNode Node0_6_1 = {0x61, (uint8_t *)"Lora����", &Node0_6_4, NULL, &s_win_lora_test};

/* �豸 �����˵� */
const static lkdMenuNode Node0_5_3 = {0x53, (uint8_t *)"Ӳ��",   NULL,       &Node0_5_3_2, NULL};
const static lkdMenuNode Node0_5_2 = {0x52, (uint8_t *)"���ư�", &Node0_5_3, NULL, NULL};
const static lkdMenuNode Node0_5_1 = {0x51, (uint8_t *)"ͨ�Ű�", &Node0_5_2, NULL, NULL};

/* ���� �����˵� */
#if 0
const static lkdMenuNode Node0_2_3 = {0x23, (uint8_t *)"����", NULL, NULL, &s_win_upgrade_mode_set};
#endif
const static lkdMenuNode Node0_2_2 = {0x22, (uint8_t *)"ƽ̨", NULL, NULL, &s_win_mqtt_select_set};
const static lkdMenuNode Node0_2_1 = {0x21, (uint8_t *)"������", &Node0_2_2, NULL, &s_win_server_info_set};

/* ���� �����˵� */
const static lkdMenuNode Node0_1_3 = {0x13, (uint8_t *)"��λ", NULL, NULL, &s_win_term_raw_set};
const static lkdMenuNode Node0_1_2 = {0x12, (uint8_t *)"��Ԫ��", &Node0_1_3, NULL, &s_win_unit_num_set};
const static lkdMenuNode Node0_1_1 = {0x11, (uint8_t *)"����", &Node0_1_2, NULL, &s_win_unit_type_set};

/* һ���˵� */
const static lkdMenuNode Node0_A = {0xA, (uint8_t *)"ģ��", NULL, &Node0_A_1, NULL};
const static lkdMenuNode Node0_9 = {0x9, (uint8_t *)"����", &Node0_A, &Node0_9_3, NULL};
// const static lkdMenuNode Node0_8 = {0x8, (uint8_t *)"Lora", &Node0_9, &Node0_8_1, NULL};
const static lkdMenuNode Node0_7 = {0x7, (uint8_t *)"����", &Node0_9, &Node0_7_1, NULL};
const static lkdMenuNode Node0_6 = {0x6, (uint8_t *)"����", &Node0_7, &Node0_6_1, NULL};
const static lkdMenuNode Node0_5 = {0x5, (uint8_t *)"�豸", &Node0_6, &Node0_5_1, NULL};
const static lkdMenuNode Node0_4 = {0x4, (uint8_t *)"����", &Node0_5, NULL, &s_win_unit_feed_version_set};
const static lkdMenuNode Node0_3 = {0x3, (uint8_t *)"ͨ��", &Node0_4, NULL, &s_win_unit_comm_type_set};
const static lkdMenuNode Node0_2 = {0x2, (uint8_t *)"����", &Node0_3, &Node0_2_1, NULL};
const static lkdMenuNode Node0_1 = {0x1, (uint8_t *)"����", &Node0_2, &Node0_1_1, NULL};
/* ���˵� */
const static lkdMenuNode Node0 = {0x0, (uint8_t *)"root",  NULL,       &Node0_1,      NULL};

/************************************************/
/*                 ���ں���ʵ��                  */
/************************************************/
/*�˵����*/
static void MenuItemDealWith(const lkdMenuNode *pNode)
{
    if(pNode->pSon != NULL){/*չ��ѡ�нڵ�Ĳ˵�*/
        if(pNode->user == NULL)
        {
            GuiMenuCurrentNodeSonUnfold(&HmiMenu);
        }
        else
        {
            DrawNodes(&HmiMenu, HmiMenu.pDraw, HmiMenu.cx, HmiMenu.cy, HmiMenu.index, HmiMenu.cNum);
        }
    }
    else if(pNode->user != NULL){/*�򿪲˵���Ӧ�Ĵ���*/
        GuiWinAdd(pNode->user);
    }
}

/*�˵����ƺ���*/
static void MenuControlFun(void)
{
    switch(s_i_key_status)
    {
        case KEY_UP_DOWN:GuiMenuItemUpMove(&HmiMenu);break; /*button_up_down*/
        case KEY_DOWN_DOWN:GuiMenuItemDownMove(&HmiMenu);break;/*button_down_down*/
        case KEY_LEFT_DOWN:
        case KEY_CANCEL_DOWN:
            GuiMenuCurrentNodeHide(&HmiMenu);
            if(HmiMenu.count == 0)/*��⵽�˵��˳��ź�*/
            {
                GuiWinDeleteTop();
            }break;
        case KEY_RIGHT_DOWN:
        case KEY_OK_DOWN:
            MenuItemDealWith(GuiMenuGetCurrentpNode(&HmiMenu));
            break;
        default:
            break;
    }
}

/*������������ʾ*/
static void _guiPowerOnProgress(void)
{
    lkdProgress tProGress;
    tProGress.x = 21;
    tProGress.y = 35;
    tProGress.wide = 80;
    tProGress.high = 5;
    tProGress.ratio = 0;

    for(uint8_t i = 0; i < 100; i ++){
        tProGress.ratio = i;        /* ���ȿ��� */
        GuiProGress(&tProGress);    /* �������� */
        GuiRowTextPlus(0, 3, 127, FONT_MID, (uint8_t *)"loading...");
        GuiUpdateDisplayAll();/* ���� */

        /* ����ط�����ܴﵽΪ48��, ͨѶ������ϵ�����λ����Ϊ120��λʱ, ʱ����ܴﵽ48������;
           ���ǲ�Ӱ��, ���������������԰�����һ�ᣬ��һ���ϵ�ͨ����26�룻����һ����11������ */
        if (s_gui_process_end == 0)
        {
            rt_thread_mdelay(200);
        }
        else
        {
            rt_thread_mdelay(10);
        }
    }
    CloseLcdDisplay();
}

/*�˵���ʼ��*/
static void UserMenuInit(void)
{
    HmiMenu.x = 1;
    HmiMenu.y = 1;
    HmiMenu.wide = 126;     //@ ����Ƿ��� GUIXMAX ���
    HmiMenu.hight = 62;     //@ ����Ƿ��� GUIYMAX ���
    HmiMenu.Itemshigh = 15;/*�˵���Ŀ�߶�*/
    HmiMenu.ItemsWide = 42;/*�˵���Ŀ���*/
    HmiMenu.stack = userMenuStack;
    HmiMenu.stackNum = 8;   //@ �� GUIWINMANAGE_NUM ��أ���������9
    HmiMenu.Root = &Node0;/* �˵�������˵��ڵ�󶨵�һ�� */
    HmiMenu.MenuDealWithFun = MenuControlFun;/* �˵����ƻص����� */
    GuiMenuInit(&HmiMenu);
}

/* ����ע����� */
static void _userWinRemoteReg(void)
{
    if (paramsCheckTermCommMode(E_TERM_COMM_MODE_REG) != 1)
    {
        GuiWinAdd(&s_win_lora_reg);
    }
}

/* �ֿ����ؽ�����ʾ */
static void _userWinFontLoadOnProcess(void)
{
    char data[30]={0};
    lkdProgress tProGress;

    tProGress.x = 21;
    tProGress.y = 35;
    tProGress.wide = 80;
    tProGress.high = 5;
    tProGress.ratio = 0;
    if (appOtaGetCurUpdateType() == OTA_FIEL_TYPE_FONT_CN_E)
    {
        tProGress.ratio = paramsOtaGetCurUpgradeProgress();
        if (tProGress.ratio > 100)
        {
            tProGress.ratio = 100;
        }
    }

    GuiProGress(&tProGress); /* �������� */
    MY_SNPRINTF(data, sizeof(data), "Loading font [%03d]", tProGress.ratio);
    GuiRowTextPlus(0, 1, 128, FONT_MID, (uint8_t *)data);
    GuiUpdateDisplayAll(); /* ���� */
}

/* ������ */
static void _userWinHomeFunc(void *param)
{
    const char* const comm_type[] = {"����", "����"};

    char str[GUI_ONE_LINE_MAX_SIZE];

    uint32_t timestamp;
    Plat_Time_T t_plat_time;

    uint8_t comm_type_index = 0;
    uint16_t unit_type = 0;
    uint16_t unit_num = 0;
    uint8_t net_ipv4[4] = {0};

    if (paramsTermCollideAddrGet() != 0)
    {
        if (MY_BIT_CHECK(s_gui_custom_flag, GUI_CUSTOM_FLAG_ADDR_COLLIDE) == 0)
        {
            MY_BIT_SET(s_gui_custom_flag, GUI_CUSTOM_FLAG_ADDR_COLLIDE);
            GuiWinAdd(&s_win_addr_collide);
        }
        return;
    }

#if 0
    if (s_short_key == KEY_UP_DOWN)
    {
        _userWinQrCodeFunc(NULL);
        return;
    }
    else if (s_short_key == KEY_LEFT_DOWN)
    {
        _userCommSuccRateFunc(NULL);
        return;
    }
    else if (s_short_key == KEY_RIGHT_DOWN)
    {
        _userLoraRssiFunc(NULL);
        return;
    }
#endif

    /* ��ʾ���ں�ʱ�� */
    timestamp = paramsGetGwTimestamp();
    platTimestampToDateTime(timestamp, &t_plat_time);
    MY_SNPRINTF(str, sizeof(str), "%04d-%02d-%02d %02d:%02d:%02d", t_plat_time.uh_year, t_plat_time.uc_mon,\
                t_plat_time.uc_day, t_plat_time.uc_hour, t_plat_time.uc_min, t_plat_time.uc_sec);
    GuiRowTextPlus(2, 0, 128, FONT_LEFT, (uint8_t *)str);

    /* ��ʾ�������ͺ�ͨ������ */
    if (paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_RS485)
    {
        comm_type_index = 1;
    }
    unit_type = paramsGetGwUnitType();
    unit_num  = paramsGwUnitNumGet();
    if (unit_type == Unit_Type_NO_TYPE)
    {
        MY_SNPRINTF(str, sizeof(str), "δѡ��:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_MR)
    {
        MY_SNPRINTF(str, sizeof(str), "������:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_HY)
    {
        MY_SNPRINTF(str, sizeof(str), "������:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_BY)
    {
        MY_SNPRINTF(str, sizeof(str), "������:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_YF)
    {
        MY_SNPRINTF(str, sizeof(str), "������:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_BY_YF)
    {
        MY_SNPRINTF(str, sizeof(str), "һ����:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    else
    {
        LOG_E("[%s] unknow unit type:%d", __func__, unit_type);
        Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
        if (pt_gw_config != NULL)
        {
            pt_gw_config->uh_unit_type = Unit_Type_NO_TYPE;
            paramsReleaseGwConfig();
        }
        else
        {
            LOG_E("[%s] get gw config failed", __func__);
        }
        MY_SNPRINTF(str, sizeof(str), "δѡ��:%02d|ͨ��:%s ", unit_num, comm_type[comm_type_index]);
    }
    GuiRowTextPlus(2, 1, 128, FONT_LEFT, (uint8_t *)str);

    /* ��ʾIP��ַ*/
    basicHalEtHIpv4Get(net_ipv4);
    MY_SNPRINTF(str, sizeof(str), "����IP:%d.%d.%d.%d", net_ipv4[0], net_ipv4[1], net_ipv4[2], net_ipv4[3]);
    GuiRowTextPlus(2, 2, 128, FONT_LEFT, (uint8_t *)str);

    /* ���ߣ���ʾ�汾���ŵ������ߣ���ʾ�汾����������*/
    if (comm_type_index == 1)   /* ���� */
    {
        uint32_t start_cnt = paramsGetGwStartCnt();
#if (GW_APP_FW > 999999)
        MY_SNPRINTF(str, sizeof(str), "V%d.%d.%02d.%02d.%d����:%3d", GW_APP_FW/1000000, (GW_APP_FW/100000)%10, (GW_APP_FW/1000)%100, (GW_APP_FW/10)%100, (GW_APP_FW%10), start_cnt);
#elif (GW_APP_FW > 9999)
        MY_SNPRINTF(str, sizeof(str), "V%02d.%02d.%02d ����:%3d", GW_APP_FW/10000, (GW_APP_FW/100)%100, GW_APP_FW%100, start_cnt);
#else
        MY_SNPRINTF(str, sizeof(str), "�汾:%d.%d.%02d����:%3d", GW_APP_FW/1000, (GW_APP_FW/100)%10, GW_APP_FW%100, start_cnt);
#endif
    }
    else    /* ���� */
    {
#if (GW_APP_FW > 999999)
        MY_SNPRINTF(str, sizeof(str), "V%d.%d.%02d.%02d.%d �ŵ�:%3d", GW_APP_FW/1000000, (GW_APP_FW/100000)%10, (GW_APP_FW/1000)%100, (GW_APP_FW/10)%100, (GW_APP_FW%10), paramsLoraGetChannel());
#elif (GW_APP_FW > 9999)
        MY_SNPRINTF(str, sizeof(str), "V%02d.%02d.%02d �ŵ�:%3d", GW_APP_FW/10000, (GW_APP_FW/100)%100, GW_APP_FW%100, paramsLoraGetChannel());
#else
        MY_SNPRINTF(str, sizeof(str), "�汾:%d.%d.%02d �ŵ�:%3d", GW_APP_FW/1000, (GW_APP_FW/100)%10, GW_APP_FW%100, paramsLoraGetChannel());
#endif
    }
    GuiRowTextPlus(2, 3, 128, FONT_LEFT, (uint8_t *)str);
    GuiUpdateDisplayAll();

#if 0
    if (paramsLoraRadioInitErrorFlagCheck() == 1)
    {
        static uint8_t cnt_t = 0;
        if(cnt_t==0)
        {
            cnt_t = 1;
            GuiWinAdd(&win7);
        }
    }
#endif

    switch(s_i_key_status)
    {
        case KEY_LEFT_DOWN:
            GuiWinAdd(&s_win_comm_info);
            break;
        case KEY_UP_DOWN:
            GuiWinAdd(&s_win_Qrcode);
            break;
        case KEY_RIGHT_DOWN:
            GuiWinAdd(&s_win_lora_rssi);
            break;
        case KEY_DOWN_DOWN:
            if (comm_type_index == 0)
            {
                GuiWinAdd(&s_win_lora_reg);
            }
            break;
        case KEY_OK_DOWN:
            GuiWinAdd(&s_win_menu);
            break;
        default:
            break;
    }
}

/* ע�ᴰ�ڷ����� */
static void _userWinLoraRegFunc(void *param)
{

/* lora������ʱʱ�䣬��λms; ��������30���� */
#define LORA_INNET_TIMEOUT  (1800000)

    static uint8_t first_enter_mode = 0;

    uint32_t cur_tick = MY_TICK_GET();

    if (first_enter_flag == 0)
    {
        paramsSetTermCommMode(E_TERM_COMM_MODE_REG);

        midLoraInNet();     //�л���ע���ŵ����Լ�ע������ID
        // LOG_I("[%s] LoRa in register mode!", __func__);

        GuiRowTextPlus(5, 1, 120, FONT_LEFT, (uint8_t *)"ע��ģʽ�Ѿ�������");
        GuiRowTextPlus(5, 2, 120, FONT_LEFT, (uint8_t *)"��򿪿��ư�ע�Ṧ��");
        GuiUpdateDisplayAll();

        s_gui_wait_last_tick = cur_tick;
        first_enter_mode = paramsLoraGetRemoteRegFlag();

        first_enter_flag = 1;
    }

    /* ������������һ�����˳���1-��ʱ�˳���2-����Զ�̱�־���룬Զ�̱�־ȡ�����˳� */
    if (((uint32_t)(cur_tick-s_gui_wait_last_tick) > LORA_INNET_TIMEOUT)||
        ((first_enter_mode == 1)&&(paramsLoraGetRemoteRegFlag() == 0)))
    {
        goto exit;
    }

    switch(s_i_key_status)
    {
        case KEY_OK_DOWN:
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        default:
            break;
    }
    return;
exit:
    first_enter_flag = 0;
    paramsLoraSetRemoteRegFlag(0);
    paramsResetTermCommMode(E_TERM_COMM_MODE_REG);

    midLoraBackToCom(); //�л��ع����ŵ����Լ�����ID
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    // LOG_I("[lora] LoRa out register mode!");
}

/* ��ά�봰�ڷ����� */
static void _userWinQrCodeFunc(void *param)
{
    uint8_t mac_str[GUI_ONE_LINE_MAX_SIZE] = {0};

    MY_MEMSET(mac_str, 0, sizeof(mac_str));
    paramsGetGwMacStr(mac_str, sizeof(mac_str));

    _userMakeQrcode(10, 10, mac_str);
    GuiUpdateDisplayAll();

    switch(s_i_key_status)
    {
        case KEY_OK_DOWN:
        case KEY_CANCEL_DOWN:
            GuiWinDeleteTop();
            GuiMenuRedrawMenu(&HmiMenu);
            break;
        default:
            break;
    }
}

/* �������ն�ͨ�ųɹ��ʴ��ڷ����� */
static void _userCommSuccRateFunc(void *param)
{
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];
    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page = (term_num+2)/3;  /* ������һҳ�����һҳ */
    uint8_t cur_term_addr = cur_page * 3;
    uint8_t cur_line = 0;
    Term_List_T *pt_take_list = NULL;
    uint32_t success_rate;

    switch(s_i_key_status)
    {
        case KEY_RIGHT_DOWN:
        case KEY_DOWN_DOWN:
            if ((++cur_page) >= max_page)
            {
                cur_page = 0;
            }
            break;
        case KEY_LEFT_DOWN:
        case KEY_UP_DOWN:
            if (cur_page > 0)
            {
                cur_page--;
            }
            else
            {
                cur_page = max_page - 1;
            }
            break;
        case KEY_OK_DOWN:
        case KEY_CANCEL_DOWN:
            cur_page = 0;
            GuiWinDeleteTop();
            GuiMenuRedrawMenu(&HmiMenu);
            return;
            break;
        default:
            break;
    }

    while((cur_line++) < 3)
    {
        cur_term_addr++;
        MY_MEMSET(str, 0, sizeof(str));
        if (cur_term_addr <= term_num)
        {
            pt_take_list = paramsTakeTermList(cur_term_addr);
            if (pt_take_list != NULL)
            {
                if (pt_take_list->com_send_cnt != 0)
                {
                    success_rate = pt_take_list->com_success_cnt * 100 / pt_take_list->com_send_cnt;
                }
                else
                {
                    success_rate = 0;
                }
                MY_SNPRINTF(str, sizeof(str), "[%3d] s%3d,t%3d,%3d%%", cur_term_addr, pt_take_list->com_success_cnt,\
                            pt_take_list->com_send_cnt, success_rate);
                GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)str);
                paramsReleaseTermList();
                pt_take_list = NULL;
            }
            else
            {
                LOG_E("[%s] get take list failed", __func__);
            }
        }
        else
        {
            GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)"                        ");
        }
    }
    GuiUpdateDisplayAll();
}


/* ������Lora�ն�ͨ���ź�ǿ�ȴ��ڷ����� */
static void _userLoraRssiFunc(void *param)
{
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];
    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page = (term_num+2)/3;  /* ������һҳ�����һҳ */
    uint8_t cur_term_addr = cur_page * 3;
    uint8_t cur_line = 0;
    Term_List_T *pt_take_list = NULL;

    switch(s_i_key_status)
    {
        case KEY_RIGHT_DOWN:
        case KEY_DOWN_DOWN:
            if ((++cur_page) >= max_page)
            {
                cur_page = 0;
            }
            break;
        case KEY_LEFT_DOWN:
        case KEY_UP_DOWN:
            if (cur_page > 0)
            {
                cur_page--;
            }
            else
            {
                cur_page = max_page - 1;
            }
            break;
        case KEY_OK_DOWN:
        case KEY_CANCEL_DOWN:
            cur_page = 0;
            GuiWinDeleteTop();
            GuiMenuRedrawMenu(&HmiMenu);
            return;
            break;
        default:
            break;
    }

    while((cur_line++) < 3)
    {
        cur_term_addr++;
        MY_MEMSET(str, 0, sizeof(str));
        if (cur_term_addr <= term_num)
        {
            pt_take_list = paramsTakeTermList(cur_term_addr);
            if (pt_take_list != NULL)
            {
                MY_SNPRINTF(str, sizeof(str), "[%3d] G:%4d,T:%4d", cur_term_addr, pt_take_list->Rssi_Gateway,\
                            pt_take_list->Rssi_term);
                GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)str);
                paramsReleaseTermList();
                pt_take_list = NULL;
            }
            else
            {
                LOG_E("[%s] get take list failed", __func__);
            }
        }
        else
        {
            GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)"                      ");
        }
    }
    GuiUpdateDisplayAll();
}

/* ��ַ��ͻ���ڷ����� */
static void _userAddrCollideFunc(void *param)
{
    uint16_t collide_addr = paramsTermCollideAddrGet();
    uint16_t line_index = 0, line_addr = 0;
    char str[GUI_ONE_LINE_MAX_SIZE];

    switch(s_i_key_status)
    {
        case KEY_RIGHT_DOWN:
        case KEY_DOWN_DOWN:
        case KEY_LEFT_DOWN:
        case KEY_UP_DOWN:
        case KEY_OK_DOWN:
        case KEY_CANCEL_DOWN:
            goto exit;
        default:
            break;
    }
    if ((collide_addr == 0)||(collide_addr > paramsGetGwTermNum()))
    {
        LOG_W("[%s] collide addr(%d) invalid", __func__, collide_addr);
        goto exit;
    }
    if (Get_Line_Addr(collide_addr, &line_index, &line_addr) == 0)
    {
        LOG_W("[%s] collide addr(%d) get line failed", __func__, collide_addr);
        goto exit;
    }
    switch(line_index)
    {
        case 1:
            MY_SNPRINTF(str, sizeof(str), "һ�� %2d ��ַ��ͻ", line_addr);
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "���� %2d ��ַ��ͻ", line_addr);
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "���� %2d ��ַ��ͻ", line_addr);
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "���� %2d ��ַ��ͻ", line_addr);
            break;
        default:
            LOG_W("[%s] line index(%d:%d) unknown", __func__, line_index, line_addr);
            goto exit;
    }
    GuiRowTextPlus(10, 2, 108, FONT_MID, (uint8_t *)str);
    GuiUpdateDisplayAll();
    return;
exit:
    paramsTermCollideAddrSet(0);
    MY_BIT_RESET(s_gui_custom_flag, GUI_CUSTOM_FLAG_ADDR_COLLIDE);
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* �˵���������� */
static void _userMenuFun(void *param)
{
    /*�û��Զ������*/
    if(HmiMenu.count == 0)
    {
        GuiMenuCurrentNodeSonUnfold(&HmiMenu);
    }
    HmiMenu.MenuDealWithFun();
}

/* ��Ԫ�������÷����� */
static void _userUnitTypeSetFunc(void *param)
{
    static uint8_t unit_type_index = 0;
    uint16_t unit_type = 0;
    uint16_t old_unit_type;

    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (s_gui_saving != 0)
    {
        goto saving;
    }

    if (first_enter_flag == 0)
    {
        unit_type = paramsGetGwUnitType();
        switch(unit_type)
        {
            case Unit_Type_MR:
                unit_type_index = 1;
                break;
            case Unit_Type_HY:
                unit_type_index = 2;
                break;
            case Unit_Type_BY:
                unit_type_index = 3;
                break;
            case Unit_Type_YF:
                unit_type_index = 4;
                break;
            case Unit_Type_BY_YF:
                unit_type_index = 5;
                break;
            default:
                unit_type_index = 0;
                break;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            unit_type_index++;
            if (unit_type_index > 5)
                unit_type_index = 0;
            break;

        case KEY_DOWN_DOWN:
            if (unit_type_index > 0)
                unit_type_index--;
            else
                unit_type_index = 5;
            break;

        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:   /* ok���ں������⴦�� */
        default:
            break;
    }

    switch(unit_type_index)
    {
        case 1:
            unit_type = Unit_Type_MR;
            MY_SNPRINTF(str, sizeof(str), "%s", "������");
            break;
        case 2:
            unit_type = Unit_Type_HY;
            MY_SNPRINTF(str, sizeof(str), "%s", "������");
            break;
        case 3:
            unit_type = Unit_Type_BY;
            MY_SNPRINTF(str, sizeof(str), "%s", "������");
            break;
        case 4:
            unit_type = Unit_Type_YF;
            MY_SNPRINTF(str, sizeof(str), "%s", "������");
            break;
        case 5:
            unit_type = Unit_Type_BY_YF;
            MY_SNPRINTF(str, sizeof(str), "%s", "һ����");
            break;
        default:
            unit_type = Unit_Type_NO_TYPE;
            unit_type_index = 0;
            MY_SNPRINTF(str, sizeof(str), "%s", "δѡ��");
            break;
    }

    if (s_i_key_status == KEY_OK_DOWN)  goto key_ok;

    GuiRowTextPlus(16, 2, 127, FONT_LEFT, (uint8_t *)"��������");

    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(33, 2, 127, FONT_MID, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
key_ok:
    old_unit_type = paramsGetGwUnitType();
    if (old_unit_type == unit_type)     goto saved;

    GuiWinDraw(&s_win_saving);
    Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
    if (pt_gw_config != NULL)
    {
        pt_gw_config->uh_unit_type = unit_type;
        paramsReleaseGwConfig();
    }
    else
    {
        LOG_E("[%s] get gw config failed", __func__);
        //@ ��ʾ����ʧ��
    }
    s_gui_saving = 1;
saving:
    if (s_gui_term_addr > paramsGetGwTermNum())
    {
        goto saved;
    }
    paramsSaveTermDefaultFeedParam(s_gui_term_addr);
    s_gui_term_addr++;
    return;
saved:
    s_gui_term_addr = 1;
    s_gui_saving = 0;
    paramsSetTermComEvent(0, event_set_feed_param);
    paramsSetTermComEvent(0, event_set_pig_type);
    paramsSetTermComEvent(0, event_get_pig_type);
    GuiWinDraw(&s_win_save_success);
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    first_enter_flag = 0;
}

/* ��Ԫ�����÷����� */
static void _userUnitNumSetFunc(void *param)
{
    static uint16_t unit_num = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        unit_num = paramsGwUnitNumGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (unit_num < 300)     unit_num++;
            else                    unit_num = 0;
            break;
        case KEY_DOWN_DOWN:
            if (unit_num > 0)       unit_num--;
            else                    unit_num = 300;
            break;
        case KEY_LEFT_DOWN:
            if (unit_num >= 10)     unit_num -= 10;
            else                    unit_num = 300;
            break;
        case KEY_RIGHT_DOWN:
            if (unit_num <= 290)    unit_num += 10;
            else                    unit_num = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            goto saved;
            break;
        default:
            break;
    }
    GuiRowTextPlus(20, 2, 128, FONT_LEFT, (uint8_t *)"��Ԫ��");

    MY_SNPRINTF(str, sizeof(str), "%3d", unit_num);

    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(20, 2, 128, FONT_MID, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    if (unit_num != paramsGwUnitNumGet())
    {
        Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
        if (pt_gw_config != NULL)
        {
            pt_gw_config->uh_unit_num = unit_num;
            paramsReleaseGwConfig();
        }
        else
        {
            LOG_E("[%s] get gw config failed", __func__);
            //@ ��ʾ����ʧ��
        }
    }
    GuiWinDraw(&s_win_save_success);
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��Ԫ��λ���÷����� */
static void _userUnitTermRawSetFunc(void *param)
{
    //�û�Ӧ�ô���:��ͼ��
    static uint8_t line = 0;
    static Gateway_Term_Layout_T gw_layout = {0};

    char str[GUI_ONE_LINE_MAX_SIZE];

    Gateway_Config_Param_T *pt_gw_config = NULL;

    if (s_gui_saving == 1)      goto saving;
    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
        if (pt_gw_config != NULL)
        {
            MY_MEMCPY(&gw_layout, &(pt_gw_config->unit_layout), sizeof(gw_layout));
            paramsReleaseGwConfig();
            first_enter_flag = 1;
        }
        else
        {
            LOG_E("[%s] get gw config failed", __func__);
            return;
        }
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (line > 0)   line--;
            else            line = 3;
            break;
        case KEY_DOWN_DOWN:
            if (line < 3)   line++;
            else            line = 0;
            break;
        case KEY_LEFT_DOWN:
            if (gw_layout.term_layout[line] > 0)        gw_layout.term_layout[line]--;
            else                                        gw_layout.term_layout[line] = 99;
            break;
        case KEY_RIGHT_DOWN:
            if (gw_layout.term_layout[line] < 99)       gw_layout.term_layout[line]++;
            else                                        gw_layout.term_layout[line] = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            goto key_ok;
            break;
        default:
            break;
    }

    GuiRowTextPlus(5, 0, 40, FONT_MID, (uint8_t *)"��1��");
    GuiRowTextPlus(5, 1, 40, FONT_MID, (uint8_t *)"��2��");
    GuiRowTextPlus(5, 2, 40, FONT_MID, (uint8_t *)"��3��");
    GuiRowTextPlus(5, 3, 40, FONT_MID, (uint8_t *)"��4��");
    for (int i = 0; i < 4; i++)
    {
        MY_SNPRINTF(str, sizeof(str), "��%2d��", gw_layout.term_layout[i]);
        if (i == line)
        {
            GuiSetbackcolor(CBLACK);
            GuiRowTextPlus(60, i, 40, FONT_MID, (uint8_t *)str);
            GuiSetbackcolor(CWHITLE);
        }
        else
        {
            GuiRowTextPlus(60, i, 40, FONT_MID, (uint8_t *)str);
        }
    }
    GuiUpdateDisplayAll();
    return;

key_ok:
    pt_gw_config = paramsTakeGwConfig();
    if (pt_gw_config == NULL)
    {
        LOG_E("[%s] get gw config failed", __func__);
        //@ ��ʾʧ����Ϣ
        return;
    }

    GuiWinDraw(&s_win_saving);

    if (MY_MEMCMP(&gw_layout, &(pt_gw_config->unit_layout), sizeof(gw_layout)) != 0)
    {
        uint16_t term_num = 0, term_raw_value = 0;
        MY_MEMCPY(&(pt_gw_config->unit_layout), &gw_layout, sizeof(pt_gw_config->unit_layout));
        for (int i = 0; i < GW_TERM_ROW_MAX; i++)
        {
            if (gw_layout.term_layout[i] > 0)
            {
                term_num += gw_layout.term_layout[i];
                term_raw_value++;
            }
        }
        if (pt_gw_config->uh_term_num < term_num)
        {
            s_gui_term_addr = pt_gw_config->uh_term_num + 1;
            s_gui_saving = 1;
        }
        pt_gw_config->unit_layout.term_raw_value = term_raw_value;
        pt_gw_config->unit_layout.term_init_flag = 1;
        pt_gw_config->uh_term_num = term_num;
    }
    paramsReleaseGwConfig();

    if (s_gui_saving == 0)      goto saved;
saving:
    if (s_gui_term_addr > paramsGetGwTermNum())
    {
        goto saved;
    }
    paramsSaveTermDefaultFeedParam(s_gui_term_addr);
    s_gui_term_addr++;
    return;
saved:
    s_gui_term_addr = 1;
    s_gui_saving = 0;
    paramsSetTermComEvent(0, event_set_feed_param);
    GuiWinDraw(&s_win_save_success);
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��ιƽ̨ip�Ͷ˿����ý�������� */
static void _userServerInfoSetFunc(void *param)
{
    static uint8_t index = 0;   /* ��Ӧserver_info_str���ֽڵ����� */
    static uint8_t server_info_str[16] = {0};   /* ����ip�Ͷ˿ڵ�ÿһλ */

    uint8_t server_ipv4[4];
    uint16_t server_port;

    uint8_t cur_max = 9;    /* Ĭ�Ͽ������� 0~9 */

    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
        if (pt_gw_config == NULL)
        {
            LOG_E("[%s] get gw config failed", __func__);
            goto exit;
        }
        MY_MEMCPY(server_ipv4, pt_gw_config->t_network_param.auc_server_ip, sizeof(server_ipv4));
        server_port = pt_gw_config->t_network_param.uh_server_port;
        paramsReleaseGwConfig();

        server_info_str[0] = server_ipv4[0] / 100;
        server_info_str[1] = (server_ipv4[0] % 100) / 10;
        server_info_str[2] = server_ipv4[0] % 10;
        server_info_str[3] = server_ipv4[1] / 100;
        server_info_str[4] = (server_ipv4[1] % 100) / 10;
        server_info_str[5] = server_ipv4[1] % 10;
        server_info_str[6] = server_ipv4[2] / 100;
        server_info_str[7] = (server_ipv4[2] % 100) / 10;
        server_info_str[8] = server_ipv4[2] % 10;
        server_info_str[9] = server_ipv4[3] / 100;
        server_info_str[10] = (server_ipv4[3] % 100) / 10;
        server_info_str[11] = server_ipv4[3] % 10;

        server_info_str[12] = server_port / 1000;
        server_info_str[13] = (server_port % 1000) / 100;
        server_info_str[14] = (server_port % 100) / 10;
        server_info_str[15] = (server_port % 10) / 1;

        first_enter_flag = 1;
        LOG_D("[%s] first enter", __func__);
    }

    /* ����ip��ַ����ÿλ���ֵ */ //@ ��������޸�Ϊ����255����������Ϊ255
    if (index < 12)
    {
        if ((index%3) == 0)
        {
            cur_max = 2;    /* ip�ֽڵİ�λ��Χλ 0~2 */
        }
        else if (((index%3) == 1)&&(server_info_str[index-1] == 2))
        {
            cur_max = 5;    /* ��ip�ֽڵİ�λΪ2ʱ��ʮλ��ΧΪ 0~5 */
        }
        else if (((index%3) == 2)&&(server_info_str[index-2] == 2)&&(server_info_str[index-1] == 5))
        {
            cur_max = 5;    /* ��ip�ֽڵİ�λΪ2��ʮλΪ5ʱ����λ��ΧΪ 0~5 */
        }
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (server_info_str[index] < cur_max)
            {
                server_info_str[index]++;
            }
            else
            {
                server_info_str[index] = 0;
            }
            break;
        case KEY_DOWN_DOWN:
            if (server_info_str[index] > 0)
            {
                server_info_str[index]--;
            }
            else
            {
                server_info_str[index] = cur_max;
            }
            break;
        case KEY_LEFT_DOWN:
            if (index > 0)
            {
                index--;
            }
            else
            {
                index = 15;
            }
            break;
        case KEY_RIGHT_DOWN:
            if (index < 15)
            {
                index++;
            }
            else
            {
                index = 0;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
        {
            server_ipv4[0] = (server_info_str[0] * 100) + (server_info_str[1] * 10) + server_info_str[2];
            server_ipv4[1] = (server_info_str[3] * 100) + (server_info_str[4] * 10) + server_info_str[5];
            server_ipv4[2] = (server_info_str[6] * 100) + (server_info_str[7] * 10) + server_info_str[8];
            server_ipv4[3] = (server_info_str[9] * 100) + (server_info_str[10] * 10) + server_info_str[11];

            server_port = (server_info_str[12] * 1000) + (server_info_str[13] * 100) + (server_info_str[14] * 10) + server_info_str[15];

            Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
            if (pt_gw_config == NULL)
            {
                LOG_E("[%s] get gw config failed", __func__);
                return;
            }
            if ((MY_MEMCMP(server_ipv4, pt_gw_config->t_network_param.auc_server_ip, sizeof(server_ipv4)) != 0)||\
                (server_port != pt_gw_config->t_network_param.uh_server_port))
            {
                s_gui_saving = 1;
                MY_MEMCPY(pt_gw_config->t_network_param.auc_server_ip, server_ipv4, sizeof(pt_gw_config->t_network_param.auc_server_ip));
                pt_gw_config->t_network_param.uh_server_port = server_port;
            }
            paramsReleaseGwConfig();
            goto saved;
            break;
        }
        default:
            break;
    }

    MY_MEMSET(str, ' ', sizeof(str));
    if (index == 15)
    {
        str[index+4] = '*';
    }
    else
    {
        str[index + index/3] = '*';
    }
    GuiRowTextPlus(5, 1, 120, FONT_LEFT, (uint8_t *)str);

    MY_SNPRINTF(str, sizeof(str), "%d%d%d.%d%d%d.%d%d%d.%d%d%d:%d%d%d%d",
                server_info_str[0], server_info_str[1], server_info_str[2],
                server_info_str[3], server_info_str[4], server_info_str[5],
                server_info_str[6], server_info_str[7], server_info_str[8],
                server_info_str[9], server_info_str[10], server_info_str[11],
                server_info_str[12], server_info_str[13], server_info_str[14], server_info_str[15]);
    GuiRowTextPlus(5, 2, 120, FONT_LEFT, (uint8_t *)str);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_save_success);
    if (s_gui_saving == 1)
    {
        s_gui_saving = 0;
        /* ��������ɹ�����Ҫ���� */
        if (_userMqttRestart() != RT_EOK)
        {
            rt_hw_cpu_reset();
        }
    }
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��ιƽ̨ѡ���������� */
static void _userMqttSelectSetFunc(void *param)
{
    static uint8_t mqtt_flag = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        first_enter_flag = 1;
        mqtt_flag = paramsGetGwMqttSelected();
        LOG_D("[%s] first enter", __func__);
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            mqtt_flag = !mqtt_flag;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (mqtt_flag != paramsGetGwMqttSelected())
            {
                GuiWinDraw(&s_win_saving);
                paramsSetGwMqttSelected(mqtt_flag);
                s_gui_saving = 1;
            }
            goto saved;
            break;
        default:
            break;
    }

    GuiRowTextPlus(5, 2, 50, FONT_RIGHT, (uint8_t *)"ƽ̨��");
    GuiSetbackcolor(CBLACK);
    if (mqtt_flag != 0)
    {
        GuiRowTextPlus(56, 2, 50, FONT_LEFT, (uint8_t *)"�ܲ�");
    }
    else
    {
        GuiRowTextPlus(56, 2, 50, FONT_LEFT, (uint8_t *)"����");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_save_success);
    if (s_gui_saving == 1)
    {
        s_gui_saving = 0;
        /* ��������ɹ�����Ҫ���� */
        if (_userMqttRestart() != RT_EOK)
        {
            rt_hw_cpu_reset();
        }
    }
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* �û�����ѡ���������� */
static void _userUpgradeModeSetFunc(void *param)
{
    static uint8_t upgrade_mode = 0;    /* 0-�㲥; 1-����(��Ե�) */

    int32_t ret = RT_ERROR;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        upgrade_mode = 0;
        first_enter_flag = 1;
        LOG_D("[%s] first enter", __func__);
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            if (paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_LORA)
            {
                upgrade_mode = !upgrade_mode;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            // ret = ����ʹ�����õķ�ʽ������������0-��ʾ���óɹ���1-��ʾ����ʧ��
            LOG_I("gui set upgrade mode:%d", upgrade_mode);
            if (ret == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }
            goto saved;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"������ʽ��");
    GuiSetbackcolor(CBLACK);
    if (upgrade_mode != 0)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"����");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"�㲥");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif

/* ��Ԫͨ���������ý�������� */
static void _userUnitCommTypeSetFunc(void *param)
{
    static uint8_t unit_comm_type = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        unit_comm_type = paramsGetGwUnitCommType();
        first_enter_flag = 1;
        LOG_D("[%s] first enter", __func__);
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            unit_comm_type = !unit_comm_type;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (unit_comm_type != paramsGetGwUnitCommType())
            {
                Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
                if (pt_gw_config == NULL)
                {
                    LOG_E("[%s] get gw config failed", __func__);
                    return;
                }
                pt_gw_config->uc_unit_com_type = unit_comm_type;
                paramsReleaseGwConfig();
                s_gui_saving = 1;
            }
            goto saved;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"ͨѶ��ʽ��");
    GuiSetbackcolor(CBLACK);
    if (unit_comm_type != GW_TERM_COM_TYPE_RS485)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"����");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"����");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_set_complete);
    if (s_gui_saving == 1)
    {
       //@ ������Ҫ�л����ն�ͨ�ŷ�ʽ��������Ӧ�¼�,ȥ�����������
       s_gui_saving = 0;
    }
    basicHalReboot(10);   //@ ����������Ҫȥ��
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��ι�����·���ʽ���ý�������� */
static void _userUnitFeedVersionSetFunc(void *param)
{
    static uint8_t unit_fee_version = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        unit_fee_version = paramsGetGwUnitFeedVersion();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            unit_fee_version = !unit_fee_version;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (unit_fee_version != paramsGetGwUnitFeedVersion())
            {
                Gateway_Config_Param_T *pt_gw_config = paramsTakeGwConfig();
                if (pt_gw_config == NULL)
                {
                    LOG_E("[%s] get gw config failed", __func__);
                    return;
                }
                pt_gw_config->uc_unit_feed_version = unit_fee_version;
                paramsReleaseGwConfig();
                s_gui_saving = 1;
                paramsSetTermComEvent(0, event_set_feed_param);
            }
            goto saved;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"��ιģʽ��");
    GuiSetbackcolor(CBLACK);
    if (unit_fee_version == 1)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"����");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"��̨");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_set_complete);
    if (s_gui_saving == 1)
    {
       s_gui_saving = 0;
    }
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��Ԫ���Ϸ�ʽ���ý��������*/
static void _userUnitFoodModeSetFunc(void *param)
{
    static uint8_t food_mode = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag==0)
    {
        first_enter_flag = 1;
        food_mode = paramsGetGwFoodMode();
    }

    switch (s_i_key_status)
    {
        case KEY_RIGHT_DOWN:
        case KEY_DOWN_DOWN:
            if (food_mode < 2)
            {
                food_mode++;
            }
            else
            {
                food_mode = 0;
            }
            break;
        case KEY_LEFT_DOWN:
        case KEY_UP_DOWN:
            if (food_mode > 0)
            {
                food_mode--;
            }
            else
            {
                food_mode = 2;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (food_mode != paramsGetGwFoodMode())
            {
                paramsSetGwFoodMode(food_mode);
                s_gui_saving = 1;
            }
            goto saved;
            break;
        default:
            break;
    }

    GuiRowTextPlus(2, 2, 126, FONT_LEFT, (uint8_t *)"���Ϸ�ʽ:");
    GuiSetbackcolor(CBLACK);
    if (food_mode == 1)
    {
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"����ˮ������");
    }
    else if (food_mode == 2)
    {
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"�����Ϻ���ˮ");
    }
    else
    {
        food_mode = 0;
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"ͬʱ������ˮ");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_set_complete);
    if (s_gui_saving == 1)
    {
        if(paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_RS485)
        {
            paramsSetTermComEvent(0, event_gateway_set_param);
        }
        else
        {
            paramsSetTermComEvent(0, event_set_food_mode);
        }
        s_gui_saving = 0;
    }
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* lora�ź�ǿ�Ȳ��Խ�������� */
static void _userLorsTestFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    int16_t Rssi_Gateway = -255;
    int16_t Rssi_term    = -255;

    Term_List_T *pt_term_list = NULL;

    if (first_enter_flag == 0)
    {
        pt_term_list = paramsTakeTermList(1);
        if (pt_term_list == NULL)
        {
            LOG_E("[%s] get term list failed", __func__);
            goto exit;
        }
        pt_term_list->Rssi_Gateway = -255;
        pt_term_list->Rssi_term    = -255;
        paramsReleaseTermList();
        midLoraInNet();    //�л���ע���ŵ����Լ�ע������ID����������ע�ἴ�ɲ����ź�ǿ��
        paramsSetTermCommMode(E_TERM_COMM_MODE_TEST);   /* loraͨ��Э���л����źŲ���ģʽ,���л��ŵ�������ģʽ */
        LOG_I("[lora] LoRa in test mode!");
        first_enter_flag = 1;
    }
    else
    {
        pt_term_list = paramsTakeTermList(1);
        if (pt_term_list == NULL)
        {
            LOG_E("[%s] get term list failed", __func__);
            return;
        }
        Rssi_Gateway = pt_term_list->Rssi_Gateway;
        Rssi_term = pt_term_list->Rssi_term;
        paramsReleaseTermList();
    }

    switch (s_i_key_status)
    {
        case KEY_CANCEL_DOWN:
        case KEY_OK_DOWN:
            paramsResetTermCommMode(E_TERM_COMM_MODE_TEST);     /* ��λ����ģʽ��־ */
            midLoraBackToCom(); //�л������������ŵ����Լ��ظ�����ID
            LOG_I("[lora] LoRa out test mode!");
            goto exit;
            break;
        default:
            break;
    }

    MY_SNPRINTF(str, sizeof(str), "[01]G��%04d,T��%04d", Rssi_Gateway, Rssi_term);
    GuiRowTextPlus(2, 2, 120, FONT_LEFT, (uint8_t *)str);
    GuiUpdateDisplayAll();
    return;
exit:
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ��ι�������ò��·���������� */
static void _userFeedParamSetDefault(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (s_gui_saving == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "�װ�ȷ�Ͽ�ʼ��");
        GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)str);
        GuiUpdateDisplayAll();
    }
    else
    {
        goto saving;
    }

    switch (s_i_key_status)
    {
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            s_gui_saving = 1;
            s_gui_term_addr = 1;
            break;
        default:
            break;
    }
    return;
saving:
    if (s_gui_term_addr > paramsGetGwTermNum())
    {
        goto saved;
    }
    paramsSaveTermDefaultFeedParam(s_gui_term_addr);
    s_gui_term_addr++;
    return;
saved:
    s_gui_saving = 0;
    s_gui_term_addr = 1;
    GuiWinDraw(&s_win_set_complete);
    paramsSetTermComEvent(0, event_set_feed_param);
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif
/* ������ι��¼��ս��� */
static void _userTermRecodeClear(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        char str[GUI_ONE_LINE_MAX_SIZE];
        Term_Recode_Cursor_T t_term_recode_cursor;
        Term_Recode_T *pt_term_recode = paramsTakeTermRecode(0);
        if (pt_term_recode == NULL)
        {
            LOG_E("[%s] get term recode failed", __func__);
            goto exit;
        }
        MY_MEMCPY(&t_term_recode_cursor, &(pt_term_recode->recode_cursor), sizeof(t_term_recode_cursor));
        paramsReleaseTermRecode(0);

        MY_SNPRINTF(str, sizeof(str), "n:%06d", t_term_recode_cursor.report_num);
        GuiRowTextPlus(2, 0, 120, FONT_LEFT, (uint8_t *)str);
        MY_SNPRINTF(str, sizeof(str), "r:%06d", t_term_recode_cursor.read_cursor);
        GuiRowTextPlus(2, 1, 120, FONT_LEFT, (uint8_t *)str);
        MY_SNPRINTF(str, sizeof(str), "w:%06d", t_term_recode_cursor.write_cursor);
        GuiRowTextPlus(2, 2, 120, FONT_LEFT, (uint8_t *)str);
        GuiUpdateDisplayAll();

        first_enter_flag = 1;
    }
    else if (s_gui_saving == 1)
    {
        MY_SNPRINTF(str, sizeof(str), "�Ƿ���ձ�����ι��¼��");

        GuiRowTextPlus(2, 3, 120, FONT_LEFT, (uint8_t *)str);
        GuiUpdateDisplayAll();
    }

    switch (s_i_key_status)
    {
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (s_gui_saving == 0)
            {
                s_gui_saving = 1;
            }
            else
            {
                MY_SNPRINTF(str, sizeof(str), "��ձ�����ι��¼��...");
                GuiRowTextPlus(2, 3, 120, FONT_LEFT, (uint8_t *)str);
                GuiUpdateDisplayAll();

                if (paramsDelTermRecode() == RT_EOK)
                {
                    GuiWinDraw(&s_win_set_complete);
                }
                else
                {
                    GuiWinDraw(&s_win_set_failed);
                }
                goto saved;
            }
            break;
        default:
            break;
    }
    return;
saved:
    s_gui_saving = 0;
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_saving = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* �����ŵ����ý�������� */
static void _userLoraChannelSetFunc(void *param)
{
    uint32_t cur_tick = MY_TICK_GET();
    char str[GUI_ONE_LINE_MAX_SIZE];
    uint8_t ch_adjust_run_flag = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (s_gui_saving != 0)
    {
        goto set_channel;
    }

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraGetChannel();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, LORA_CH_MIN, LORA_CH_MAX, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, LORA_CH_MIN, LORA_CH_MAX, 1);
            break;
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, LORA_CH_MIN, LORA_CH_MAX, 10);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, LORA_CH_MIN, LORA_CH_MAX, 10);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (s_gui_value == paramsLoraGetChannel())
            {
                s_gui_wait_flag = 1;
                s_gui_wait_last_tick = MY_TICK_GET();
                GuiWinDraw(&s_win_set_complete);
                return;
            }

            /* �����������ŵ������׶� */
            ch_adjust_run_flag = paramsLoraChAdjustRunFlagGet();
            paramsLoraChAdjustResetFlagSet(1);
            if (ch_adjust_run_flag != 0)
            {
                midLoraBackToCom();
            }

            appTermEventLoraParamSet(s_gui_value, paramsLoraGetNetId());
            s_gui_saving = 1;   /* ��ʾ���������ŵ� */
            s_gui_value_0 = 0;  /* ����, �����ŵ���ʱ���� */
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"�����ŵ���");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d", s_gui_value);
    GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
set_channel:
    if ((uint32_t)(cur_tick - s_gui_wait_last_tick) < 2000)
    {
        return;
    }
    s_gui_wait_last_tick = cur_tick;
    s_gui_value_0++;
    extern unsigned char _midCheckLoraIsIdle(void);
    if ((s_gui_value_0 >= 30)||(_midCheckLoraIsIdle() <= 5))
    {
        LOG_D("wait cnt: %d", s_gui_value_0);
        s_gui_saving = 0;
        s_gui_value_0 = 0;
        s_gui_wait_flag = 1;
        s_gui_wait_last_tick = MY_TICK_GET();
        paramsLoraSetChannel(s_gui_value, 1);
        midLoraBackToCom();
        GuiWinDraw(&s_win_set_complete);
        return;
    }

    if (_midCheckLoraIsIdle() == 30)
    {
        LOG_W("resend set channel");
        appTermEventLoraParamSet(s_gui_value, paramsLoraGetNetId());
    }
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ���߷��͹������ý�������� */
static void _userLoraTxpowerSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraGetTxPower();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 20)
            {
                s_gui_value++;
            }
            else
            {
                s_gui_value = 1;
            }
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 1)
            {
                s_gui_value--;
            }
            else
            {
                s_gui_value = 20;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (s_gui_value == paramsLoraGetTxPower())
            {
                goto exit;
            }
            GuiWinDraw(&s_win_saving);
            paramsLoraSetTxPower(s_gui_value, 1);
            midLoraBackToCom();
            GuiWinDraw(&s_win_save_success);
            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"���߷��书�ʣ�");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%02d", s_gui_value);
    GuiRowTextPlus(100, 2, 28, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ������Ƶ�������ý�������� */
static void _userLoraSfSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraGetSf();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 12)
            {
                s_gui_value++;
            }
            else
            {
                s_gui_value = 7;
            }
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 7)
            {
                s_gui_value--;
            }
            else
            {
                s_gui_value = 12;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (s_gui_value == paramsLoraGetSf())
            {
                goto exit;
            }
            GuiWinDraw(&s_win_saving);
            paramsLoraSetSf(s_gui_value, 1);
            midLoraBackToCom();
            GuiWinDraw(&s_win_save_success);
            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"������Ƶ���ӣ�");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%02d", s_gui_value);
    GuiRowTextPlus(100, 2, 28, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ���߱��������ý�������� */
static void _userLoraCrSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraGetCr();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 4)
            {
                s_gui_value++;
            }
            else
            {
                s_gui_value = 1;
            }
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 1)
            {
                s_gui_value--;
            }
            else
            {
                s_gui_value = 4;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (s_gui_value == paramsLoraGetCr())
            {
                goto exit;
            }
            GuiWinDraw(&s_win_saving);
            paramsLoraSetCr(s_gui_value, 1);
            midLoraBackToCom();
            GuiWinDraw(&s_win_save_success);
            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"���߱����ʣ�");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%01d", s_gui_value);
    GuiRowTextPlus(100, 2, 28, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ���ߴ������ý�������� */
static void _userLoraBwSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraGetBw();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 2)
            {
                s_gui_value++;
            }
            else
            {
                s_gui_value = 0;
            }
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)
            {
                s_gui_value--;
            }
            else
            {
                s_gui_value = 2;
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            if (s_gui_value == paramsLoraGetBw())
            {
                goto exit;
            }
            GuiWinDraw(&s_win_saving);
            paramsLoraSetBw(s_gui_value, 1);
            midLoraBackToCom();
            GuiWinDraw(&s_win_save_success);
            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"���ߴ���");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%01d", s_gui_value);
    GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/** �����ŵ��Զ������������ý�������� */
static void _userLoraChAdjustSetFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsLoraChAdjustSwitchGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            s_gui_value = !s_gui_value;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_saving);
            paramsLoraChAdjustSwitchSet(s_gui_value);
            GuiWinDraw(&s_win_save_success);
            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"�Զ��������أ�");
    GuiSetbackcolor(CBLACK);
    if(s_gui_value == 0x01)
    {
        GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)" �� ");
    }
    else
    {
        GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)" �� ");
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/** �����ŵ��Զ�������ʼ��������� */
static void _userLoraChAdjustStartFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];
    uint32_t cur_tick = MY_TICK_GET();
    uint32_t delay_s = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_wait_last_tick = cur_tick;
        paramsLoraChAdjustForceFlagSet(1);
        first_enter_flag = 1;
    }

    delay_s = (uint32_t)(cur_tick - s_gui_wait_last_tick);
    delay_s /= 1000;

    /* ��ʱ10���ӻ����ŵ���������������˳� */
    if ((delay_s > 600)||
        ((delay_s > 3)&&(paramsLoraChAdjustRunFlagGet() == 0)))
    {
        GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"�ŵ��������     ");
        MY_SNPRINTF(str, sizeof(str), "�������ŵ�: %3d  ", paramsLoraGetChannel());
        GuiRowTextPlus(0, 3, 128, FONT_MID, (uint8_t *)str);
        GuiUpdateDisplayAll();
        s_gui_wait_flag = 1;
        s_gui_wait_last_tick = cur_tick;
        return;
    }

    if (delay_s <= 300)         delay_s = 300 -delay_s;
    else                        delay_s = 0;

    GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"���ڽ����ŵ�����,");
    rt_snprintf(str, sizeof(str), "�����ĵȴ� %3d ��", delay_s);
    GuiRowTextPlus(0, 3, 128, FONT_MID, (uint8_t *)str);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif
/* Loraͨ�Ų�����ʼ����������� */
static void _userLoraSetDefaultFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    GuiWinDraw(&s_win_lora_initing);
    paramsLoraSetDefault();
    midLoraBackToCom();

    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    if (s_gui_saving == 0)
    {
        s_gui_saving = 1;
        s_gui_wait_flag = 1;
        s_gui_wait_last_tick = MY_TICK_GET();
        GuiWinDraw(&s_win_save_success);
        return;
    }
    s_gui_wait_flag = 0;
    s_gui_saving = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ���ö�ˮģʽ��������� */
static void _userConfigNoWaterFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigNoWaterModeGet();   // 0-�򿪣�1-�ر�
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            s_gui_value = !s_gui_value;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_saving);
            paramsTermConfigNoWaterModeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_set_pig_type);
            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"��ˮ����ģʽ��");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value == 1)
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"��");
    }
    else
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"�ر�");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif

/**
  *@brief  һ�巧�������ý���
  *@param  None
  *@retval None
  */
static void _userValveSpeedSetFunc(void *param)
{
    //�û�Ӧ�ô���:��ͼ��
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterSpeedGet();
        first_enter_flag = 1;
    }

    /* ����״̬���� */
    switch (s_i_key_status)
    {
    case KEY_UP_DOWN:
        s_gui_value = _userGuiValueIncStep(s_gui_value, 60, 500, 1);
        break;
    case KEY_DOWN_DOWN:
        s_gui_value = _userGuiValueDecStep(s_gui_value, 60, 500, 1);
        break;
    case KEY_LEFT_DOWN:
        s_gui_value = _userGuiValueDecStep(s_gui_value, 60, 500, 10);
        break;
    case KEY_RIGHT_DOWN:
        s_gui_value = _userGuiValueIncStep(s_gui_value, 60, 500, 10);
        break;
    case KEY_CANCEL_DOWN:
        goto exit;
        break;
    case KEY_OK_DOWN:
        GuiWinDraw(&s_win_set_ing);
        paramsTermConfigWaterSpeedSet(s_gui_value);
        GuiWinDraw(&s_win_set_complete);

        paramsSetTermComEvent(0, event_gateway_set_param);

        s_gui_wait_flag = 1;
        s_gui_wait_last_tick = MY_TICK_GET();
        return;
    default:
        break;
    }
    GuiRowTextPlus(7, 2, 114, FONT_LEFT, (uint8_t *)"һ�巧���ʣ�");

    MY_MEMSET(str, 0, sizeof(str));

    MY_SNPRINTF((char *)str, sizeof(str), "%3d g/s", s_gui_value);
#if 0
    if (s_gui_value == 1)
    {
        MY_SNPRINTF((char *)str, sizeof(str), "%sL/min", "3.3");
    }
    else if (s_gui_value == 2)
    {
        MY_SNPRINTF((char *)str, sizeof(str), "%sL/min", " 10");
    }
    else if (s_gui_value == 3)
    {
        MY_SNPRINTF((char *)str, sizeof(str), "%sL/min", " 15");
    }
    else if (s_gui_value == 4)
    {
        MY_SNPRINTF((char *)str, sizeof(str), "%sL/min", " 20");
    }
    else
    {
        MY_SNPRINTF((char *)str, sizeof(str), "NULL");
    }
#endif
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(4, 2, 120, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ���ٲ������ý�������� */
static void _userWaterSpeedDiffSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterSpeedDiffGet();
        first_enter_flag = 1;
    }

    /* ����״̬���� */
    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 20, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 20, 1);
            break;
        case KEY_LEFT_DOWN:
            break;
        case KEY_RIGHT_DOWN:
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigWaterSpeedDiffSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }
    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"���첹����");
    if (s_gui_value > 10)
    {
        MY_SNPRINTF(str, sizeof(str), "-%2dL", s_gui_value-10);
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), " %2dL", s_gui_value);
    }
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(22, 2, 84, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��������ģʽʱ�����ý�������� */
static void _userConfigSellTimeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsSellDurationTimeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 25)       s_gui_value++;
            else                        s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)        s_gui_value--;
            else                        s_gui_value = 25;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (s_gui_value != paramsSellDurationTimeGet())   // 0-�򿪣�1-�ر�
            {
                paramsSellDurationTimeSet(s_gui_value);
                for (uint16_t i = 1; i <= paramsGetGwTermNum(); i++)
                {
                    if (paramsSellParaGet(i)->sell_mode_switch > 0)
                    {
                        appSellModeEndFore(i);
                        paramsSetTermComEvent(i,event_set_feed_param);
                    }

                }
            }
            GuiWinDraw(&s_win_set_complete);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 73, FONT_RIGHT, (uint8_t *)"����ʱ����");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value > 24)
    {
        MY_SNPRINTF(str, sizeof(str), "30 ����");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "%2d Сʱ", s_gui_value);
    }
    GuiRowTextPlus(73, 2, 55, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��λ����ģʽ�������ý�������� */
static void _userConfigSellSwitchSetFunc(void *param)
{
    /* s_gui_value ��������Ϊѡ�е���λ�α� */
    static uint32_t s_gui_sell_status[(GW_TERM_NUM_MAX+31)>>5];
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];

    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page  = (term_num+2)/3;  /* ������һҳ�����һҳ */

    uint8_t cur_page_start = cur_page*3;
    uint8_t cur_line = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        MY_MEMSET(s_gui_sell_status, 0, sizeof(s_gui_sell_status));
        for (int i = 0; i < term_num; i++)
        {
            // if (sell_mode[i].sell_mode_switch == 1) //�ж�����ģʽ�Ƿ��
            if (paramsSellModeSwitchStatusGet(i+1) == 1) //Ӧ�������ģ�  spy 0509
            {
                MY_BIT_SET(s_gui_sell_status[i>>5], (i&0x1F));
            }
        }
        s_gui_value = 0;
        cur_page = 0;
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value > 0)
            {
                s_gui_value--;
            }
            else
            {

                if(cur_page > 0)
                {
                    cur_page--;
                    s_gui_value = 2;
                }
                else
                {
                    cur_page = max_page-1;
                    s_gui_value = term_num - (cur_page*3) - 1;
                }
            }
            break;
        case KEY_DOWN_DOWN:
            s_gui_value++;
            if ((s_gui_value > 2)||((cur_page_start+s_gui_value) >= term_num))
            {
                s_gui_value = 0;
                cur_page++;
                if(cur_page >= max_page)
                {
                    cur_page = 0;
                }
            }
            break;
        case KEY_LEFT_DOWN:
        case KEY_RIGHT_DOWN:
            if (MY_BIT_CHECK(s_gui_sell_status[(cur_page_start+s_gui_value)>>5], ((cur_page_start+s_gui_value)&0x1F)) != 0)
            {
                MY_BIT_RESET(s_gui_sell_status[(cur_page_start+s_gui_value)>>5], ((cur_page_start+s_gui_value)&0x1F));
            }
            else
            {
                MY_BIT_SET(s_gui_sell_status[(cur_page_start+s_gui_value)>>5], ((cur_page_start+s_gui_value)&0x1F));
            }
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_saving);
#if 0//�������õ�ֵ
            for(int i = 1;i <= paramsGetGwTermNum();i++)
            {
                if(paramsSellModeSwitchStatusGet(i) != pig_sell_status[i])    // ������ô�޸ģ�  spy 0509
                {
                    if(pig_sell_status[i]==0)
                    {
                        appSellModeEndSet(i);                             
                    }

                    /* sell_mode[i].sell_mode_switch = pig_sell_status[i]; */
                    paramsSellParaTake(i)->sell_mode_switch = pig_sell_status[i];
                    paramsSellParaRelease();


                    appSellModeEndFore(i);
                    paramsSetTermComEvent(i,event_set_feed_param);
                }
            }
#endif
            GuiWinDraw(&s_win_save_success);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }
    while((cur_line++) < 3)
    {
        if (cur_page_start < term_num)
        {
            MY_SNPRINTF(str, sizeof(str), "[%3d]:", cur_page_start+1);
            GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)str);

            if (MY_BIT_CHECK(s_gui_sell_status[cur_page_start>>5], (cur_page_start&0x1F)) != 0)
            {
                MY_SNPRINTF(str, sizeof(str), "��");
            }
            else
            {
                MY_SNPRINTF(str, sizeof(str), "�ر�");
            }
            if (s_gui_value == (cur_line-1))
            {
                GuiSetbackcolor(CBLACK);
                GuiRowTextPlus(40, cur_line, 88, FONT_LEFT, (uint8_t *)str);
                GuiSetbackcolor(CWHITLE);
            }
            else
            {
                GuiRowTextPlus(40, cur_line, 88, FONT_LEFT, (uint8_t *)str);
            }
        }
        else
        {
            GuiRowTextPlus(2, cur_line, 120, FONT_LEFT, (uint8_t *)"                   ");
        }
        cur_page_start++;
    }
    cur_page_start = cur_page * 3;
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif

/* ̽����ֵ���ý�������� */
static void _userConfigProbeAdcSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigProbeAdcValueGet();
        if (s_gui_value == 0)
        {
            if (paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_RS485)
            {
                s_gui_value = 3;
            }
            else
            {
                s_gui_value = 36;
            }
        }
        first_enter_flag = 1;
    }
    switch (s_i_key_status)
    {
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 40, 1);
            break;
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 40, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (s_gui_value == 0)
            {
                s_gui_value = 3;
            }
            paramsTermConfigProbeAdcValueSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);
            if (paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_RS485)
            {
                paramsSetTermComEvent(0, event_gateway_set_param);
            }
            else
            {
                Term_List_T *pt_term_list = paramsTakeTermList(TERM_COM_START_ADDR);
                if (pt_term_list != NULL)
                {
                    for (uint16_t i = 0; i < paramsGetGwTermNum(); i++)
                    {
                        (pt_term_list+i)->set_probe_signal_adc = s_gui_value*100;
                        paramsSetTermConfigParamEvent(TERM_COM_START_ADDR+i, set_event_probe_adc);
                        paramsSetTermComEvent(TERM_COM_START_ADDR+i, event_set_term_config_param);
                    }
                    paramsReleaseTermList();
                }
                else
                {
                    LOG_E("[%s] take term(%d) failed", __func__, TERM_COM_START_ADDR);
                }
            }

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }
    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"̽����ֵ��");
    MY_SNPRINTF(str, sizeof(str), "%4d", s_gui_value*100);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(22, 2, 84, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬��ۿ������ý�������� */
static void _userFoodClearSwitchSetFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigDtcSwitchGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            s_gui_value = !s_gui_value;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcSwitchSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"��̬��ۿ��أ�");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value == 1)
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"��");
    }
    else
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"�ر�");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬��ۼ��ʱ�����ý�������� */
static void _userFoodClearCheckTimeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigDtcCheckTimeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 120)      s_gui_value++;
            else                        s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)        s_gui_value--;
            else                        s_gui_value = 120;
            break;
        case KEY_LEFT_DOWN:
            if (s_gui_value < 5)        s_gui_value = 120;
            else                        s_gui_value -= 5;
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value += 5;
            if (s_gui_value > 120)      s_gui_value = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcCheckTimeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"���ʱ�䣺");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d ����", s_gui_value);
    GuiRowTextPlus(70, 2, 58, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬���ʱ�����ý�������� */
static void _userFoodClearFoodTimeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigDtcFoodTimeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 120)      s_gui_value++;
            else                        s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)        s_gui_value--;
            else                        s_gui_value = 120;
            break;
        case KEY_LEFT_DOWN:
            if (s_gui_value < 5)        s_gui_value = 120;
            else                        s_gui_value -= 5;
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value += 5;
            if (s_gui_value > 120)      s_gui_value = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcFoodTimeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"���ʱ�䣺");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d ����", s_gui_value);
    GuiRowTextPlus(70, 2, 58, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬��ˮʱ�����ý�������� */
static void _userFoodClearWaterTimeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigDtcWaterTimeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 120)      s_gui_value++;
            else                        s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)        s_gui_value--;
            else                        s_gui_value = 120;
            break;
        case KEY_LEFT_DOWN:
            if (s_gui_value < 5)        s_gui_value = 120;
            else                        s_gui_value -= 5;
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value += 5;
            if (s_gui_value > 120)      s_gui_value = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcWaterTimeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"��ˮʱ�䣺");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d ����", s_gui_value);
    GuiRowTextPlus(70, 2, 58, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬�����ʼ�������ý�������� */
static void _userFoodClearStartAgeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigDtcStartAgeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value < 200)      s_gui_value++;
            else                        s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value > 0)        s_gui_value--;
            else                        s_gui_value = 200;
            break;
        case KEY_LEFT_DOWN:
            if (s_gui_value < 5)        s_gui_value = 200;
            else                        s_gui_value -= 5;
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value += 5;
            if (s_gui_value > 200)      s_gui_value = 0;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcStartAgeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"��ʼ���䣺");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d ����", s_gui_value);
    GuiRowTextPlus(70, 2, 58, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��̬��۽����������ý�������� */
static void _userFoodClearStopAgeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];
    uint16_t stop_age = paramsTermConfigDtcStopAgeGet();
    uint16_t start_age = paramsTermConfigDtcStartAgeGet();

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = stop_age;
        if (s_gui_value < (start_age+1))
        {
            s_gui_value = start_age+1;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, start_age+1, 300, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, start_age+1, 300, 1);
            break;
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, start_age+1, 300, 5);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, start_age+1, 300, 5);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigDtcStopAgeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"�������䣺");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d ����", s_gui_value);
    GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ����ʱ�����ý�������� */
static void _userProtectTimeSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigProtectTimeUnitGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            s_gui_value = !s_gui_value;
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigProtectTimeUnitSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_set_feed_param);
            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
        default:
            break;
    }

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"����ʱ�䵥λ��");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value == 1)
    {
        MY_SNPRINTF(str, sizeof(str), " 60s");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), " 10s");
    }
    GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
#endif
/* ��ˮģʽ���ý�������� */
static void _userWaterModeSetFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterModeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 2, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 2, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigWaterModeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"��ˮģʽ��");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case 0:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"һ�巧ʽ");
            break;
        case 1:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"Сˮ��ʽ");
            break;
        default:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"���߹���");
            break;
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* ��˸�̽�����������ý�������� */
static void _userProbeSenSetFunc(void *param)
{
    Term_List_T *pt_term_list = NULL;
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigProbeSenGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 1, 4, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 1, 4, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigProbeSenSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            pt_term_list = paramsTakeTermList(TERM_COM_START_ADDR);
            if (pt_term_list != NULL)
            {
                for (uint16_t i = 0; i < GW_TERM_NUM_MAX; i++)
                {
                    (pt_term_list+i)->set_prob_sen = s_gui_value;
                }
                paramsReleaseTermList();
            }
            else
            {
                LOG_E("[%s] take term(%d) list failed", __func__, TERM_COM_START_ADDR);
            }
            paramsSetTermConfigParamEvent(0, set_even_probe_sen);
            paramsSetTermComEvent(0, event_set_term_config_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(19, 2, 90, FONT_LEFT, (uint8_t *)"̽�������ȣ�");
    MY_SNPRINTF(str, sizeof(str), " %2d", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(19, 2, 90, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/**
  *@brief  ǿ�����ϵȼ�����ʵ�庯��
  *@param  None
  *@retval None
  */
static void _userStrongFoodGearSetFunc(void *param)
{
    //�û�Ӧ�ô���:��ͼ��

    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigStrongFoodGearGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value++;
            if (s_gui_value > 4 || s_gui_value < 0)
                s_gui_value = 0;
            break;

        case KEY_DOWN_DOWN:
            s_gui_value--;
            if (s_gui_value < 0 || s_gui_value > 4)
                s_gui_value = 4;
            break;

        case KEY_LEFT_DOWN:
            break;
        case KEY_RIGHT_DOWN:
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            /* �洢 */
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigStrongFoodGearSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);
            paramsSetTermComEvent(0, event_set_food_mode);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
    }

    MY_MEMSET(str, 0, sizeof(str));

    switch(s_gui_value)
    {
        case 0:
            MY_SNPRINTF(str, sizeof(str), "ǿ�����Ϲر�");
            break;
        case 1:
            MY_SNPRINTF(str, sizeof(str), "ǿ������100g");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "ǿ������200g");
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "ǿ������300g");
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "ǿ������400g");
            break;
        default:
            break;
    }
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(28, 2, 72, FONT_MID, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/**
  *@brief  ��ǿ��ˮ�ȼ�����ʵ�庯��
  *@param  None
  *@retval None
  */
static void _userStrongWaterGearSetFunc(void *param)
{
    //�û�Ӧ�ô���:��ͼ��
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigStrongWaterGearGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value++;
            if (s_gui_value > 4 || s_gui_value < 0)
                s_gui_value = 0;
            break;

        case KEY_DOWN_DOWN:
            s_gui_value--;
            if (s_gui_value < 0 || s_gui_value > 4)
                s_gui_value = 4;
            break;

        case KEY_LEFT_DOWN:
            break;
        case KEY_RIGHT_DOWN:
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigStrongWaterGearSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);
            paramsSetTermComEvent(0, event_set_food_mode);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
            break;
    }

    MY_MEMSET(str, 0, sizeof(str));

    switch(s_gui_value)
    {
        case 0:
            MY_SNPRINTF(str, sizeof(str), "ǿ����ˮ�ر� ");
            break;
        case 1:
            MY_SNPRINTF(str, sizeof(str), "ǿ����ˮ0.5��");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "ǿ����ˮ 1 ��");
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "ǿ����ˮ1.5��");
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "ǿ����ˮ 2 ��");
            break;
        default:
            break;
    }
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(25, 2, 78, FONT_MID, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/**
  *@brief  ����У׼ģʽ���ý���
  *@param  None
  *@retval None
  */
static void _userProtectPulseCalFunc(void *param)
{
    //�û�Ӧ�ô���:��ͼ��
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterPulseModeGet();
        first_enter_flag = 1;
    }

    /* ����״̬���� */
    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            if (s_gui_value == 0)
                s_gui_value++;
            else
                s_gui_value = 0;
            break;
        case KEY_DOWN_DOWN:
            if (s_gui_value == 1)
                s_gui_value--;
            else
                s_gui_value = 1;
            break;
        case KEY_LEFT_DOWN:
            break;
        case KEY_RIGHT_DOWN:
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigWaterPulseModeSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    MY_MEMSET(str, 0, sizeof(str));

    if (s_gui_value == 1)
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "�ر�");
    }

    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"�Զ�Уˮ��");
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(22, 2, 84, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/** �������ʱ������ */
static void _userMrFeedCleanSetFunc(void *param)
{
    Mr_Config_T *cur_mr_config = NULL;
    Mr_Config_T *pt_mr_config = (Mr_Config_T *)s_gui_value;
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value_0 = 0;  /* chocie */
        s_gui_value_1 = 0;  /* clean */
        s_gui_value_2 = 0;  /* meal */
        s_gui_value_3 = 1;  /* same */
        pt_mr_config = MY_MALLOC(sizeof(Mr_Config_T));
        if (pt_mr_config == NULL)
        {
            LOG_W("[%s] malloc(%d) failed", __func__, sizeof(Mr_Config_T));
            goto exit;
        }
        s_gui_value = (int32_t)pt_mr_config;

        cur_mr_config = paramsTermConfigMrConfigTake();
        if (cur_mr_config == NULL)
        {
            LOG_W("[%s] take mr config failed", __func__);
            goto exit;
        }
        MY_MEMCPY(pt_mr_config, cur_mr_config, sizeof(Mr_Config_T));
        paramsTermConfigMrConfigRelease(0);

        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value_0 = _userGuiValueDecStep(s_gui_value_0, 0, 5, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value_0 = _userGuiValueIncStep(s_gui_value_0, 0, 5, 1);
            break;
        case KEY_LEFT_DOWN:
        {
            switch(s_gui_value_0)
            {
                case 1:
                    s_gui_value_3 = !s_gui_value_3;
                    break;
                case 2:
                    s_gui_value_2 = _userGuiValueDecStep(s_gui_value_2, 0, MAX_MR_FC_MEALS-1, 1);
                    break;
                case 3:
                    s_gui_value_1 = _userGuiValueDecStep(s_gui_value_1, 0, MAX_MR_FC_TIMES-1, 1);
                    break;
                case 4:
                    pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1] =
                        _userGuiValueDecStep(pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1],
                        0, MAX_MR_F_TIME, 1);
                    break;
                case 5:
                    pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1] =
                        _userGuiValueDecStep(pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1],
                        0, MAX_MR_C_TIME, 1);
                    break;
                case 0:
                default:
                    s_gui_value_0 = 0;
                    pt_mr_config->fc_param.fc_switch = !(pt_mr_config->fc_param.fc_switch);
                    break;
            }
            break;
        }
        case KEY_RIGHT_DOWN:
        {
            switch(s_gui_value_0)
            {
                case 1:
                    s_gui_value_3 = !s_gui_value_3;
                    break;
                case 2:
                    s_gui_value_2 = _userGuiValueIncStep(s_gui_value_2, 0, MAX_MR_FC_MEALS-1, 1);
                    break;
                case 3:
                    s_gui_value_1 = _userGuiValueIncStep(s_gui_value_1, 0, MAX_MR_FC_TIMES-1, 1);
                    break;
                case 4:
                    pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1] =
                        _userGuiValueIncStep(pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1],
                        0, MAX_MR_F_TIME, 1);
                    break;
                case 5:
                    pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1] =
                        _userGuiValueIncStep(pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1],
                        0, MAX_MR_C_TIME, 1);
                    break;
                case 0:
                default:
                    s_gui_value_0 = 0;
                    pt_mr_config->fc_param.fc_switch = !(pt_mr_config->fc_param.fc_switch);
                    break;
            }
            break;
        }
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);

            if (s_gui_value_3 != 0)   /* ���ж�������Ϊһ���Ĳ��� */
            {
                for(uint8_t i = 1; i < MAX_MR_FC_MEALS; i++)
                {
                    MY_MEMCPY(&(pt_mr_config->fc_param.fc_meals[i]), &(pt_mr_config->fc_param.fc_meals[0]),
                              sizeof(pt_mr_config->fc_param.fc_meals[0]));
                }
            }

            cur_mr_config = paramsTermConfigMrConfigTake();
            if (cur_mr_config != NULL)
            {
                MY_MEMCPY(cur_mr_config, pt_mr_config, sizeof(Mr_Config_T));
                paramsTermConfigMrConfigRelease(1);
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                LOG_W("[%s] take mr config failed", __func__);
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
    }

    MY_MEMSET(str, 0, sizeof(str));

    /* ���ÿ��� */
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(6, 0, 127, FONT_LEFT, (uint8_t *)"����:");
    if (s_gui_value_0 == 0)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (pt_mr_config->fc_param.fc_switch == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    GuiRowTextPlus(35, 0, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ����ÿ����ͬ��־ */
    GuiRowTextPlus(60, 0, 127, FONT_LEFT, (uint8_t *)"ÿ����ͬ:");
    if (s_gui_value_0 == 1)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (s_gui_value_3 == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    GuiRowTextPlus(115, 0, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ���ö��� */
    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"����:");
    if(s_gui_value_0 == 2)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%2d", (s_gui_value_2+1));
    GuiRowTextPlus(35, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ���ô��� */
    GuiRowTextPlus(60, 1, 127, FONT_LEFT, (uint8_t *)"����:");
    if(s_gui_value_0 == 3)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%2d", (s_gui_value_1+1));
    GuiRowTextPlus(90, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ������ιʱ�� */
    GuiRowTextPlus(6, 2, 127, FONT_LEFT, (uint8_t *)"��ιʱ��:");
    if(s_gui_value_0 == 4)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%3d����", pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1]);
    GuiRowTextPlus(60, 2, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* �������ʱ�� */
    GuiRowTextPlus(6, 3, 127, FONT_LEFT, (uint8_t *)"���ʱ��:");
    if(s_gui_value_0 == 5)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%3d����", pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1]);
    GuiRowTextPlus(60, 3, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    s_gui_value_0 = 0;  /* chocie */
    s_gui_value_1 = 0;  /* clean */
    s_gui_value_2 = 0;  /* meal */
    s_gui_value_3 = 0;  /* same */
    if (pt_mr_config != NULL)
    {
        MY_FREE(pt_mr_config);
    }
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
/** ���鴥����ʱʱ������ */
static void _userMrTouchDelaySetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    Mr_Config_T *cur_mr_config = NULL;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        cur_mr_config = paramsTermConfigMrConfigTake();
        if (cur_mr_config == NULL)
        {
            LOG_W("[%s] take mr config failed", __func__);
            goto exit;
        }
        s_gui_value = cur_mr_config->touch_delay_time_5s;
        paramsTermConfigMrConfigRelease(0);

        first_enter_flag = 1;
    }

    /* ����״̬���� */
    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 60, 1);
            break;
        case KEY_DOWN_DOWN:
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 60, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            cur_mr_config = paramsTermConfigMrConfigTake();
            if (cur_mr_config != NULL)
            {
                cur_mr_config->touch_delay_time_5s = s_gui_value;
                paramsTermConfigMrConfigRelease(1);
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                LOG_W("[%s] take mr config failed", __func__);
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    MY_MEMSET(str, 0, sizeof(str));

    GuiRowTextPlus(13, 2, 102, FONT_LEFT, (uint8_t *)"�����Ӻ�ʱ��:");
    MY_SNPRINTF(str, sizeof(str), "%3ds", s_gui_value*5);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(13, 2, 102, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}
/** ����ˮ�������� */
static void _userMrFeedWaterSetFunc(void *param)
{
    Mr_Config_T *cur_mr_config = NULL;
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value_0 = 0;  /* chocie */
        s_gui_value_1 = 0;  /* fw switch */
        s_gui_value_2 = 0;  /* water100 */
        s_gui_value_3 = 0;  /* water50 */

        cur_mr_config = paramsTermConfigMrConfigTake();
        if (cur_mr_config == NULL)
        {
            LOG_W("[%s] take mr config failed", __func__);
            goto exit;
        }
        s_gui_value_1 = cur_mr_config->wr_param.mr_fw_switch;
        s_gui_value_2 = cur_mr_config->wr_param.strong_water_100g;
        s_gui_value_3 = cur_mr_config->wr_param.support_water_50g;
        paramsTermConfigMrConfigRelease(0);

        if (s_gui_value_2 < MIN_SW_100G)
        {
            s_gui_value_2 = STRONG_WATER_100G;
        }
        if (s_gui_value_3 < MIN_SW_50G)
        {
            s_gui_value_3 = SUPPORT_WATER_50G;
        }

        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value_0 = _userGuiValueDecStep(s_gui_value_0, 0, 2, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value_0 = _userGuiValueIncStep(s_gui_value_0, 0, 2, 1);
            break;
        case KEY_LEFT_DOWN:
        {
            switch(s_gui_value_0)
            {
                case 1:
                    s_gui_value_2 = _userGuiValueDecStep(s_gui_value_2, MIN_SW_100G, MAX_SW_100G, 1);
                    break;
                case 2:
                    s_gui_value_3 = _userGuiValueDecStep(s_gui_value_3, MIN_SW_50G, MAX_SW_50G, 1);
                    break;
                case 0:
                default:
                    s_gui_value_0 = 0;
                    s_gui_value_1 = !s_gui_value_1;
                    break;
            }
            break;
        }
        case KEY_RIGHT_DOWN:
        {
            switch(s_gui_value_0)
            {
                case 1:
                    s_gui_value_2 = _userGuiValueIncStep(s_gui_value_2, MIN_SW_100G, MAX_SW_100G, 1);
                    break;
                case 2:
                    s_gui_value_3 = _userGuiValueIncStep(s_gui_value_3, MIN_SW_50G, MAX_SW_50G, 1);
                    break;
                case 0:
                default:
                    s_gui_value_0 = 0;
                    s_gui_value_1 = !s_gui_value_1;
                    break;
            }
            break;
        }
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);

            cur_mr_config = paramsTermConfigMrConfigTake();
            if (cur_mr_config != NULL)
            {
                cur_mr_config->wr_param.mr_fw_switch = s_gui_value_1;
                cur_mr_config->wr_param.strong_water_100g = s_gui_value_2;
                cur_mr_config->wr_param.support_water_50g = s_gui_value_3;
                paramsTermConfigMrConfigRelease(1);
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                LOG_W("[%s] take mr config failed", __func__);
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
    }

    MY_MEMSET(str, 0, sizeof(str));

    /* ����ˮ�������� */
    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"��ˮ����:");
    if(s_gui_value_0 == 0)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (s_gui_value_1 == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    GuiRowTextPlus(60, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ���ô̼���ˮ�� */
    GuiRowTextPlus(6, 2, 127, FONT_LEFT, (uint8_t *)"�̼���ˮ:");
    if(s_gui_value_0 == 1)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%4dg", s_gui_value_2*100);
    GuiRowTextPlus(60, 2, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* ���õ�����ˮ�� */
    GuiRowTextPlus(6, 3, 127, FONT_LEFT, (uint8_t *)"������ˮ:");
    if(s_gui_value_0 == 2)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%4dg", s_gui_value_3*50);
    GuiRowTextPlus(60, 3, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    s_gui_value_0 = 0;  /* chocie */
    s_gui_value_1 = 0;  /* fw switch */
    s_gui_value_2 = 0;  /* water100 */
    s_gui_value_3 = 0;  /* water50 */
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/** ˮ����Ͽ������� */
static void _userWaterAlluvialSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterAlluvialFeedGet();
        first_enter_flag = 1;
    }

    /* ����״̬���� */
    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_DOWN_DOWN:
            s_gui_value = !s_gui_value;
            break;
        case KEY_LEFT_DOWN:
            break;
        case KEY_RIGHT_DOWN:
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
            break;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            paramsTermConfigWaterAlluvialFeedSet(s_gui_value);
            GuiWinDraw(&s_win_set_complete);

            paramsSetTermComEvent(0, event_gateway_set_param);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    MY_MEMSET(str, 0, sizeof(str));

    if (s_gui_value == 1)
    {
        MY_SNPRINTF(str, sizeof(str), "��");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "�ر�");
    }

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"ˮ����Ͽ��أ�");
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;

exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* ��ԪӲ����Ϣ������Ͳ���� */
static void _userUnitHwInfoSetFeeder(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetFeederType();
        if (s_gui_value > Feeder_Type_List_Electric_Fork_Puller)
        {
            s_gui_value = Feeder_Type_List_Electric_Fork_Puller;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Feeder_Type_List_Feeder_No_Set, Feeder_Type_List_Electric_Fork_Puller, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Feeder_Type_List_Feeder_No_Set, Feeder_Type_List_Electric_Fork_Puller, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetFeederType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(10, 2, 70, FONT_LEFT, (uint8_t *)"�������ͣ�");

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Feeder_Type_List_Small_Strand:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"С����  ");
            break;
        case Feeder_Type_List_Closures:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"�շ���  ");
            break;
        case Feeder_Type_List_Electric_Fork_Puller:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"�綯����");
            break;
        default:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"δ����  ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
#endif
/* ��ԪӲ����Ϣ�����ϲ����� */
static void _userUnitHwInfoSetTrough(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetTroughType();
        if (s_gui_value > Feed_Trough_Type_List_ONE_Tubular_Trough)
        {
            s_gui_value = Feed_Trough_Type_List_ONE_Tubular_Trough;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Feed_Trough_Type_List_Feed_Trough_No_Set, Feed_Trough_Type_List_ONE_Tubular_Trough, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Feed_Trough_Type_List_Feed_Trough_No_Set, Feed_Trough_Type_List_ONE_Tubular_Trough, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetTroughType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);
            /*���ݲ�ͬ�ϲ����� ������ι3.0��ͬ���ض����������ޡ�����ʱ��*/
            switch(s_gui_value)
            {
                /* ������ */
                case Feed_Trough_Type_List_BY_Long_Trough:      /* ������ ���� */
                case Feed_Trough_Type_List_BY_Disc_Trough:      /* ������ Բ�� */
                case Feed_Trough_Type_List_BY_Half_Disc_Trough: /* ������ ��Բ�� */
                case Feed_Trough_Type_List_ONE_Long_Trough:     /* һ���� ���� */
                case Feed_Trough_Type_List_ONE_1m_Long_Trough:  /* һ���� 1�׳��� */
                case Feed_Trough_Type_List_YF_Long_Trough:      /* ������ ���� */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_LONG_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* Բ���� */
                case Feed_Trough_Type_List_ONE_Disc_Trough: /* һ���� Բ�ϲ� */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_DISC_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* ���Ͱ�� */
                case Feed_Trough_Type_List_BY_Toggle_Trough:    /* ������ �����ϲ� */
                case Feed_Trough_Type_List_BY_Endotube_Trough:  /* ������ �ڲ���ϲ� */
                case Feed_Trough_Type_List_YF_Toggle_Trough:    /* ������ �����ϲ� */
                case Feed_Trough_Type_List_YF_Endotube_Trough:  /* ������ ����ϲ� */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_BIG_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* ��ʽ�� */
                case Feed_Trough_Type_List_ONE_Tubular_Trough:
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_TUBULAR_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                default:
                    break;
            }

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        /* ������ */
        case Feed_Trough_Type_List_MR_Standard_Feed_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��׼�ϲ�  ");
            break;
        case Feed_Trough_Type_List_MR_Non_Standard_Feed_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �Ǳ�׼�ϲ�");
            break;
        /* ������ */
        case Feed_Trough_Type_List_BY_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ����      ");
            break;
        case Feed_Trough_Type_List_BY_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ Բ��      ");
            break;
        case Feed_Trough_Type_List_BY_Half_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��Բ��    ");
            break;
        case Feed_Trough_Type_List_BY_Toggle_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �����ϲ�  ");
            break;
        case Feed_Trough_Type_List_BY_Endotube_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �ڲ���ϲ�");
            break;
        /* ������ */
        case Feed_Trough_Type_List_YF_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ����      ");
            break;
        case Feed_Trough_Type_List_YF_Toggle_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �����ϲ�  ");
            break;
        case Feed_Trough_Type_List_YF_Endotube_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ����ϲ�  ");
            break;
        /* һ���� */
        case Feed_Trough_Type_List_ONE_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ����      ");
            break;
        case Feed_Trough_Type_List_ONE_1m_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� 1�׳���   ");
            break;
        case Feed_Trough_Type_List_ONE_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� Բ�ϲ�    ");
            break;
        case Feed_Trough_Type_List_ONE_Tubular_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ��ʽ�ϲ�  ");
            break;

        default:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"δ����           ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* ��ԪӲ����Ϣ����̽������ */
static void _userUnitHwInfoSetProbeType(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetProbeType();
        if (s_gui_value > Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe)
        {
            s_gui_value = Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Feed_Probe_Type_List_Feed_Probe_No_Set, Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Feed_Probe_Type_List_Feed_Probe_No_Set, Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetProbeType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        /* ������ */
        case Feed_Probe_Type_List_MR_L_TYPE_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ L��̽��       ");
            break;
        case Feed_Probe_Type_List_MR_Four_In_One_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �ĺ�һ̽��    ");
            break;
        case Feed_Probe_Type_List_MR_Drinking_Rod_Universal_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��ˮ������̽�� ");
            break;
        case Feed_Probe_Type_List_MR_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��ˮ�˰ڶ�̽�� ");
            break;
        case Feed_Probe_Type_List_MR_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��˿��̽��     ");
            break;

        /* ������ */
        case Feed_Probe_Type_List_BY_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��˿��̽��     ");
            break;
        case Feed_Probe_Type_List_BY_Drinking_Rod_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��ˮ�˸�˿��̽��");
            break;
        case Feed_Probe_Type_List_BY_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��ˮ�˰ڶ�̽��  ");
            break;

        /* ������ */
        case Feed_Probe_Type_List_YF_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��˿��̽��      ");
            break;
        case Feed_Probe_Type_List_YF_Reinforcing_Steel_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ �ֽ�̽��        ");
            break;
        case Feed_Probe_Type_List_YF_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"������ ��ˮ�˰ڶ�̽��  ");
            break;

        /* һ���� */
        case Feed_Probe_Type_List_ONE_Drinking_Rod_Double_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ��ˮ��˫̽��    ");
            break;
        case Feed_Probe_Type_List_ONE_Box_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ��ʽ̽��        ");
            break;
        case Feed_Probe_Type_List_ONE_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ��˿��̽��      ");
            break;
        case Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"һ���� ��ˮ�˰ڶ�̽��  ");
            break;

        default:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"δ����                ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
#if 0
/* ��ԪӲ����Ϣ����̽��߶� */
static void _userUnitHwInfoSetProbeHigh(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetProbeHigh();
        if (s_gui_value < Feed_Probe_Hight_List_Min_Hight)
        {
            s_gui_value = Feed_Probe_Hight_List_Min_Hight;
        }
        else if (s_gui_value > Feed_Probe_Hight_List_Max_Hight)
        {
            s_gui_value = Feed_Probe_Hight_List_Max_Hight;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Feed_Probe_Hight_List_Min_Hight, Feed_Probe_Hight_List_Max_Hight, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Feed_Probe_Hight_List_Min_Hight, Feed_Probe_Hight_List_Max_Hight, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetProbeHigh(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    rt_snprintf(str, sizeof(str), "%d.%d ", s_gui_value/10, s_gui_value%10);

    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(40, 2, 24, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(65, 2, 63, FONT_LEFT, (uint8_t *)"����");

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* ��ԪӲ����Ϣ���ô������� */
static void _userUnitHwInfoSetTouchType(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetTouchType();
        if (s_gui_value > Touch_Type_List_L_Type_Touch)
        {
            s_gui_value = Touch_Type_List_L_Type_Touch;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Touch_Type_List_Touch_No_Set, Touch_Type_List_L_Type_Touch, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Touch_Type_List_Touch_No_Set, Touch_Type_List_L_Type_Touch, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetTouchType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Touch_Type_List_No_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"�޴�����");
            break;
        case Touch_Type_List_Travel_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"�г�ʽ  ");
            break;
        case Touch_Type_List_Reed_Switch_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"�ɻɹ�ʽ");
            break;
        case Touch_Type_List_L_Type_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"��еʽ  ");
            break;
        default:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"δ����  ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* ��ԪӲ����Ϣ�����Ϲ޼�� */
static void _userUnitHwInfoSetFoodSpace(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetFoodSpace();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 255, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 255, 1);
            break;
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 255, 5);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 255, 5);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetFoodSpace(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(1, 2, 65, FONT_RIGHT, (uint8_t *)"���Ϲ޼�� ");

    rt_snprintf(str, sizeof(str), "%3d", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(66, 2, 18, FONT_LEFT, (uint8_t*)str);
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(84, 2, 44, FONT_LEFT, (uint8_t *)" ����Ԫ");

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* ��ԪӲ����Ϣ����ˮԴ��� */
static void _userUnitHwInfoSetWaterSpace(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetWaterSpace();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 255, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 255, 1);
            break;
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 255, 5);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 255, 5);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetWaterSpace(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiRowTextPlus(1, 2, 65, FONT_RIGHT, (uint8_t *)"��ˮԴ��� ");

    rt_snprintf(str, sizeof(str), "%3d", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(66, 2, 18, FONT_LEFT, (uint8_t*)str);
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(84, 2, 44, FONT_LEFT, (uint8_t *)" ����Ԫ");

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* ��ԪӲ����Ϣ���õ������ */
static void _userUnitHwInfoSetMotorType(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetMotorType();
        if (s_gui_value >= TERM_MOTOR_TYPE_MAX_E)
        {
            s_gui_value = TERM_MOTOR_TYPE_MAX_E-1;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, TERM_MOTOR_TYPE_NO_E, TERM_MOTOR_TYPE_MAX_E-1, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, TERM_MOTOR_TYPE_NO_E, TERM_MOTOR_TYPE_MAX_E-1, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetMotorType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case TERM_MOTOR_TYPE_ZHENGK_5_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"����5Ƶ ");
            break;
        case TERM_MOTOR_TYPE_ZHENGK_2_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"����2Ƶ ");
            break;
        case TERM_MOTOR_TYPE_CJH_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"������  ");
            break;
        case TERM_MOTOR_TYPE_YT_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"����    ");
            break;
        case TERM_MOTOR_TYPE_JL_LONG_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"��������");
            break;
        case TERM_MOTOR_TYPE_JL_SHORT_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"�����̿�");
            break;
        case TERM_MOTOR_TYPE_DMK_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"�����  ");
            break;
        default:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"δ����  ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
#endif
/* ��ԪӲ����Ϣ����һ�巧���� */
static void _userUnitHwInfoSetFlowType(void *param)
{
    uint16_t water_value_speed = 0;
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramUnitHwInfoGetFlowType();
        if (s_gui_value > Flowmeter_Type_List_Pulse_1400)
        {
            s_gui_value = Flowmeter_Type_List_Pulse_1400;
        }
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, Flowmeter_Type_List_Pulse_No, Flowmeter_Type_List_Pulse_1400, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, Flowmeter_Type_List_Pulse_No, Flowmeter_Type_List_Pulse_1400, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            water_value_speed = paramsTermConfigWaterSpeedGet();
            switch(s_gui_value)
            {
                case Flowmeter_Type_List_Pulse_1400:
                case Flowmeter_Type_List_Pulse_1200:
                    if ((water_value_speed < 60)||(water_value_speed > 100))
                    {
                        paramsTermConfigWaterSpeedSet(60);
                        paramsSetTermComEvent(0, event_gateway_set_param);
                    }
                    break;
                case Flowmeter_Type_List_Pulse_600:
                case Flowmeter_Type_List_Pulse_520:
                    if ((water_value_speed < 100)||(water_value_speed > 180))
                    {
                        paramsTermConfigWaterSpeedSet(160);
                        paramsSetTermComEvent(0, event_gateway_set_param);
                    }
                    break;
                default:
                    break;
            }
            GuiWinDraw(&s_win_set_ing);
            if (paramUnitHwInfoSetFlowType(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_term_hw_info);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Flowmeter_Type_List_Pulse_1400:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"1400����/L");
            break;
        case Flowmeter_Type_List_Pulse_1200:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"1200����/L");
            break;
        case Flowmeter_Type_List_Pulse_600:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"600����/L ");
            break;
        case Flowmeter_Type_List_Pulse_520:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"520����/L ");
            break;
        default:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"δ����    ");
            break;
    }
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/** ��ιģ���䷽���� */
static void _userFeedModelRecipeSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsFeedModelRecipeGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, Recipe_total-1, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, Recipe_total-1, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);
            if (paramsFeedModelRecipeSet(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"�䷽ѡ��: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Corn:
            MY_SNPRINTF(str, sizeof(str), "����");break;
        case Brown_rice:
            MY_SNPRINTF(str, sizeof(str), "����");break;
        case Broken_rice:
            MY_SNPRINTF(str, sizeof(str), "����");break;
        case Barley:
            MY_SNPRINTF(str, sizeof(str), "����");break;
        case Wheat:
            MY_SNPRINTF(str, sizeof(str), "С��");break;
        case Kaoliang:
            MY_SNPRINTF(str, sizeof(str), "����");break;
        default:
            MY_SNPRINTF(str, sizeof(str), "����");break;
    }
    GuiRowTextPlus(22, 2, 84, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
/** ��ιģ�ͺ��������� */
static void _userFeedModelPowderSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsFeedModelPowderGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, Powder_total-1, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, Powder_total-1, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);

            if (paramsFeedModelPowderSet(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(19, 2, 90, FONT_LEFT, (uint8_t *)"����ѡ��: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Powder_0:
            MY_SNPRINTF(str, sizeof(str), " <10 ");break;
        case Powder_10:
            MY_SNPRINTF(str, sizeof(str), "10~30");break;
        case Powder_30:
            MY_SNPRINTF(str, sizeof(str), "30~60");break;
        case Powder_60:
            MY_SNPRINTF(str, sizeof(str), "60~90");break;
        case Powder_90:
            MY_SNPRINTF(str, sizeof(str), " >90 ");break;
        default:
            MY_SNPRINTF(str, sizeof(str), "���� ");break;
    }
    GuiRowTextPlus(19, 2, 90, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
/** ��ιģ����ʪ������ */
static void _userFeedModelTempHumiSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];
    uint8_t deviation = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        paramsFeedModelTempHumiGet(&s_df_gui_value_0, &s_df_gui_value_1);
        s_gui_value_0 = 1;  /* choice */
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            switch(s_gui_value_0)
            {
                case 1:
                    s_df_gui_value_0 += 10;
                    break;
                case 2:
                    s_df_gui_value_0 += 1;
                    break;
                case 3:
                    s_df_gui_value_0 += 0.1;
                    break;
                case 4:
                    s_df_gui_value_1 += 0.1;
                    break;
                case 5:
                    s_df_gui_value_1 += 0.01;
                    break;
                default:break;
            }
            if((s_df_gui_value_0 < 20) || (s_df_gui_value_0 > 38))
            {
                s_df_gui_value_0 = 38;
            }
            if((s_df_gui_value_1 < 0.4) || (s_df_gui_value_1 > 0.99))
            {
                s_df_gui_value_1 = 0.99;
            }
            break;
        case KEY_DOWN_DOWN:
            switch(s_gui_value_0)
            {
                case 1:
                    s_df_gui_value_0 -= 10;
                    break;
                case 2:
                    s_df_gui_value_0 -= 1;
                    break;
                case 3:
                    s_df_gui_value_0 -= 0.1;
                    break;
                case 4:
                    s_df_gui_value_1 -= 0.1;
                    break;
                case 5:
                    s_df_gui_value_1 -= 0.01;
                    break;
                default:break;
            }
            if((s_df_gui_value_0 < 20) || (s_df_gui_value_0 > 38))
            {
                s_df_gui_value_0 = 20;
            }
            if((s_df_gui_value_1 < 0.4) || (s_df_gui_value_1 > 0.99))
            {
                s_df_gui_value_1 = 0.4;
            }
            break;
        case KEY_LEFT_DOWN:
            s_gui_value_0 = _userGuiValueDecStep(s_gui_value_0, 1, 5, 1);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value_0 = _userGuiValueIncStep(s_gui_value_0, 1, 5, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);

            if (paramsFeedModelTempHumiSet(&s_df_gui_value_0, &s_df_gui_value_1) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"�¶�     ʪ��");
    /* ���ﲻ����MY_SNPRINTF, ��Ϊ��ӡ���˸����� */
    snprintf(str, sizeof(str), "%02.1f��   %02.1f%%", s_df_gui_value_0, (s_df_gui_value_1*100));
    GuiRowTextPlus(6, 2, 127, FONT_LEFT, (uint8_t *)str);

    MY_MEMSET(str, 0, sizeof(str));
    if(s_gui_value_0 == 3)
    {
        deviation = 6;
    }
    else if((s_gui_value_0 == 4) || (s_gui_value_0 == 5))
    {
        deviation = 6*6;
    }
    else
    {
        deviation = 0;
    }
    GuiRowTextPlus(6 , 3, 127, FONT_LEFT, (uint8_t *)"-------------");
    MY_SNPRINTF(str, sizeof(str), "*");
    GuiRowTextPlus(6 + (s_gui_value_0-1)*6+deviation, 3, 127, FONT_LEFT, (uint8_t *)str);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    s_df_gui_value_0 = 0;  /* temp */
    s_df_gui_value_1 = 0;  /* humi */
    s_gui_value_0 = 0;  /* choice */

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
/** ��ιģ�ͽ������� */
static void _userFeedModelHealthSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsFeedModelHealthGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, Health_total-1, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, Health_total-1, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);
            
            if (paramsFeedModelHealthSet(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_feed_model_healthy);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"����ѡ��: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Mild_fever:
            MY_SNPRINTF(str, sizeof(str), "��ȷ���");
            break;
        case Moderate_fever:
            MY_SNPRINTF(str, sizeof(str), "�жȷ���");
            break;
        case Severe_fever:
            MY_SNPRINTF(str, sizeof(str), "�ضȷ���");
            break;
        case Mild_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "��ȸ�к");
            break;
        case Moderate_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "�жȸ�к");
            break;
        case Severe_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "�ضȸ�к ");
            break;
        case Mild_immune:
            MY_SNPRINTF(str, sizeof(str), "�������");
            break;
        case Moderate_immune:
            MY_SNPRINTF(str, sizeof(str), "�ж�����");
            break;
        case Severe_immune:
            MY_SNPRINTF(str, sizeof(str), "�ض�����");
            break;
        case healthy:
        default:
            s_gui_value = healthy;
            MY_SNPRINTF(str, sizeof(str), "��Ⱥ����");
            break;
    }
    GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
/** ��ιģ���������� */
static void _userFeedModelWeightSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsFeedModelWeightGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 2, 180, 1);
            break;
        case KEY_DOWN_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 2, 180, 1);
            break;
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 2, 180, 10);
            break;
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 2, 180, 10);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);

            if (paramsFeedModelWeightSet(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }
            paramsSetTermComEvent(0, event_set_feed_model_pig_weight);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(28, 2, 72, FONT_LEFT, (uint8_t *)"����: ");
    MY_SNPRINTF(str, sizeof(str), "%3d Kg", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(28, 2, 72, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}
/** ��ιģ�Ϳ������� */
static void _userFeedModelOpenSet(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsFeedModelOpenGet();
        first_enter_flag = 1;
    }

    switch (s_i_key_status)
    {
        case KEY_UP_DOWN:
        case KEY_LEFT_DOWN:
            s_gui_value = _userGuiValueDecStep(s_gui_value, 0, 2, 1);
            break;
        case KEY_DOWN_DOWN:
        case KEY_RIGHT_DOWN:
            s_gui_value = _userGuiValueIncStep(s_gui_value, 0, 2, 1);
            break;
        case KEY_CANCEL_DOWN:
            goto exit;
        case KEY_OK_DOWN:
        {
            GuiWinDraw(&s_win_set_ing);

            if (paramsFeedModelOpenSet(s_gui_value) == RT_EOK)
            {
                GuiWinDraw(&s_win_set_complete);
            }
            else
            {
                GuiWinDraw(&s_win_set_failed);
            }

            paramsSetTermComEvent(0, event_set_feed_model_open);

            s_gui_wait_flag = 1;
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        }
        default:
            break;
    }

    GuiRowTextPlus(16, 2, 96, FONT_LEFT, (uint8_t *)"ģ�Ϳ���: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case 1:
            MY_SNPRINTF(str, sizeof(str), " �Ͽ� ");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "ˮ�Ͽ�");
            break;
        case 0:
        default:
            s_gui_value = 0;
            MY_SNPRINTF(str, sizeof(str), "  ��  ");
            break;
    }
    GuiRowTextPlus(16, 2, 96, FONT_RIGHT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    GuiUpdateDisplayAll();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    s_gui_value = 0;

    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    return;
}

/* �ַ���תΪ��ά�벢��ʾ���� */
static void _userMakeQrcode(uint8_t addr_x, uint8_t addr_y, uint8_t* qrstr)
{

#define DEFAULT_QR_VERSION 7

    uint8_t x, y;
    int8_t result;
    QRCode qrc;

    uint8_t *qrcodeBytes = (uint8_t *)MY_CALLOC(1, qrcode_getBufferSize(DEFAULT_QR_VERSION));
    if (qrcodeBytes == NULL)
    {
        return;
    }

    result = qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, (const char *)qrstr);
    if(result >= 0)
    {
        for (y = 0; y < qrc.size; y++)
        {
            for (x = 0; x < qrc.size; x++)
            {
                if (qrcode_getModule(&qrc, x, y))
                {
                    GuiDrawPoint(addr_x+x,addr_y+y,1);
                }
                else
                {
                    GuiDrawPoint(addr_x+x,addr_y+y,0);
                }
            }
        }
    }
    MY_FREE(qrcodeBytes);
}

/* ��������mqtt���� */
static int32_t _userMqttRestart(void)
{
    Plat_Eos_Custom_Data_Head_T t_eos_custom_data_head = {0};

    t_eos_custom_data_head.e_src  = PLAT_EOS_CUSTOM_DATA_SRC_GUI_E;
    t_eos_custom_data_head.e_type = PLAT_EOS_CUSTOM_DATA_TYPE_RESTART_E;

    /* ͨ��eos֪ͨfeed mqtt���¿�ʼmqtt���� */
    if (platEosEventPubCustomData(PLAT_EOS_EVENT_FEED_MQTT_E, &t_eos_custom_data_head, NULL, 0) == RT_EOK)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

/* ��������: min <= max */
static int _userGuiValueIncStep(int value, int min, int max, int step)
{
    value += step;
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return min;
    }
    return value;
}
/* ��������: min <= max */
static int _userGuiValueDecStep(int value, int min, int max, int step)
{
    value -= step;
    if (value < min)
    {
        return max;
    }
    else if (value > max)
    {
        return max;
    }
    return value;
}
/************************************************/
/*                 ���⺯��ʵ��                 */
/************************************************/
/**
  *@brief gui��ʼ���ο�����
  *@param  None
  *@retval None
  */
void userAppPortInit(void)
{
    s_pt_key_node = platNodeMidFind(MID_NODE_KEY_NAME);
    if (s_pt_key_node != NULL)
    {
        s_pt_key_node->pf_init(NULL);
        LOG_I("[%s] mid %s init ok", __func__, MID_NODE_KEY_NAME);
    }
    else
    {
        LOG_E("[%s] mid %s not find", __func__, MID_NODE_KEY_NAME);
    }

    /* 1. ��ʼ��lcd */
    LcdInit();
    /* 2. ��ʼ���û����� */
    defaultFontInit();
    /* 3.�˵���ʼ�� */
    UserMenuInit();
    /* 4.��ʼ����ʾ */
    GuiSetForecolor(CBLACK);
    GuiSetbackcolor(CWHITLE);
    _guiPowerOnProgress();
    /* 5. ��ʼ������ */
    GuiWinInit();
    /* 6. ������Ļ */
    GuiClearScreen(GuiGetbackcolor());
    /* 7. ������ҳ��  */
    GuiWinAdd(&s_win_home);

    OpenLcdDisplay();
    GuiUpdateDisplayAll();
    s_gui_inited_end = 1;
}

/**
  *@brief gui���к���
  *@param  None
  *@retval None
  */
void userAppPortRun(void)
{
    static uint8_t font_load_flag = 0;
    static uint32_t last_tick = 0;
    uint32_t cur_tick = MY_TICK_GET();

    /* userAppPortInit û��ִ������ֱ�ӷ��� */
    if (s_gui_inited_end == 0)      return;

    /* ��ȡ����״̬ */
    if (s_pt_key_node != NULL)
    {
        s_i_key_status = s_pt_key_node->pf_ctrl(MY_NODE_CTRL_CMD_READ,&s_i_key_status);
    }
    else
    {
        s_i_key_status = 0;
    }

    /* ��û�а�������ʱ����Ļ��ִ����ôƵ�� */
    if ((s_i_key_status == 0)&&(s_gui_saving == 0)&&((uint32_t)(cur_tick-last_tick) < 200))
    {
        return;
    }
    last_tick = cur_tick;

    /* ���ڵ��ȹ��� */
    /* Lora�ն�Զ��ע�����, ��������ͬʱ���㣺Զ��ע���־��Ϊ0����Lora�ն� */
    if ((paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_LORA) && (paramsLoraGetRemoteRegFlag() == 1))
    {
        _userWinRemoteReg();
    }
    
    if (appOtaGetCurUpdateType() == OTA_FIEL_TYPE_GW_FW_E)
    {
        if (font_load_flag == 1)
        {
            font_load_flag = 0;
            GuiClearScreen(GuiGetbackcolor());
            GuiMenuRedrawMenu(&HmiMenu);
        }
        GuiWinDraw(&s_win_gateway_update);
    }
    else if(paramsCheckChFont() != RT_EOK)
    {
        if (font_load_flag == 0)
        {
            font_load_flag = 1;
            GuiClearScreen(GuiGetbackcolor());
        }
        _userWinFontLoadOnProcess();
    }
    else
    {
        if (font_load_flag == 1)
        {
            font_load_flag = 0;
            GuiClearScreen(GuiGetbackcolor());
            GuiMenuRedrawMenu(&HmiMenu);
        }
        GuiWinDisplay();
    }

    /* ����״̬��� */
    if(s_i_key_status)
    {
        //LOG_D("[key]:%d\n",s_i_key_status);
        s_i_key_status = 0;
        if (s_pt_key_node != NULL)
        {
            s_i_key_status = s_pt_key_node->pf_ctrl(MY_NODE_CTRL_CMD_WRITE,&s_i_key_status);
        }
    }
}

/* ��Ӧ�ó�ʼ����ɺ󣬵��øú�����֪ͨgui������Ӧ�þ������ */
void userGuiSetProcessEnd(void)
{
    s_gui_process_end = 1;
}




