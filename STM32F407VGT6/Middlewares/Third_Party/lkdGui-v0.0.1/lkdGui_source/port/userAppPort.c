/**
  * @file   userAppPort.c
  * @author  guoweilkd
  * @version V0.0.0
  * @date    2019/01/17
  * @brief  lkdGui用户应用编写文件。
  * 请使用GB2312编码格式打开此文件！！！！！！！！！！！！
  * 请使用GB2312编码格式打开此文件！！！！！！！！！！！！
  * 请使用GB2312编码格式打开此文件！！！！！！！！！！！！
  * 请使用GB2312编码格式打开此文件！！！！！！！！！！！！
  * 请使用GB2312编码格式打开此文件！！！！！！！！！！！！
  * 说明：旧版本中的网络状态界面没有添加进来 Deprecated
  */

/************************************************/
/*                 头文件包含                    */
/************************************************/
/* 数据类型定义头文件 */
#include "basic_os.h"

/* 下层引用头文件 */
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
/* 本模块声明头文件 */

/* 当前层私有引用头文件 */
#include "lkdGui.h"
#include "UC1698Driver.h"

#define DBG_TAG "GUI"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <stdio.h>
/************************************************/
/*                 数据模型                                                         */
/************************************************/
/* 宏 */
#define MENUSTACK_NUM               8         //@ 与 GUIWINMANAGE_NUM 相关

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

/* 对应s_gui_custom_flag中的对应位 */
/** 1-表示处于正在进行地址冲突界面显示; 0-表示没有地址冲突界面显示 */
#define GUI_CUSTOM_FLAG_ADDR_COLLIDE         0

/*按键句柄*/
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

/* 菜单句柄 */
static lkdMenu HmiMenu;
/* 菜单栈 */
static MenuStack userMenuStack[MENUSTACK_NUM];

/* 当应用初始化完成后，该变量用于通知gui进度条应该结束, 0:表示进度条继续等待；1-表示进度条应该尽快结束  */
static uint8_t s_gui_process_end = 0;
/* gui初始化完成标志，0-未初始化完成；1-初始化完成 */
static uint8_t s_gui_inited_end = 0;
/* gui有数据需要保存标志 */
static uint8_t s_gui_saving = 0;
/* 用于异步使用 */
static uint16_t s_gui_term_addr = 1;
static uint8_t s_gui_wait_flag = 0;
static uint32_t s_gui_wait_last_tick = 0;
static uint8_t first_enter_flag = 0;

/* 这些值用于一些设置 */
static int32_t s_gui_value = 0;
static uint8_t s_gui_value_0 = 0;
static uint8_t s_gui_value_1 = 0;
static uint8_t s_gui_value_2 = 0;
static uint8_t s_gui_value_3 = 0;

static double s_df_gui_value_0 = 0;
static double s_df_gui_value_1 = 0;

/* 每一位代表一个标志 */
static uint32_t s_gui_custom_flag = 0;

/************************************************/
/*                 外部调用函数集中声明           */
/************************************************/
extern void GuiWinDraw(const lkdWin *pWindow);

/************************************************/
/*                 内部函数集中声明               */
/************************************************/
/** 主界面 */
static void _userWinHomeFunc(void *param);
/** 注册窗口服务函数 */
static void _userWinLoraRegFunc(void *param);
/** 二维码窗口服务函数 */
static void _userWinQrCodeFunc(void *param);
/** 网关与终端通信成功率窗口服务函数 */
static void _userCommSuccRateFunc(void *param);
/** 网关与Lora终端通信信号强度窗口服务函数 */
static void _userLoraRssiFunc(void *param);
/* 地址冲突窗口服务函数 */
static void _userAddrCollideFunc(void *param);

/** 菜单界面服务函数 */
static void _userMenuFun(void *param);

/** 单元类型设置服务函数 */
static void _userUnitTypeSetFunc(void *param);
/** 单元号设置服务函数 */
static void _userUnitNumSetFunc(void *param);
/** 单元栏位设置服务函数 */
static void _userUnitTermRawSetFunc(void *param);

/** 饲喂平台ip和端口设置界面服务函数 */
static void _userServerInfoSetFunc(void *param);
/** 饲喂平台选择界面服务函数 */
static void _userMqttSelectSetFunc(void *param);
#if 0
/** 用户升级选择界面服务函数 */
static void _userUpgradeModeSetFunc(void *param);
#endif

/** 单元通信类型设置界面服务函数 */
static void _userUnitCommTypeSetFunc(void *param);

/** 饲喂方案下发方式设置界面服务函数 */
static void _userUnitFeedVersionSetFunc(void *param);

/** 单元下料方式设置界面服务函数*/
static void _userUnitFoodModeSetFunc(void *param);

/** lora信号强度测试界面服务函数 */
static void _userLorsTestFunc(void *param);
#if 0
/** 饲喂参数重置并下发界面服务函数 */
static void _userFeedParamSetDefault(void *param);
#endif
/** 本地饲喂记录清空界面 */
static void _userTermRecodeClear(void *param);

/** 无线信道设置界面服务函数 */
static void _userLoraChannelSetFunc(void *param);

/** 无线发送功率设置界面服务函数 */
static void _userLoraTxpowerSetFunc(void *param);

/** 无线扩频因子设置界面服务函数 */
static void _userLoraSfSetFunc(void *param);

#if 0
/** 无线编码率设置界面服务函数 */
static void _userLoraCrSetFunc(void *param);

/** 无线带宽设置界面服务函数 */
static void _userLoraBwSetFunc(void *param);

/** 无线信道自动调整开关设置界面服务函数 */
static void _userLoraChAdjustSetFunc(void *param);

/** 无线信道自动调整开始界面服务函数 */
static void _userLoraChAdjustStartFunc(void *param);
#endif

/** Lora通信参数初始化界面服务函数 */
static void _userLoraSetDefaultFunc(void *param);

#if 0
/** 配置断水模式界面服务函数 */
static void _userConfigNoWaterFunc(void *param);

/** 配置销售模式时长设置界面服务函数 */
static void _userConfigSellTimeSetFunc(void *param);

/** 配置销售模式开关设置界面服务函数 */
static void _userConfigSellSwitchSetFunc(void *param);
#endif

/** 探针阈值设置界面服务函数 */
static void _userConfigProbeAdcSetFunc(void *param);

/** 动态清槽开关设置界面服务函数 */
static void _userFoodClearSwitchSetFunc(void *param);

/** 动态清槽检查时间设置界面服务函数 */
static void _userFoodClearCheckTimeSetFunc(void *param);

/** 动态清槽时间设置界面服务函数 */
static void _userFoodClearFoodTimeSetFunc(void *param);

/** 动态清水时间设置界面服务函数 */
static void _userFoodClearWaterTimeSetFunc(void *param);

/** 动态清槽起始日龄设置界面服务函数 */
static void _userFoodClearStartAgeSetFunc(void *param);

/* 动态清槽结束日龄设置界面服务函数 */
static void _userFoodClearStopAgeSetFunc(void *param);

#if 0
/** 保护时间设置界面服务函数 */
static void _userProtectTimeSetFunc(void *param);
#endif
/* 下水模式设置界面服务函数 */
static void _userWaterModeSetFunc(void *param);
/* 后端盖探针灵敏度设置界面服务函数 */
static void _userProbeSenSetFunc(void *param);
/**一体阀速率设置界面*/
static void _userValveSpeedSetFunc(void *param);
#if 0
/* 流速补偿设置界面服务函数 */
static void _userWaterSpeedDiffSetFunc(void *param);
#endif
/** 脉冲校准设置界面服务函数 */
static void _userProtectPulseCalFunc(void *param);
/** 增强下料设置界面服务函数 */
static void _userStrongFoodGearSetFunc(void *param);
/** 增强下水设置界面服务函数 */
static void _userStrongWaterGearSetFunc(void *param);

/** 泌乳清槽时间设置 */
static void _userMrFeedCleanSetFunc(void *param);
/** 泌乳触碰延时时间设置 */
static void _userMrTouchDelaySetFunc(void *param);
/** 泌乳水参数设置 */
static void _userMrFeedWaterSetFunc(void *param);
/** 水冲积料开关设置 */
static void _userWaterAlluvialSetFunc(void *param);

#if 0
/** 单元硬件信息设置料筒类型 */
static void _userUnitHwInfoSetFeeder(void *param);
#endif
/** 单元硬件信息设置料槽类型 */
static void _userUnitHwInfoSetTrough(void *param);
/** 单元硬件信息设置探针类型 */
static void _userUnitHwInfoSetProbeType(void *param);
#if 0
/** 单元硬件信息设置探针高度 */
static void _userUnitHwInfoSetProbeHigh(void *param);
/** 单元硬件信息设置触碰类型 */
static void _userUnitHwInfoSetTouchType(void *param);
/** 单元硬件信息设置料罐间距 */
static void _userUnitHwInfoSetFoodSpace(void *param);
/** 单元硬件信息设置水源间距 */
static void _userUnitHwInfoSetWaterSpace(void *param);
/** 单元硬件信息设置电机类型 */
static void _userUnitHwInfoSetMotorType(void *param);
#endif
/** 单元硬件信息设置一体阀类型 */
static void _userUnitHwInfoSetFlowType(void *param);

/** 饲喂模型配方设置 */
static void _userFeedModelRecipeSet(void *param);
/** 饲喂模型含粉率设置 */
static void _userFeedModelPowderSet(void *param);
/** 饲喂模型温湿度设置 */
static void _userFeedModelTempHumiSet(void *param);
/** 饲喂模型健康设置 */
static void _userFeedModelHealthSet(void *param);
/** 饲喂模型体重设置 */
static void _userFeedModelWeightSet(void *param);
/** 饲喂模型开关设置 */
static void _userFeedModelOpenSet(void *param);

/** 字符串转为二维码并显示函数 */
static void _userMakeQrcode(uint8_t addr_x, uint8_t addr_y, uint8_t* qrstr);

/** 用于重启mqtt服务 */
static int32_t _userMqttRestart(void);

/* 限制条件: min <= max */
static int _userGuiValueIncStep(int value, int min, int max, int step);
/* 限制条件: min <= max */
static int _userGuiValueDecStep(int value, int min, int max, int step);

/************************************************/
/*                 内部界面集中声明               */
/************************************************/
/* 界面 */
/** 主界面(桌面) */
const static lkdWin s_win_home                   = {0, 0, 128, 64, NULL,                   _userWinHomeFunc,      NULL};
/** 注册界面：快捷键下键进入/调试->注册 */
const static lkdWin s_win_lora_reg               = {0, 0, 128, 64, (uint8_t*)"注册",        _userWinLoraRegFunc, NULL};
/** 二维码显示界面：快捷键上建进入  */
const static lkdWin s_win_Qrcode                 = {0, 0, 128, 64, NULL,                   _userWinQrCodeFunc,    NULL};
/** 网关与终端通信成功率信息界面：快捷键左键进入/Lora->通信 */
const static lkdWin s_win_comm_info              = {0, 0, 128, 64, (uint8_t*)"终端通信详情",  _userCommSuccRateFunc, NULL};
/** 网关与lora终端信号强度信息界面：快捷键右键进入/Lora->强度 */
const static lkdWin s_win_lora_rssi              = {0, 0, 128, 64, (uint8_t*)"Lora信号强度", _userLoraRssiFunc,     NULL};

/** 地址冲突显示界面 */
const static lkdWin s_win_addr_collide           = {10, 5, 108, 54, (uint8_t*)"地址冲突",    _userAddrCollideFunc, NULL};

/** 菜单界面：确认键进入 */
const static lkdWin s_win_menu                   = {0, 0, 128, 64, NULL,                   _userMenuFun,          NULL};

/** 猪舍类型设置界面：猪舍->类型 */
const static lkdWin s_win_unit_type_set          = {0, 0, 128, 64, (uint8_t*)"猪舍类型设置", _userUnitTypeSetFunc, NULL};
/** 单元号设置界面：猪舍->单元号 */
const static lkdWin s_win_unit_num_set           = {0, 0, 128, 64, (uint8_t*)"单元设置", _userUnitNumSetFunc, NULL};
/** 单元栏位设置界面：猪舍->栏位 */
const static lkdWin s_win_term_raw_set           = {0, 0, 128, 64, NULL, _userUnitTermRawSetFunc, NULL};

/** 饲喂平台ip和端口设置界面：网络->服务器 */
const static lkdWin s_win_server_info_set        = {0, 0, 128, 64, (uint8_t *)"服务器IP设置", _userServerInfoSetFunc, NULL};
/** 饲喂平台选择界面：网络->平台 */
const static lkdWin s_win_mqtt_select_set        = {0, 0, 128, 64, (uint8_t *)"平台选择", _userMqttSelectSetFunc, NULL};
#if 0
/** 用户升级选择界面：网络->升级 */
const static lkdWin s_win_upgrade_mode_set       = {0, 0, 128, 64, (uint8_t *)"终端升级", _userUpgradeModeSetFunc, NULL};
#endif

/** 通信模式设置界面：通信 */
const static lkdWin s_win_unit_comm_type_set     = {0, 0, 128, 64, (uint8_t *)"通讯模式", _userUnitCommTypeSetFunc , NULL};

/** 饲喂方案下发方式设置界面：方案 */
const static lkdWin s_win_unit_feed_version_set  = {0, 0, 128, 64, (uint8_t *)"饲喂方案下发方式", _userUnitFeedVersionSetFunc, NULL};

/** Lora信号强度测试界面：调试->Lora测试 */
const static lkdWin s_win_lora_test              = {0, 0, 128, 64, (uint8_t *)"Lora信号强度测试", _userLorsTestFunc, NULL};
#if 0
/** 基础参数下发界面：调试->Ba参数 */
const static lkdWin s_win_feed_param_set_default = {0, 0, 128, 64, (uint8_t *)"基础参数下发", _userFeedParamSetDefault, NULL};
/** 调试->Re记录 */
const static lkdWin Gui_Recode               = {0, 0, 128, 64, NULL, Gui_Recode_Fun, NULL};
#endif
/** 调试->Re清空 */
const static lkdWin s_win_term_recode_clear      = {0, 0, 128, 64, NULL, _userTermRecodeClear, NULL}; /*存储区缓存初始化*/

/** 无线->信道 */
const static lkdWin s_win_lora_channel_set       = {0, 0, 128, 64, (uint8_t *)"无线信道设置", _userLoraChannelSetFunc, NULL};
/** 无线->功率 */
const static lkdWin s_win_lora_txpower_set       = {0, 0, 128, 64, (uint8_t *)"无线通信功率", _userLoraTxpowerSetFunc, NULL};
/** 无线->扩频 */
const static lkdWin s_win_lora_sf_set            = {0, 0, 128, 64, (uint8_t *)"无线扩频因子", _userLoraSfSetFunc, NULL};
#if 0
/** 无线->编码率 */
const static lkdWin s_win_lora_cr_set            = {0, 0, 128, 64, (uint8_t *)"无线编码率", _userLoraCrSetFunc, NULL};
/** 无线->带宽 */
const static lkdWin s_win_lora_bw_set            = {0, 0, 128, 64, (uint8_t *)"无线带宽", _userLoraBwSetFunc, NULL};
/** 无线->自动 */
const static lkdWin s_win_lora_ch_adjust_set     = {0, 0, 128, 64, (uint8_t *)"无线信道自动调整开关", _userLoraChAdjustSetFunc, NULL};
/** 无线->执行 */
const static lkdWin s_win_lora_ch_adjust_start   = {0, 0, 128, 64, (uint8_t *)"执行无线信道自动调整", _userLoraChAdjustStartFunc, NULL};
#endif
/** Lora->初始化 */
const static lkdWin s_win_lora_default           = {0, 0, 128, 64, (uint8_t *)"lora设置初始化", _userLoraSetDefaultFunc, NULL};

#if 0
/** 配置->断水 */
const static lkdWin s_win_config_no_water        = {0, 0, 128, 64, (uint8_t *)"断水处理模式", _userConfigNoWaterFunc , NULL};
/** 配置->销售->时长 */
const static lkdWin s_win_sell_time_set          = {0, 0, 128, 64, (uint8_t *)"时长设置", _userConfigSellTimeSetFunc, NULL};
/** 配置->销售->栏位 */
const static lkdWin s_win_sell_switch_set        = {0, 0, 128, 64, (uint8_t *)"栏位设置", _userConfigSellSwitchSetFunc , NULL};
#endif
/**配置->阈值 */
const static lkdWin s_win_probe_adc_set          = {0, 0, 128, 64, (uint8_t *)"二代探针阈值设置", _userConfigProbeAdcSetFunc , NULL};
/** 配置->清槽->开关 */
const static lkdWin s_win_food_clear_switch      = {0, 0, 128, 64, (uint8_t *)"动态清槽开关", _userFoodClearSwitchSetFunc, NULL};
/** 配置->清槽->检测 */
const static lkdWin s_win_food_clear_check_time  = {0, 0, 128, 64, (uint8_t *)"动态清槽检测时间", _userFoodClearCheckTimeSetFunc, NULL};
/** 配置->清槽->清槽 */
const static lkdWin s_win_food_clear_food_time   = {0, 0, 128, 64, (uint8_t *)"动态清槽清槽时间", _userFoodClearFoodTimeSetFunc, NULL};
/** 配置->清槽->饮水 */
const static lkdWin s_win_food_clear_water_time  = {0, 0, 128, 64, (uint8_t *)"动态清槽饮水时间", _userFoodClearWaterTimeSetFunc, NULL};
/** 配置->清槽->始日龄 */
const static lkdWin s_win_food_clear_start_age   = {0, 0, 128, 64, (uint8_t *)"动态清槽起始日龄", _userFoodClearStartAgeSetFunc, NULL};
/** 配置->清槽->终日龄 */
const static lkdWin s_win_food_clear_stop_age    = {0, 0, 128, 64, (uint8_t *)"动态清槽结束日龄", _userFoodClearStopAgeSetFunc, NULL};
#if 0
/** 配置->保护 */
const static lkdWin s_win_protect_time           = {0, 0, 128, 64, (uint8_t *)"保护时间单位设置", _userProtectTimeSetFunc, NULL};
#endif
/** 配置->下水 */
const static lkdWin s_win_water_mode_set         = {0, 0, 128, 64, (uint8_t *)"下水方式设置", _userWaterModeSetFunc, NULL};
/** 配置->探针->灵敏度 */
const static lkdWin s_win_probe_sen_set          = {0, 0, 128, 64, (uint8_t *)"后端盖探针灵敏度", _userProbeSenSetFunc, NULL};
/** 配置->脉冲 */
const static lkdWin s_win_pluse_cal              = {0, 0, 128, 64, (uint8_t *)"自动校水方式", _userProtectPulseCalFunc, NULL};
/** 配置->流速->流速 */
const static lkdWin s_win_Valve_Speed            = {0, 0, 128, 64, (uint8_t *)"一体阀速率", _userValveSpeedSetFunc, NULL};
#if 0
/** 配置->流速->补偿 */
const static lkdWin s_win_water_speed_diff       = {0, 0, 128, 64, (uint8_t *)"差异补偿", _userWaterSpeedDiffSetFunc, NULL};
#endif
/** 配置->强制->下料 */
const static lkdWin s_win_Strong_Food = {0, 0, 128, 64, (uint8_t *)"强制下料设置", _userStrongFoodGearSetFunc, NULL};
/** 配置->强制->下水 */
const static lkdWin s_win_Strong_Water = {0, 0, 128, 64, (uint8_t *)"强制下水设置", _userStrongWaterGearSetFunc, NULL};
/** 配置->料水顺序 */
const static lkdWin s_win_unit_food_mode_set     = {0, 0, 128, 64, (uint8_t *)"饲喂下料方式",_userUnitFoodModeSetFunc , NULL};
/** 配置->泌乳->清槽 */
const static lkdWin s_win_mr_feed_clean_set  = {0, 0, 128, 64, NULL, _userMrFeedCleanSetFunc, NULL};
/** 配置->泌乳->触延 */
const static lkdWin s_win_mr_touch_delay_set = {0, 0, 128, 64, NULL, _userMrTouchDelaySetFunc, NULL};
/** 配置->泌乳->水参数 */
const static lkdWin s_win_mr_feed_water_set  = {0, 0, 128, 64, (uint8_t *)"泌乳下水相关参数", _userMrFeedWaterSetFunc, NULL};
/** 配置->水冲 */
const static lkdWin s_win_water_alluvial_set = {0, 0, 128, 64, (uint8_t *)"水冲积料开关", _userWaterAlluvialSetFunc, NULL};
#if 0
/* 设备->硬件->绞龙 */
const static lkdWin s_win_uint_hw_info_feeder_set      = {0, 0, 128, 64, (uint8_t *)"绞龙类型",   _userUnitHwInfoSetFeeder,    NULL};
#endif
/* 设备->硬件->料槽 */
const static lkdWin s_win_uint_hw_info_trough_set      = {0, 0, 128, 64, (uint8_t *)"料槽类型",   _userUnitHwInfoSetTrough,    NULL};
/* 设备->硬件->探针类型 */
const static lkdWin s_win_uint_hw_info_probe_type_set  = {0, 0, 128, 64, (uint8_t *)"探针类型",   _userUnitHwInfoSetProbeType, NULL};
#if 0
/* 设备->硬件->探针高度 */
const static lkdWin s_win_uint_hw_info_probe_high_set  = {0, 0, 128, 64, (uint8_t *)"探针高度",   _userUnitHwInfoSetProbeHigh, NULL};
/* 设备->硬件->触碰 */
const static lkdWin s_win_uint_hw_info_touch_type_set  = {0, 0, 128, 64, (uint8_t *)"触碰杆类型", _userUnitHwInfoSetTouchType, NULL};
/* 设备->硬件->料距 */
const static lkdWin s_win_uint_hw_info_food_space_set  = {0, 0, 128, 64, (uint8_t *)"料罐间距",   _userUnitHwInfoSetFoodSpace, NULL};
/* 设备->硬件->水距 */
const static lkdWin s_win_uint_hw_info_water_space_set = {0, 0, 128, 64, (uint8_t *)"水源间距",   _userUnitHwInfoSetWaterSpace, NULL};
/* 设备->硬件->电机 */
const static lkdWin s_win_uint_hw_info_motor_type_set  = {0, 0, 128, 64, (uint8_t *)"电机",       _userUnitHwInfoSetMotorType, NULL};
#endif
/* 设备->硬件->一体阀 */
const static lkdWin s_win_uint_hw_info_flow_type_set   = {0, 0, 128, 64, (uint8_t *)"一体阀",     _userUnitHwInfoSetFlowType,  NULL};
/* 模型->配方 */
const static lkdWin s_win_feed_model_recipe_set   = {0, 0, 128, 64, (uint8_t *)"配方设置",   _userFeedModelRecipeSet,  NULL};
/* 模型->含粉 */
const static lkdWin s_win_feed_model_powder_set   = {0, 0, 128, 64, (uint8_t *)"含粉率设置", _userFeedModelPowderSet,  NULL};
/* 模型->温湿度 */
const static lkdWin s_win_feed_model_temphumi_set = {0, 0, 128, 64, (uint8_t *)"温湿度设置", _userFeedModelTempHumiSet,  NULL};
/* 模型->健康 */
const static lkdWin s_win_feed_model_health_set   = {0, 0, 128, 64, (uint8_t *)"健康设置",   _userFeedModelHealthSet,  NULL};
/* 模型->体重 */
const static lkdWin s_win_feed_model_weight_set   = {0, 0, 128, 64, (uint8_t *)"体重设置",   _userFeedModelWeightSet,  NULL};
/* 模型->开关 */
const static lkdWin s_win_feed_model_open_set     = {0, 0, 128, 64, (uint8_t *)"模型开关",   _userFeedModelOpenSet,  NULL};

/** 提示界面 */
const static lkdWin s_win_saving                 = {21, 20, 88, 30, (uint8_t *)"保存中...", NULL, NULL};
const static lkdWin s_win_save_success           = {21, 20, 88, 30, (uint8_t *)"保存成功", NULL, NULL};
const static lkdWin s_win_set_complete           = {21, 20, 88, 30, (uint8_t *)"设置完成", NULL, NULL};
const static lkdWin s_win_set_failed             = {21, 20, 88, 30, (uint8_t *)"设置失败", NULL, NULL};
const static lkdWin s_win_set_ing                = {21, 20, 88, 30, (uint8_t *)"参数设置中...", NULL, NULL};
const static lkdWin s_win_lora_initing           = {21, 20, 88, 30, (uint8_t *)"Lora初始化中...", NULL, NULL};
const static lkdWin s_win_gateway_update         = {10, 5, 108, 54, (uint8_t *)"网关升级勿操作！！", NULL, NULL};

/* 菜单 */
/* 配置->清槽 三级菜单 */
const static lkdMenuNode Node0_9_4_6 = {0x946, (uint8_t *)"终日龄", NULL, NULL, &s_win_food_clear_stop_age};
const static lkdMenuNode Node0_9_4_5 = {0x945, (uint8_t *)"始日龄", &Node0_9_4_6, NULL, &s_win_food_clear_start_age};
const static lkdMenuNode Node0_9_4_4 = {0x944, (uint8_t *)"饮水", &Node0_9_4_5, NULL, &s_win_food_clear_water_time};
const static lkdMenuNode Node0_9_4_3 = {0x943, (uint8_t *)"清槽", &Node0_9_4_4, NULL, &s_win_food_clear_food_time};
const static lkdMenuNode Node0_9_4_2 = {0x942, (uint8_t *)"检测", &Node0_9_4_3, NULL, &s_win_food_clear_check_time};
const static lkdMenuNode Node0_9_4_1 = {0x941, (uint8_t *)"开关", &Node0_9_4_2, NULL, &s_win_food_clear_switch};

/* 配置->销售 三级菜单 */
#if 0
const static lkdMenuNode Node0_9_2_2 = {0x922, (uint8_t *)"栏位", NULL, NULL, &s_win_sell_switch_set};
const static lkdMenuNode Node0_9_2_1 = {0x921, (uint8_t *)"时长", &Node0_9_2_2, NULL, &s_win_sell_time_set};
#endif

/* 配置->探针 三级菜单 */
const static lkdMenuNode Node0_9_7_1 = {0x971, (uint8_t *)"灵敏度", NULL, NULL, &s_win_probe_sen_set};

/* 配置->强制 三级菜单 */
const static lkdMenuNode Node0_9_8_2 = {0x982, (uint8_t *)"下水", NULL, NULL, &s_win_Strong_Water};
const static lkdMenuNode Node0_9_8_1 = {0x981, (uint8_t *)"下料", &Node0_9_8_2, NULL, &s_win_Strong_Food};

/* 配置->流速 三级菜单 */
#if 0
const static lkdMenuNode Node0_9_B_2 = {0x9B2, (uint8_t *)"补偿", NULL, NULL, &s_win_water_speed_diff};
const static lkdMenuNode Node0_9_B_1 = {0x9B1, (uint8_t *)"流速", &Node0_9_B_2, NULL, &s_win_Valve_Speed};
#endif

/* 配置->泌乳 三级菜单 */
const static lkdMenuNode Node0_9_C_3 = {0x9C3, (uint8_t *)"水参数", NULL,         NULL, &s_win_mr_feed_water_set};
const static lkdMenuNode Node0_9_C_2 = {0x9C2, (uint8_t *)"触延",   &Node0_9_C_3, NULL, &s_win_mr_touch_delay_set};
const static lkdMenuNode Node0_9_C_1 = {0x9C1, (uint8_t *)"清槽",   &Node0_9_C_2, NULL, &s_win_mr_feed_clean_set};

/* 设备->硬件 三级菜单 */
#if 0
const static lkdMenuNode Node0_5_3_9 = {0x539, (uint8_t *)"水距",     NULL,         NULL, &s_win_uint_hw_info_water_space_set};
const static lkdMenuNode Node0_5_3_8 = {0x538, (uint8_t *)"料距",     &Node0_5_3_9, NULL, &s_win_uint_hw_info_food_space_set};
const static lkdMenuNode Node0_5_3_7 = {0x537, (uint8_t *)"触碰",     &Node0_5_3_8, NULL, &s_win_uint_hw_info_touch_type_set};
const static lkdMenuNode Node0_5_3_6 = {0x536, (uint8_t *)"探针高度", &Node0_5_3_7, NULL, &s_win_uint_hw_info_probe_high_set};
const static lkdMenuNode Node0_5_3_5 = {0x535, (uint8_t *)"电机",     &Node0_5_3_6, NULL, &s_win_uint_hw_info_motor_type_set};
#endif
const static lkdMenuNode Node0_5_3_4 = {0x534, (uint8_t *)"一体阀",   NULL, NULL, &s_win_uint_hw_info_flow_type_set};
const static lkdMenuNode Node0_5_3_3 = {0x533, (uint8_t *)"探针类型", &Node0_5_3_4, NULL, &s_win_uint_hw_info_probe_type_set};
const static lkdMenuNode Node0_5_3_2 = {0x532, (uint8_t *)"料槽",     &Node0_5_3_3, NULL, &s_win_uint_hw_info_trough_set};
#if 0
const static lkdMenuNode Node0_5_3_1 = {0x531, (uint8_t *)"绞龙",     &Node0_5_3_2, NULL, &s_win_uint_hw_info_feeder_set};

/* 设备->控制板 三级菜单 */
const static lkdMenuNode Node0_5_2_1 = {0x521, (uint8_t *)"下料", NULL, NULL, &s_win_unit_food_mode_set};
#endif
/* 模型 二级菜单 */
const static lkdMenuNode Node0_A_6   = {0xA6, (uint8_t *)"开关",   NULL, NULL, &s_win_feed_model_open_set};
const static lkdMenuNode Node0_A_5   = {0xA5, (uint8_t *)"体重",   &Node0_A_6, NULL, &s_win_feed_model_weight_set};
const static lkdMenuNode Node0_A_4   = {0xA4, (uint8_t *)"健康",   &Node0_A_5, NULL, &s_win_feed_model_health_set};
const static lkdMenuNode Node0_A_3   = {0xA3, (uint8_t *)"温湿度", &Node0_A_4, NULL, &s_win_feed_model_temphumi_set};
const static lkdMenuNode Node0_A_2   = {0xA2, (uint8_t *)"含粉",   &Node0_A_3, NULL, &s_win_feed_model_powder_set};
const static lkdMenuNode Node0_A_1   = {0xA1, (uint8_t *)"配方",   &Node0_A_2, NULL, &s_win_feed_model_recipe_set};

/* 配置 二级菜单 */
const static lkdMenuNode Node0_9_D   = {0x9D, (uint8_t *)"水冲", NULL,       NULL, &s_win_water_alluvial_set};
const static lkdMenuNode Node0_9_C   = {0x9C, (uint8_t *)"泌乳", &Node0_9_D, &Node0_9_C_1, NULL};
#if 0
const static lkdMenuNode Node0_9_B   = {0x9B, (uint8_t *)"流速", &Node0_9_C, &Node0_9_B_1, NULL};
#endif
const static lkdMenuNode Node0_9_B   = {0x9B, (uint8_t *)"流速", &Node0_9_C, NULL, &s_win_Valve_Speed};
const static lkdMenuNode Node0_9_A   = {0x9A, (uint8_t *)"料水顺序", &Node0_9_B, NULL, &s_win_unit_food_mode_set};
const static lkdMenuNode Node0_9_9   = {0x99, (uint8_t *)"校水", &Node0_9_A, NULL, &s_win_pluse_cal};
const static lkdMenuNode Node0_9_8   = {0x98, (uint8_t *)"强制", &Node0_9_9, &Node0_9_8_1, NULL};
const static lkdMenuNode Node0_9_7   = {0x97, (uint8_t *)"探针", &Node0_9_8, &Node0_9_7_1, NULL};
const static lkdMenuNode Node0_9_6   = {0x96, (uint8_t *)"小水壶", &Node0_9_7, NULL, &s_win_water_mode_set};
#if 0
const static lkdMenuNode Node0_9_5   = {0x95, (uint8_t *)"保护", &Node0_9_6, NULL, &s_win_protect_time};
#endif
const static lkdMenuNode Node0_9_4   = {0x94, (uint8_t *)"清槽", &Node0_9_6, &Node0_9_4_1, NULL};
const static lkdMenuNode Node0_9_3   = {0x93, (uint8_t *)"阈值", &Node0_9_4, NULL, &s_win_probe_adc_set};
#if 0
const static lkdMenuNode Node0_9_2   = {0x92, (uint8_t *)"销售", &Node0_9_3, &Node0_9_2_1, NULL};
const static lkdMenuNode Node0_9_1   = {0x91, (uint8_t *)"断水", &Node0_9_2, NULL, &s_win_config_no_water};
#endif

/* lora二级菜单 */
#if 0
const static lkdMenuNode Node0_8_3 = {0x83, (uint8_t *)"初始化", NULL, NULL, &s_win_lora_default};
const static lkdMenuNode Node0_8_2 = {0x82, (uint8_t *)"强度", &Node0_8_3, NULL, &s_win_lora_rssi};
const static lkdMenuNode Node0_8_1 = {0x81, (uint8_t *)"通信", &Node0_8_2, NULL, &s_win_comm_info};
#endif

/* 无线二级菜单 */
#if 0
const static lkdMenuNode Node0_7_7 = {0x77, (uint8_t *)"执行", NULL, NULL, &s_win_lora_ch_adjust_start};
const static lkdMenuNode Node0_7_6 = {0x76, (uint8_t *)"自动", &Node0_7_7, NULL, &s_win_lora_ch_adjust_set};
const static lkdMenuNode Node0_7_5 = {0x75, (uint8_t *)"带宽", &Node0_7_6, NULL, &s_win_lora_bw_set};
const static lkdMenuNode Node0_7_4 = {0x74, (uint8_t *)"编码率", &Node0_7_5, NULL, &s_win_lora_cr_set};
#endif
const static lkdMenuNode Node0_7_4 = {0x74, (uint8_t *)"初始化", NULL, NULL, &s_win_lora_default};
const static lkdMenuNode Node0_7_3 = {0x73, (uint8_t *)"扩频", &Node0_7_4, NULL, &s_win_lora_sf_set};
const static lkdMenuNode Node0_7_2 = {0x72, (uint8_t *)"功率", &Node0_7_3, NULL, &s_win_lora_txpower_set};
const static lkdMenuNode Node0_7_1 = {0x71, (uint8_t *)"信道", &Node0_7_2, NULL, &s_win_lora_channel_set};

/* 调试 二级菜单 */
const static lkdMenuNode Node0_6_6 = {0x66, (uint8_t *)"Re清空", NULL, NULL, &s_win_term_recode_clear};
#if 0
const static lkdMenuNode Node0_6_5 = {0x65, (uint8_t *)"Re记录", &Node0_6_6, NULL, &Gui_Recode};
#endif
const static lkdMenuNode Node0_6_4 = {0x64, (uint8_t *)"注册", &Node0_6_6, NULL, &s_win_lora_reg};
#if 0
const static lkdMenuNode Node0_6_3 = {0x63, (uint8_t *)"Ba参数", &Node0_6_4, NULL, &s_win_feed_param_set_default};
const static lkdMenuNode Node0_6_2 = {0x62, (uint8_t *)"Ba设置", &Node0_6_3, NULL, NULL};
#endif
const static lkdMenuNode Node0_6_1 = {0x61, (uint8_t *)"Lora测试", &Node0_6_4, NULL, &s_win_lora_test};

/* 设备 二级菜单 */
const static lkdMenuNode Node0_5_3 = {0x53, (uint8_t *)"硬件",   NULL,       &Node0_5_3_2, NULL};
const static lkdMenuNode Node0_5_2 = {0x52, (uint8_t *)"控制板", &Node0_5_3, NULL, NULL};
const static lkdMenuNode Node0_5_1 = {0x51, (uint8_t *)"通信板", &Node0_5_2, NULL, NULL};

/* 网络 二级菜单 */
#if 0
const static lkdMenuNode Node0_2_3 = {0x23, (uint8_t *)"升级", NULL, NULL, &s_win_upgrade_mode_set};
#endif
const static lkdMenuNode Node0_2_2 = {0x22, (uint8_t *)"平台", NULL, NULL, &s_win_mqtt_select_set};
const static lkdMenuNode Node0_2_1 = {0x21, (uint8_t *)"服务器", &Node0_2_2, NULL, &s_win_server_info_set};

/* 猪舍 二级菜单 */
const static lkdMenuNode Node0_1_3 = {0x13, (uint8_t *)"栏位", NULL, NULL, &s_win_term_raw_set};
const static lkdMenuNode Node0_1_2 = {0x12, (uint8_t *)"单元号", &Node0_1_3, NULL, &s_win_unit_num_set};
const static lkdMenuNode Node0_1_1 = {0x11, (uint8_t *)"类型", &Node0_1_2, NULL, &s_win_unit_type_set};

/* 一级菜单 */
const static lkdMenuNode Node0_A = {0xA, (uint8_t *)"模型", NULL, &Node0_A_1, NULL};
const static lkdMenuNode Node0_9 = {0x9, (uint8_t *)"配置", &Node0_A, &Node0_9_3, NULL};
// const static lkdMenuNode Node0_8 = {0x8, (uint8_t *)"Lora", &Node0_9, &Node0_8_1, NULL};
const static lkdMenuNode Node0_7 = {0x7, (uint8_t *)"无线", &Node0_9, &Node0_7_1, NULL};
const static lkdMenuNode Node0_6 = {0x6, (uint8_t *)"调试", &Node0_7, &Node0_6_1, NULL};
const static lkdMenuNode Node0_5 = {0x5, (uint8_t *)"设备", &Node0_6, &Node0_5_1, NULL};
const static lkdMenuNode Node0_4 = {0x4, (uint8_t *)"方案", &Node0_5, NULL, &s_win_unit_feed_version_set};
const static lkdMenuNode Node0_3 = {0x3, (uint8_t *)"通信", &Node0_4, NULL, &s_win_unit_comm_type_set};
const static lkdMenuNode Node0_2 = {0x2, (uint8_t *)"网络", &Node0_3, &Node0_2_1, NULL};
const static lkdMenuNode Node0_1 = {0x1, (uint8_t *)"猪舍", &Node0_2, &Node0_1_1, NULL};
/* 根菜单 */
const static lkdMenuNode Node0 = {0x0, (uint8_t *)"root",  NULL,       &Node0_1,      NULL};

/************************************************/
/*                 对内函数实现                  */
/************************************************/
/*菜单项处理*/
static void MenuItemDealWith(const lkdMenuNode *pNode)
{
    if(pNode->pSon != NULL){/*展开选中节点的菜单*/
        if(pNode->user == NULL)
        {
            GuiMenuCurrentNodeSonUnfold(&HmiMenu);
        }
        else
        {
            DrawNodes(&HmiMenu, HmiMenu.pDraw, HmiMenu.cx, HmiMenu.cy, HmiMenu.index, HmiMenu.cNum);
        }
    }
    else if(pNode->user != NULL){/*打开菜单对应的窗口*/
        GuiWinAdd(pNode->user);
    }
}

/*菜单控制函数*/
static void MenuControlFun(void)
{
    switch(s_i_key_status)
    {
        case KEY_UP_DOWN:GuiMenuItemUpMove(&HmiMenu);break; /*button_up_down*/
        case KEY_DOWN_DOWN:GuiMenuItemDownMove(&HmiMenu);break;/*button_down_down*/
        case KEY_LEFT_DOWN:
        case KEY_CANCEL_DOWN:
            GuiMenuCurrentNodeHide(&HmiMenu);
            if(HmiMenu.count == 0)/*检测到菜单退出信号*/
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

/*开机进度条显示*/
static void _guiPowerOnProgress(void)
{
    lkdProgress tProGress;
    tProGress.x = 21;
    tProGress.y = 35;
    tProGress.wide = 80;
    tProGress.high = 5;
    tProGress.ratio = 0;

    for(uint8_t i = 0; i < 100; i ++){
        tProGress.ratio = i;        /* 进度控制 */
        GuiProGress(&tProGress);    /* 画进度条 */
        GuiRowTextPlus(0, 3, 127, FONT_MID, (uint8_t *)"loading...");
        GuiUpdateDisplayAll();/* 更新 */

        /* 这个地方最长可能达到为48秒, 通讯板初次上电且栏位数量为120栏位时, 时间可能达到48秒左右;
           但是不影响, 进度条加载完后可以白屏等一会，第一次上电通常是26秒；后续一般在11秒左右 */
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

/*菜单初始化*/
static void UserMenuInit(void)
{
    HmiMenu.x = 1;
    HmiMenu.y = 1;
    HmiMenu.wide = 126;     //@ 这个是否与 GUIXMAX 相关
    HmiMenu.hight = 62;     //@ 这个是否与 GUIYMAX 相关
    HmiMenu.Itemshigh = 15;/*菜单条目高度*/
    HmiMenu.ItemsWide = 42;/*菜单条目宽度*/
    HmiMenu.stack = userMenuStack;
    HmiMenu.stackNum = 8;   //@ 与 GUIWINMANAGE_NUM 相关，旧网关是9
    HmiMenu.Root = &Node0;/* 菜单句柄跟菜单节点绑定到一起 */
    HmiMenu.MenuDealWithFun = MenuControlFun;/* 菜单控制回调函数 */
    GuiMenuInit(&HmiMenu);
}

/* 进入注册界面 */
static void _userWinRemoteReg(void)
{
    if (paramsCheckTermCommMode(E_TERM_COMM_MODE_REG) != 1)
    {
        GuiWinAdd(&s_win_lora_reg);
    }
}

/* 字库下载进度显示 */
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

    GuiProGress(&tProGress); /* 画进度条 */
    MY_SNPRINTF(data, sizeof(data), "Loading font [%03d]", tProGress.ratio);
    GuiRowTextPlus(0, 1, 128, FONT_MID, (uint8_t *)data);
    GuiUpdateDisplayAll(); /* 更新 */
}

/* 主界面 */
static void _userWinHomeFunc(void *param)
{
    const char* const comm_type[] = {"无线", "有线"};

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

    /* 显示日期和时间 */
    timestamp = paramsGetGwTimestamp();
    platTimestampToDateTime(timestamp, &t_plat_time);
    MY_SNPRINTF(str, sizeof(str), "%04d-%02d-%02d %02d:%02d:%02d", t_plat_time.uh_year, t_plat_time.uc_mon,\
                t_plat_time.uc_day, t_plat_time.uc_hour, t_plat_time.uc_min, t_plat_time.uc_sec);
    GuiRowTextPlus(2, 0, 128, FONT_LEFT, (uint8_t *)str);

    /* 显示猪舍类型和通信类型 */
    if (paramsGetGwUnitCommType() == GW_TERM_COM_TYPE_RS485)
    {
        comm_type_index = 1;
    }
    unit_type = paramsGetGwUnitType();
    unit_num  = paramsGwUnitNumGet();
    if (unit_type == Unit_Type_NO_TYPE)
    {
        MY_SNPRINTF(str, sizeof(str), "未选择:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_MR)
    {
        MY_SNPRINTF(str, sizeof(str), "泌乳舍:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_HY)
    {
        MY_SNPRINTF(str, sizeof(str), "怀孕舍:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_BY)
    {
        MY_SNPRINTF(str, sizeof(str), "保育舍:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_YF)
    {
        MY_SNPRINTF(str, sizeof(str), "育肥舍:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    else if (unit_type == Unit_Type_BY_YF)
    {
        MY_SNPRINTF(str, sizeof(str), "一体舍:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
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
        MY_SNPRINTF(str, sizeof(str), "未选择:%02d|通信:%s ", unit_num, comm_type[comm_type_index]);
    }
    GuiRowTextPlus(2, 1, 128, FONT_LEFT, (uint8_t *)str);

    /* 显示IP地址*/
    basicHalEtHIpv4Get(net_ipv4);
    MY_SNPRINTF(str, sizeof(str), "本地IP:%d.%d.%d.%d", net_ipv4[0], net_ipv4[1], net_ipv4[2], net_ipv4[3]);
    GuiRowTextPlus(2, 2, 128, FONT_LEFT, (uint8_t *)str);

    /* 无线：显示版本和信道；有线：显示版本和启动次数*/
    if (comm_type_index == 1)   /* 有线 */
    {
        uint32_t start_cnt = paramsGetGwStartCnt();
#if (GW_APP_FW > 999999)
        MY_SNPRINTF(str, sizeof(str), "V%d.%d.%02d.%02d.%d启动:%3d", GW_APP_FW/1000000, (GW_APP_FW/100000)%10, (GW_APP_FW/1000)%100, (GW_APP_FW/10)%100, (GW_APP_FW%10), start_cnt);
#elif (GW_APP_FW > 9999)
        MY_SNPRINTF(str, sizeof(str), "V%02d.%02d.%02d 启动:%3d", GW_APP_FW/10000, (GW_APP_FW/100)%100, GW_APP_FW%100, start_cnt);
#else
        MY_SNPRINTF(str, sizeof(str), "版本:%d.%d.%02d启动:%3d", GW_APP_FW/1000, (GW_APP_FW/100)%10, GW_APP_FW%100, start_cnt);
#endif
    }
    else    /* 无线 */
    {
#if (GW_APP_FW > 999999)
        MY_SNPRINTF(str, sizeof(str), "V%d.%d.%02d.%02d.%d 信道:%3d", GW_APP_FW/1000000, (GW_APP_FW/100000)%10, (GW_APP_FW/1000)%100, (GW_APP_FW/10)%100, (GW_APP_FW%10), paramsLoraGetChannel());
#elif (GW_APP_FW > 9999)
        MY_SNPRINTF(str, sizeof(str), "V%02d.%02d.%02d 信道:%3d", GW_APP_FW/10000, (GW_APP_FW/100)%100, GW_APP_FW%100, paramsLoraGetChannel());
#else
        MY_SNPRINTF(str, sizeof(str), "版本:%d.%d.%02d 信道:%3d", GW_APP_FW/1000, (GW_APP_FW/100)%10, GW_APP_FW%100, paramsLoraGetChannel());
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

/* 注册窗口服务函数 */
static void _userWinLoraRegFunc(void *param)
{

/* lora组网超时时间，单位ms; 这里设置30分钟 */
#define LORA_INNET_TIMEOUT  (1800000)

    static uint8_t first_enter_mode = 0;

    uint32_t cur_tick = MY_TICK_GET();

    if (first_enter_flag == 0)
    {
        paramsSetTermCommMode(E_TERM_COMM_MODE_REG);

        midLoraInNet();     //切换到注册信道，以及注册组网ID
        // LOG_I("[%s] LoRa in register mode!", __func__);

        GuiRowTextPlus(5, 1, 120, FONT_LEFT, (uint8_t *)"注册模式已经启动，");
        GuiRowTextPlus(5, 2, 120, FONT_LEFT, (uint8_t *)"请打开控制板注册功能");
        GuiUpdateDisplayAll();

        s_gui_wait_last_tick = cur_tick;
        first_enter_mode = paramsLoraGetRemoteRegFlag();

        first_enter_flag = 1;
    }

    /* 两个条件满足一个就退出：1-超时退出；2-是由远程标志进入，远程标志取消则退出 */
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

    midLoraBackToCom(); //切换回工作信道，以及工作ID
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
    // LOG_I("[lora] LoRa out register mode!");
}

/* 二维码窗口服务函数 */
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

/* 网关与终端通信成功率窗口服务函数 */
static void _userCommSuccRateFunc(void *param)
{
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];
    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page = (term_num+2)/3;  /* 当不足一页是添加一页 */
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


/* 网关与Lora终端通信信号强度窗口服务函数 */
static void _userLoraRssiFunc(void *param)
{
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];
    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page = (term_num+2)/3;  /* 当不足一页是添加一页 */
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

/* 地址冲突窗口服务函数 */
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
            MY_SNPRINTF(str, sizeof(str), "一排 %2d 地址冲突", line_addr);
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "二排 %2d 地址冲突", line_addr);
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "三排 %2d 地址冲突", line_addr);
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "四排 %2d 地址冲突", line_addr);
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

/* 菜单界面服务函数 */
static void _userMenuFun(void *param)
{
    /*用户自定义代码*/
    if(HmiMenu.count == 0)
    {
        GuiMenuCurrentNodeSonUnfold(&HmiMenu);
    }
    HmiMenu.MenuDealWithFun();
}

/* 单元类型设置服务函数 */
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
        case KEY_OK_DOWN:   /* ok键在后面特殊处理 */
        default:
            break;
    }

    switch(unit_type_index)
    {
        case 1:
            unit_type = Unit_Type_MR;
            MY_SNPRINTF(str, sizeof(str), "%s", "泌乳舍");
            break;
        case 2:
            unit_type = Unit_Type_HY;
            MY_SNPRINTF(str, sizeof(str), "%s", "怀孕舍");
            break;
        case 3:
            unit_type = Unit_Type_BY;
            MY_SNPRINTF(str, sizeof(str), "%s", "保育舍");
            break;
        case 4:
            unit_type = Unit_Type_YF;
            MY_SNPRINTF(str, sizeof(str), "%s", "育肥舍");
            break;
        case 5:
            unit_type = Unit_Type_BY_YF;
            MY_SNPRINTF(str, sizeof(str), "%s", "一体舍");
            break;
        default:
            unit_type = Unit_Type_NO_TYPE;
            unit_type_index = 0;
            MY_SNPRINTF(str, sizeof(str), "%s", "未选择");
            break;
    }

    if (s_i_key_status == KEY_OK_DOWN)  goto key_ok;

    GuiRowTextPlus(16, 2, 127, FONT_LEFT, (uint8_t *)"猪舍类型");

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
        //@ 提示保存失败
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

/* 单元号设置服务函数 */
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
    GuiRowTextPlus(20, 2, 128, FONT_LEFT, (uint8_t *)"单元号");

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
            //@ 提示保存失败
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

/* 单元栏位设置服务函数 */
static void _userUnitTermRawSetFunc(void *param)
{
    //用户应用代码:画图等
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

    GuiRowTextPlus(5, 0, 40, FONT_MID, (uint8_t *)"第1排");
    GuiRowTextPlus(5, 1, 40, FONT_MID, (uint8_t *)"第2排");
    GuiRowTextPlus(5, 2, 40, FONT_MID, (uint8_t *)"第3排");
    GuiRowTextPlus(5, 3, 40, FONT_MID, (uint8_t *)"第4排");
    for (int i = 0; i < 4; i++)
    {
        MY_SNPRINTF(str, sizeof(str), "【%2d】", gw_layout.term_layout[i]);
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
        //@ 提示失败信息
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

/* 饲喂平台ip和端口设置界面服务函数 */
static void _userServerInfoSetFunc(void *param)
{
    static uint8_t index = 0;   /* 对应server_info_str中字节的索引 */
    static uint8_t server_info_str[16] = {0};   /* 保存ip和端口的每一位 */

    uint8_t server_ipv4[4];
    uint16_t server_port;

    uint8_t cur_max = 9;    /* 默认可以设置 0~9 */

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

    /* 重置ip地址设置每位最大值 */ //@ 这里后续修改为超过255则重新设置为255
    if (index < 12)
    {
        if ((index%3) == 0)
        {
            cur_max = 2;    /* ip字节的百位范围位 0~2 */
        }
        else if (((index%3) == 1)&&(server_info_str[index-1] == 2))
        {
            cur_max = 5;    /* 当ip字节的百位为2时，十位范围为 0~5 */
        }
        else if (((index%3) == 2)&&(server_info_str[index-2] == 2)&&(server_info_str[index-1] == 5))
        {
            cur_max = 5;    /* 当ip字节的百位为2且十位为5时，个位范围为 0~5 */
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
        /* 这里如果成功则不需要重启 */
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

/* 饲喂平台选择界面服务函数 */
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

    GuiRowTextPlus(5, 2, 50, FONT_RIGHT, (uint8_t *)"平台：");
    GuiSetbackcolor(CBLACK);
    if (mqtt_flag != 0)
    {
        GuiRowTextPlus(56, 2, 50, FONT_LEFT, (uint8_t *)"总部");
    }
    else
    {
        GuiRowTextPlus(56, 2, 50, FONT_LEFT, (uint8_t *)"场次");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_save_success);
    if (s_gui_saving == 1)
    {
        s_gui_saving = 0;
        /* 这里如果成功则不需要重启 */
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
/* 用户升级选择界面服务函数 */
static void _userUpgradeModeSetFunc(void *param)
{
    static uint8_t upgrade_mode = 0;    /* 0-广播; 1-单播(点对点) */

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
            // ret = 这里使用设置的方式来启动升级，0-表示设置成功；1-表示设置失败
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

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"升级方式：");
    GuiSetbackcolor(CBLACK);
    if (upgrade_mode != 0)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"单播");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"广播");
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

/* 单元通信类型设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"通讯方式：");
    GuiSetbackcolor(CBLACK);
    if (unit_comm_type != GW_TERM_COM_TYPE_RS485)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"无线");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"有线");
    }
    GuiSetbackcolor(CWHITLE);
    GuiUpdateDisplayAll();
    return;
saved:
    GuiWinDraw(&s_win_set_complete);
    if (s_gui_saving == 1)
    {
       //@ 这里需要切换与终端通信方式，设置相应事件,去掉后面的重启
       s_gui_saving = 0;
    }
    basicHalReboot(10);   //@ 后续重启需要去掉
    s_gui_wait_flag = 1;
    s_gui_wait_last_tick = MY_TICK_GET();
    return;
exit:
    s_gui_wait_flag = 0;
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

/* 饲喂方案下发方式设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"饲喂模式：");
    GuiSetbackcolor(CBLACK);
    if (unit_fee_version == 1)
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"本地");
    }
    else
    {
        GuiRowTextPlus(82, 2, 46, FONT_LEFT, (uint8_t *)"后台");
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

/* 单元下料方式设置界面服务函数*/
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

    GuiRowTextPlus(2, 2, 126, FONT_LEFT, (uint8_t *)"下料方式:");
    GuiSetbackcolor(CBLACK);
    if (food_mode == 1)
    {
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"先下水后下料");
    }
    else if (food_mode == 2)
    {
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"先下料后下水");
    }
    else
    {
        food_mode = 0;
        GuiRowTextPlus(0, 2, 126, FONT_RIGHT, (uint8_t *)"同时下料下水");
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

/* lora信号强度测试界面服务函数 */
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
        midLoraInNet();    //切换到注册信道，以及注册组网ID，便于无需注册即可测试信号强度
        paramsSetTermCommMode(E_TERM_COMM_MODE_TEST);   /* lora通信协议切换到信号测试模式,先切换信道再设置模式 */
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
            paramsResetTermCommMode(E_TERM_COMM_MODE_TEST);     /* 复位测试模式标志 */
            midLoraBackToCom(); //切换到正常工作信道，以及回复组网ID
            LOG_I("[lora] LoRa out test mode!");
            goto exit;
            break;
        default:
            break;
    }

    MY_SNPRINTF(str, sizeof(str), "[01]G：%04d,T：%04d", Rssi_Gateway, Rssi_term);
    GuiRowTextPlus(2, 2, 120, FONT_LEFT, (uint8_t *)str);
    GuiUpdateDisplayAll();
    return;
exit:
    first_enter_flag = 0;
    GuiWinDeleteTop();
    GuiMenuRedrawMenu(&HmiMenu);
}

#if 0
/* 饲喂参数重置并下发界面服务函数 */
static void _userFeedParamSetDefault(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (s_gui_saving == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "亲按确认开始！");
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
/* 本地饲喂记录清空界面 */
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
        MY_SNPRINTF(str, sizeof(str), "是否清空本地饲喂记录？");

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
                MY_SNPRINTF(str, sizeof(str), "清空本地饲喂记录中...");
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

/* 无线信道设置界面服务函数 */
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

            /* 避免正处于信道调整阶段 */
            ch_adjust_run_flag = paramsLoraChAdjustRunFlagGet();
            paramsLoraChAdjustResetFlagSet(1);
            if (ch_adjust_run_flag != 0)
            {
                midLoraBackToCom();
            }

            appTermEventLoraParamSet(s_gui_value, paramsLoraGetNetId());
            s_gui_saving = 1;   /* 表示正在设置信道 */
            s_gui_value_0 = 0;  /* 计数, 设置信道超时计数 */
            s_gui_wait_last_tick = MY_TICK_GET();
            return;
        default:
            break;
    }
    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"无线信道：");
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

/* 无线发送功率设置界面服务函数 */
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
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"无线发射功率：");
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

/* 无线扩频因子设置界面服务函数 */
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
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"无线扩频因子：");
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
/* 无线编码率设置界面服务函数 */
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
    GuiRowTextPlus(0, 2, 100, FONT_RIGHT, (uint8_t *)"无线编码率：");
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

/* 无线带宽设置界面服务函数 */
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
    GuiRowTextPlus(0, 2, 82, FONT_RIGHT, (uint8_t *)"无线带宽：");
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

/** 无线信道自动调整开关设置界面服务函数 */
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
    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"自动调整开关：");
    GuiSetbackcolor(CBLACK);
    if(s_gui_value == 0x01)
    {
        GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)" 开 ");
    }
    else
    {
        GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)" 关 ");
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

/** 无线信道自动调整开始界面服务函数 */
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

    /* 超时10分钟或者信道调整完成则正常退出 */
    if ((delay_s > 600)||
        ((delay_s > 3)&&(paramsLoraChAdjustRunFlagGet() == 0)))
    {
        GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"信道调整完成     ");
        MY_SNPRINTF(str, sizeof(str), "调整后信道: %3d  ", paramsLoraGetChannel());
        GuiRowTextPlus(0, 3, 128, FONT_MID, (uint8_t *)str);
        GuiUpdateDisplayAll();
        s_gui_wait_flag = 1;
        s_gui_wait_last_tick = cur_tick;
        return;
    }

    if (delay_s <= 300)         delay_s = 300 -delay_s;
    else                        delay_s = 0;

    GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"正在进行信道调整,");
    rt_snprintf(str, sizeof(str), "请耐心等待 %3d 秒", delay_s);
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
/* Lora通信参数初始化界面服务函数 */
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
/* 配置断水模式界面服务函数 */
static void _userConfigNoWaterFunc(void *param)
{
    GUI_WAIT_TIMEOUT_CHECK();

    if(first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigNoWaterModeGet();   // 0-打开，1-关闭
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

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"断水处理模式：");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value == 1)
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"打开");
    }
    else
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"关闭");
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
  *@brief  一体阀速率设置界面
  *@param  None
  *@retval None
  */
static void _userValveSpeedSetFunc(void *param)
{
    //用户应用代码:画图等
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterSpeedGet();
        first_enter_flag = 1;
    }

    /* 按键状态处理 */
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
    GuiRowTextPlus(7, 2, 114, FONT_LEFT, (uint8_t *)"一体阀速率：");

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
/* 流速补偿设置界面服务函数 */
static void _userWaterSpeedDiffSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterSpeedDiffGet();
        first_enter_flag = 1;
    }

    /* 按键状态处理 */
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
    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"差异补偿：");
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

/* 配置销售模式时长设置界面服务函数 */
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
            if (s_gui_value != paramsSellDurationTimeGet())   // 0-打开，1-关闭
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

    GuiRowTextPlus(0, 2, 73, FONT_RIGHT, (uint8_t *)"销售时长：");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value > 24)
    {
        MY_SNPRINTF(str, sizeof(str), "30 分钟");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "%2d 小时", s_gui_value);
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

/* 栏位销售模式开关设置界面服务函数 */
static void _userConfigSellSwitchSetFunc(void *param)
{
    /* s_gui_value 在这里作为选中的栏位游标 */
    static uint32_t s_gui_sell_status[(GW_TERM_NUM_MAX+31)>>5];
    static uint8_t cur_page = 0;

    char str[GUI_ONE_LINE_MAX_SIZE];

    uint16_t term_num = paramsGetGwTermNum();
    uint8_t max_page  = (term_num+2)/3;  /* 当不足一页是添加一页 */

    uint8_t cur_page_start = cur_page*3;
    uint8_t cur_line = 0;

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        MY_MEMSET(s_gui_sell_status, 0, sizeof(s_gui_sell_status));
        for (int i = 0; i < term_num; i++)
        {
            // if (sell_mode[i].sell_mode_switch == 1) //判断销售模式是否打开
            if (paramsSellModeSwitchStatusGet(i+1) == 1) //应该这样改？  spy 0509
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
#if 0//保存设置的值
            for(int i = 1;i <= paramsGetGwTermNum();i++)
            {
                if(paramsSellModeSwitchStatusGet(i) != pig_sell_status[i])    // 这里怎么修改？  spy 0509
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
                MY_SNPRINTF(str, sizeof(str), "打开");
            }
            else
            {
                MY_SNPRINTF(str, sizeof(str), "关闭");
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

/* 探针阈值设置界面服务函数 */
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
    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"探针阈值：");
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

/* 动态清槽开关设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"动态清槽开关：");
    GuiSetbackcolor(CBLACK);
    if (s_gui_value == 1)
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"打开");
    }
    else
    {
        GuiRowTextPlus(94, 2, 34, FONT_LEFT, (uint8_t *)"关闭");
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

/* 动态清槽检查时间设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"检测时间：");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d 分钟", s_gui_value);
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

/* 动态清槽时间设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"清槽时间：");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d 分钟", s_gui_value);
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

/* 动态清水时间设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"下水时间：");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d 分钟", s_gui_value);
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

/* 动态清槽起始日龄设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 70, FONT_RIGHT, (uint8_t *)"起始日龄：");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d 日龄", s_gui_value);
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

/* 动态清槽结束日龄设置界面服务函数 */
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

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"截至日龄：");
    GuiSetbackcolor(CBLACK);
    MY_SNPRINTF(str, sizeof(str), "%3d 日龄", s_gui_value);
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
/* 保护时间设置界面服务函数 */
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

    GuiRowTextPlus(0, 2, 94, FONT_RIGHT, (uint8_t *)"保护时间单位：");
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
/* 下水模式设置界面服务函数 */
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

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"下水模式：");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case 0:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"一体阀式");
            break;
        case 1:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"小水壶式");
            break;
        default:
            GuiRowTextPlus(10, 2, 108, FONT_RIGHT, (uint8_t *)"两者共存");
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

/* 后端盖探针灵敏度设置界面服务函数 */
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

    GuiRowTextPlus(19, 2, 90, FONT_LEFT, (uint8_t *)"探针灵敏度：");
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
  *@brief  强制下料等级设置实体函数
  *@param  None
  *@retval None
  */
static void _userStrongFoodGearSetFunc(void *param)
{
    //用户应用代码:画图等

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
            /* 存储 */
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
            MY_SNPRINTF(str, sizeof(str), "强制下料关闭");
            break;
        case 1:
            MY_SNPRINTF(str, sizeof(str), "强制下料100g");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "强制下料200g");
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "强制下料300g");
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "强制下料400g");
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
  *@brief  增强下水等级设置实体函数
  *@param  None
  *@retval None
  */
static void _userStrongWaterGearSetFunc(void *param)
{
    //用户应用代码:画图等
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
            MY_SNPRINTF(str, sizeof(str), "强制下水关闭 ");
            break;
        case 1:
            MY_SNPRINTF(str, sizeof(str), "强制下水0.5倍");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "强制下水 1 倍");
            break;
        case 3:
            MY_SNPRINTF(str, sizeof(str), "强制下水1.5倍");
            break;
        case 4:
            MY_SNPRINTF(str, sizeof(str), "强制下水 2 倍");
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
  *@brief  脉冲校准模式设置界面
  *@param  None
  *@retval None
  */
static void _userProtectPulseCalFunc(void *param)
{
    //用户应用代码:画图等
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterPulseModeGet();
        first_enter_flag = 1;
    }

    /* 按键状态处理 */
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
        MY_SNPRINTF(str, sizeof(str), "打开");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "关闭");
    }

    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"自动校水：");
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

/** 泌乳清槽时间设置 */
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

            if (s_gui_value_3 != 0)   /* 所有顿数设置为一样的参数 */
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

    /* 设置开关 */
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(6, 0, 127, FONT_LEFT, (uint8_t *)"开关:");
    if (s_gui_value_0 == 0)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (pt_mr_config->fc_param.fc_switch == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "关");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "开");
    }
    GuiRowTextPlus(35, 0, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置每顿相同标志 */
    GuiRowTextPlus(60, 0, 127, FONT_LEFT, (uint8_t *)"每顿相同:");
    if (s_gui_value_0 == 1)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (s_gui_value_3 == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "关");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "开");
    }
    GuiRowTextPlus(115, 0, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置顿数 */
    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"顿数:");
    if(s_gui_value_0 == 2)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%2d", (s_gui_value_2+1));
    GuiRowTextPlus(35, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置次序 */
    GuiRowTextPlus(60, 1, 127, FONT_LEFT, (uint8_t *)"次序:");
    if(s_gui_value_0 == 3)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%2d", (s_gui_value_1+1));
    GuiRowTextPlus(90, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置饲喂时间 */
    GuiRowTextPlus(6, 2, 127, FONT_LEFT, (uint8_t *)"饲喂时间:");
    if(s_gui_value_0 == 4)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%3d分钟", pt_mr_config->fc_param.fc_meals[s_gui_value_2].feed_time[s_gui_value_1]);
    GuiRowTextPlus(60, 2, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置清槽时间 */
    GuiRowTextPlus(6, 3, 127, FONT_LEFT, (uint8_t *)"清槽时间:");
    if(s_gui_value_0 == 5)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%3d分钟", pt_mr_config->fc_param.fc_meals[s_gui_value_2].clean_time[s_gui_value_1]);
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
/** 泌乳触碰延时时间设置 */
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

    /* 按键状态处理 */
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

    GuiRowTextPlus(13, 2, 102, FONT_LEFT, (uint8_t *)"触碰延后时间:");
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
/** 泌乳水参数设置 */
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

    /* 设置水参数开关 */
    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"补水开关:");
    if(s_gui_value_0 == 0)
    {
        GuiSetbackcolor(CBLACK);
    }
    if (s_gui_value_1 == 0)
    {
        MY_SNPRINTF(str, sizeof(str), "关");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "开");
    }
    GuiRowTextPlus(60, 1, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置刺激下水量 */
    GuiRowTextPlus(6, 2, 127, FONT_LEFT, (uint8_t *)"刺激下水:");
    if(s_gui_value_0 == 1)
    {
        GuiSetbackcolor(CBLACK);
    }
    MY_SNPRINTF(str, sizeof(str), "%4dg", s_gui_value_2*100);
    GuiRowTextPlus(60, 2, 127, FONT_LEFT, (uint8_t *)str);
    GuiSetbackcolor(CWHITLE);

    /* 设置单次下水量 */
    GuiRowTextPlus(6, 3, 127, FONT_LEFT, (uint8_t *)"单次下水:");
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

/** 水冲积料开关设置 */
static void _userWaterAlluvialSetFunc(void *param)
{
    char str[GUI_ONE_LINE_MAX_SIZE];

    GUI_WAIT_TIMEOUT_CHECK();

    if (first_enter_flag == 0)
    {
        s_gui_value = paramsTermConfigWaterAlluvialFeedGet();
        first_enter_flag = 1;
    }

    /* 按键状态处理 */
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
        MY_SNPRINTF(str, sizeof(str), "打开");
    }
    else
    {
        MY_SNPRINTF(str, sizeof(str), "关闭");
    }

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"水冲积料开关：");
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
/* 单元硬件信息设置料筒类型 */
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

    GuiRowTextPlus(10, 2, 70, FONT_LEFT, (uint8_t *)"绞龙类型：");

    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Feeder_Type_List_Small_Strand:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"小绞龙  ");
            break;
        case Feeder_Type_List_Closures:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"闭封器  ");
            break;
        case Feeder_Type_List_Electric_Fork_Puller:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"电动拨叉");
            break;
        default:
            GuiRowTextPlus(71, 2, 57, FONT_LEFT, (uint8_t *)"未设置  ");
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
/* 单元硬件信息设置料槽类型 */
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
            /*根据不同料槽类型 设置饲喂3.0不同体重段下料上下限、保护时间*/
            switch(s_gui_value)
            {
                /* 长槽类 */
                case Feed_Trough_Type_List_BY_Long_Trough:      /* 保育舍 长槽 */
                case Feed_Trough_Type_List_BY_Disc_Trough:      /* 保育舍 圆槽 */
                case Feed_Trough_Type_List_BY_Half_Disc_Trough: /* 保育舍 半圆槽 */
                case Feed_Trough_Type_List_ONE_Long_Trough:     /* 一体舍 长槽 */
                case Feed_Trough_Type_List_ONE_1m_Long_Trough:  /* 一体舍 1米长槽 */
                case Feed_Trough_Type_List_YF_Long_Trough:      /* 育肥舍 长槽 */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_LONG_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* 圆槽类 */
                case Feed_Trough_Type_List_ONE_Disc_Trough: /* 一体舍 圆料槽 */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_DISC_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* 大白桶类 */
                case Feed_Trough_Type_List_BY_Toggle_Trough:    /* 保育舍 拨叉料槽 */
                case Feed_Trough_Type_List_BY_Endotube_Trough:  /* 保育舍 内插管料槽 */
                case Feed_Trough_Type_List_YF_Toggle_Trough:    /* 育肥舍 拨叉料槽 */
                case Feed_Trough_Type_List_YF_Endotube_Trough:  /* 育肥舍 插管料槽 */
                    paramsFeedModelDefaultFeedLimitInfoSet(FEED_LIMIT_INFO_TYPE_BIG_E);
                    paramsSetTermComEvent(0, event_set_feed_model_feed_value);
                    break;
                /* 管式类 */
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
        /* 泌乳舍 */
        case Feed_Trough_Type_List_MR_Standard_Feed_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 标准料槽  ");
            break;
        case Feed_Trough_Type_List_MR_Non_Standard_Feed_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 非标准料槽");
            break;
        /* 保育舍 */
        case Feed_Trough_Type_List_BY_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 长槽      ");
            break;
        case Feed_Trough_Type_List_BY_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 圆槽      ");
            break;
        case Feed_Trough_Type_List_BY_Half_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 半圆槽    ");
            break;
        case Feed_Trough_Type_List_BY_Toggle_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 拨叉料槽  ");
            break;
        case Feed_Trough_Type_List_BY_Endotube_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 内插管料槽");
            break;
        /* 育肥舍 */
        case Feed_Trough_Type_List_YF_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 长槽      ");
            break;
        case Feed_Trough_Type_List_YF_Toggle_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 拨叉料槽  ");
            break;
        case Feed_Trough_Type_List_YF_Endotube_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 插管料槽  ");
            break;
        /* 一体舍 */
        case Feed_Trough_Type_List_ONE_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 长槽      ");
            break;
        case Feed_Trough_Type_List_ONE_1m_Long_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 1米长槽   ");
            break;
        case Feed_Trough_Type_List_ONE_Disc_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 圆料槽    ");
            break;
        case Feed_Trough_Type_List_ONE_Tubular_Trough:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 管式料槽  ");
            break;

        default:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"未设置           ");
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

/* 单元硬件信息设置探针类型 */
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
        /* 泌乳舍 */
        case Feed_Probe_Type_List_MR_L_TYPE_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 L型探针       ");
            break;
        case Feed_Probe_Type_List_MR_Four_In_One_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 四合一探针    ");
            break;
        case Feed_Probe_Type_List_MR_Drinking_Rod_Universal_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 饮水杆万向探针 ");
            break;
        case Feed_Probe_Type_List_MR_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 饮水杆摆动探针 ");
            break;
        case Feed_Probe_Type_List_MR_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"泌乳舍 钢丝绳探针     ");
            break;

        /* 保育舍 */
        case Feed_Probe_Type_List_BY_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 钢丝绳探针     ");
            break;
        case Feed_Probe_Type_List_BY_Drinking_Rod_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 饮水杆钢丝绳探针");
            break;
        case Feed_Probe_Type_List_BY_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"保育舍 饮水杆摆动探针  ");
            break;

        /* 育肥舍 */
        case Feed_Probe_Type_List_YF_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 钢丝绳探针      ");
            break;
        case Feed_Probe_Type_List_YF_Reinforcing_Steel_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 钢筋探针        ");
            break;
        case Feed_Probe_Type_List_YF_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"育肥舍 饮水杆摆动探针  ");
            break;

        /* 一体舍 */
        case Feed_Probe_Type_List_ONE_Drinking_Rod_Double_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 饮水杆双探针    ");
            break;
        case Feed_Probe_Type_List_ONE_Box_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 盒式探针        ");
            break;
        case Feed_Probe_Type_List_ONE_Wire_Rope_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 钢丝绳探针      ");
            break;
        case Feed_Probe_Type_List_ONE_Drinking_Rod_Swing_Probe:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"一体舍 饮水杆摆动探针  ");
            break;

        default:
            GuiRowTextPlus(0, 2, 127, FONT_MID, (uint8_t *)"未设置                ");
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
/* 单元硬件信息设置探针高度 */
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
    GuiRowTextPlus(65, 2, 63, FONT_LEFT, (uint8_t *)"厘米");

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

/* 单元硬件信息设置触碰类型 */
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
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"无触碰杆");
            break;
        case Touch_Type_List_Travel_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"行程式  ");
            break;
        case Touch_Type_List_Reed_Switch_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"干簧管式");
            break;
        case Touch_Type_List_L_Type_Touch:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"机械式  ");
            break;
        default:
            GuiRowTextPlus(0, 2, 128, FONT_MID, (uint8_t *)"未设置  ");
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

/* 单元硬件信息设置料罐间距 */
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

    GuiRowTextPlus(1, 2, 65, FONT_RIGHT, (uint8_t *)"与料罐间距 ");

    rt_snprintf(str, sizeof(str), "%3d", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(66, 2, 18, FONT_LEFT, (uint8_t*)str);
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(84, 2, 44, FONT_LEFT, (uint8_t *)" 个单元");

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

/* 单元硬件信息设置水源间距 */
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

    GuiRowTextPlus(1, 2, 65, FONT_RIGHT, (uint8_t *)"与水源间距 ");

    rt_snprintf(str, sizeof(str), "%3d", s_gui_value);
    GuiSetbackcolor(CBLACK);
    GuiRowTextPlus(66, 2, 18, FONT_LEFT, (uint8_t*)str);
    GuiSetbackcolor(CWHITLE);
    GuiRowTextPlus(84, 2, 44, FONT_LEFT, (uint8_t *)" 个单元");

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

/* 单元硬件信息设置电机类型 */
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
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"正科5频 ");
            break;
        case TERM_MOTOR_TYPE_ZHENGK_2_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"正科2频 ");
            break;
        case TERM_MOTOR_TYPE_CJH_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"创晶辉  ");
            break;
        case TERM_MOTOR_TYPE_YT_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"雅腾    ");
            break;
        case TERM_MOTOR_TYPE_JL_LONG_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"金立长款");
            break;
        case TERM_MOTOR_TYPE_JL_SHORT_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"金立短款");
            break;
        case TERM_MOTOR_TYPE_DMK_E:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"德玛克  ");
            break;
        default:
            GuiRowTextPlus(40, 2, 88, FONT_LEFT, (uint8_t *)"未设置  ");
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
/* 单元硬件信息设置一体阀类型 */
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
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"1400脉冲/L");
            break;
        case Flowmeter_Type_List_Pulse_1200:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"1200脉冲/L");
            break;
        case Flowmeter_Type_List_Pulse_600:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"600脉冲/L ");
            break;
        case Flowmeter_Type_List_Pulse_520:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"520脉冲/L ");
            break;
        default:
            GuiRowTextPlus(34, 2, 94, FONT_LEFT, (uint8_t *)"未设置    ");
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

/** 饲喂模型配方设置 */
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

    GuiRowTextPlus(22, 2, 84, FONT_LEFT, (uint8_t *)"配方选择: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Corn:
            MY_SNPRINTF(str, sizeof(str), "玉米");break;
        case Brown_rice:
            MY_SNPRINTF(str, sizeof(str), "糙米");break;
        case Broken_rice:
            MY_SNPRINTF(str, sizeof(str), "碎米");break;
        case Barley:
            MY_SNPRINTF(str, sizeof(str), "大麦");break;
        case Wheat:
            MY_SNPRINTF(str, sizeof(str), "小麦");break;
        case Kaoliang:
            MY_SNPRINTF(str, sizeof(str), "高粱");break;
        default:
            MY_SNPRINTF(str, sizeof(str), "错误");break;
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
/** 饲喂模型含粉率设置 */
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

    GuiRowTextPlus(19, 2, 90, FONT_LEFT, (uint8_t *)"含粉选择: ");
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
            MY_SNPRINTF(str, sizeof(str), "错误 ");break;
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
/** 饲喂模型温湿度设置 */
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

    GuiRowTextPlus(6, 1, 127, FONT_LEFT, (uint8_t *)"温度     湿度");
    /* 这里不能用MY_SNPRINTF, 因为打印不了浮点数 */
    snprintf(str, sizeof(str), "%02.1f℃   %02.1f%%", s_df_gui_value_0, (s_df_gui_value_1*100));
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
/** 饲喂模型健康设置 */
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

    GuiRowTextPlus(10, 2, 108, FONT_LEFT, (uint8_t *)"健康选择: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case Mild_fever:
            MY_SNPRINTF(str, sizeof(str), "轻度发烧");
            break;
        case Moderate_fever:
            MY_SNPRINTF(str, sizeof(str), "中度发烧");
            break;
        case Severe_fever:
            MY_SNPRINTF(str, sizeof(str), "重度发烧");
            break;
        case Mild_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "轻度腹泻");
            break;
        case Moderate_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "中度腹泻");
            break;
        case Severe_diarrhea:
            MY_SNPRINTF(str, sizeof(str), "重度腹泻 ");
            break;
        case Mild_immune:
            MY_SNPRINTF(str, sizeof(str), "轻度免疫");
            break;
        case Moderate_immune:
            MY_SNPRINTF(str, sizeof(str), "中度免疫");
            break;
        case Severe_immune:
            MY_SNPRINTF(str, sizeof(str), "重度免疫");
            break;
        case healthy:
        default:
            s_gui_value = healthy;
            MY_SNPRINTF(str, sizeof(str), "猪群健康");
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
/** 饲喂模型体重设置 */
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

    GuiRowTextPlus(28, 2, 72, FONT_LEFT, (uint8_t *)"体重: ");
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
/** 饲喂模型开关设置 */
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

    GuiRowTextPlus(16, 2, 96, FONT_LEFT, (uint8_t *)"模型开关: ");
    GuiSetbackcolor(CBLACK);
    switch(s_gui_value)
    {
        case 1:
            MY_SNPRINTF(str, sizeof(str), " 料开 ");
            break;
        case 2:
            MY_SNPRINTF(str, sizeof(str), "水料开");
            break;
        case 0:
        default:
            s_gui_value = 0;
            MY_SNPRINTF(str, sizeof(str), "  关  ");
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

/* 字符串转为二维码并显示函数 */
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

/* 用于重启mqtt服务 */
static int32_t _userMqttRestart(void)
{
    Plat_Eos_Custom_Data_Head_T t_eos_custom_data_head = {0};

    t_eos_custom_data_head.e_src  = PLAT_EOS_CUSTOM_DATA_SRC_GUI_E;
    t_eos_custom_data_head.e_type = PLAT_EOS_CUSTOM_DATA_TYPE_RESTART_E;

    /* 通过eos通知feed mqtt重新开始mqtt服务 */
    if (platEosEventPubCustomData(PLAT_EOS_EVENT_FEED_MQTT_E, &t_eos_custom_data_head, NULL, 0) == RT_EOK)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}

/* 限制条件: min <= max */
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
/* 限制条件: min <= max */
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
/*                 对外函数实现                 */
/************************************************/
/**
  *@brief gui初始化参考函数
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

    /* 1. 初始化lcd */
    LcdInit();
    /* 2. 初始化用户字体 */
    defaultFontInit();
    /* 3.菜单初始化 */
    UserMenuInit();
    /* 4.初始化显示 */
    GuiSetForecolor(CBLACK);
    GuiSetbackcolor(CWHITLE);
    _guiPowerOnProgress();
    /* 5. 初始化窗口 */
    GuiWinInit();
    /* 6. 更新屏幕 */
    GuiClearScreen(GuiGetbackcolor());
    /* 7. 加入主页面  */
    GuiWinAdd(&s_win_home);

    OpenLcdDisplay();
    GuiUpdateDisplayAll();
    s_gui_inited_end = 1;
}

/**
  *@brief gui运行函数
  *@param  None
  *@retval None
  */
void userAppPortRun(void)
{
    static uint8_t font_load_flag = 0;
    static uint32_t last_tick = 0;
    uint32_t cur_tick = MY_TICK_GET();

    /* userAppPortInit 没有执行完则直接返回 */
    if (s_gui_inited_end == 0)      return;

    /* 获取按键状态 */
    if (s_pt_key_node != NULL)
    {
        s_i_key_status = s_pt_key_node->pf_ctrl(MY_NODE_CTRL_CMD_READ,&s_i_key_status);
    }
    else
    {
        s_i_key_status = 0;
    }

    /* 当没有按键操作时，屏幕不执行那么频繁 */
    if ((s_i_key_status == 0)&&(s_gui_saving == 0)&&((uint32_t)(cur_tick-last_tick) < 200))
    {
        return;
    }
    last_tick = cur_tick;

    /* 窗口调度管理 */
    /* Lora终端远程注册界面, 两个条件同时满足：远程注册标志不为0且是Lora终端 */
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

    /* 按键状态清除 */
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

/* 当应用初始化完成后，调用该函数来通知gui进度条应该尽快结束 */
void userGuiSetProcessEnd(void)
{
    s_gui_process_end = 1;
}




