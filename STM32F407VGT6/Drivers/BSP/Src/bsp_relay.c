/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_relay.c
  * @brief          : Peripheral relay initialization
  ******************************************************************************
  * @attention       This function call in BSP
  *
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_relay.h"

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "module_init.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct _relay_cfg
{
    GPIO_TypeDef* port;
    uint16_t pin;
} relay_cfg_t;

typedef enum
{
    RELAY1 = 0,
    RELAY2,
    RELAY3,
    RELAY4
}RELAY_NUM;
/* Private define ------------------------------------------------------------*/
#define RELAY_NUM  4

#define RELAY1_Pin       GPIO_PIN_8
#define RELAY1_GPIO_Port GPIOA

#define RELAY2_Pin       GPIO_PIN_9
#define RELAY2_GPIO_Port GPIOA

#define RELAY3_Pin       GPIO_PIN_10
#define RELAY3_GPIO_Port GPIOA

#define RELAY4_Pin       GPIO_PIN_11
#define RELAY4_GPIO_Port GPIOA

#define RELAY_GPIO_PORT_CLK_EN()     \
        __HAL_RCC_GPIOA_CLK_ENABLE()

#define RELAY_SET(port, pin, status) \
        HAL_GPIO_WritePin(port, pin, ((status) ? GPIO_PIN_RESET : GPIO_PIN_SET));
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const relay_cfg_t relay_config[RELAY_NUM] =
{
    {RELAY1_GPIO_Port,  RELAY1_Pin},
    {RELAY2_GPIO_Port,  RELAY2_Pin},
    {RELAY3_GPIO_Port,  RELAY3_Pin},
    {RELAY4_GPIO_Port,  RELAY4_Pin},
};
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  relay gpio initialization
 * @param  none
 * @return 0
*/
int BSP_Relay_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RELAY_GPIO_PORT_CLK_EN();

    for(int i = 0; i < RELAY_NUM; i++)
    {
        const relay_cfg_t* relay_cfg = &relay_config[i];

        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Pin   = relay_cfg->pin;

        HAL_GPIO_Init(relay_cfg->port, &GPIO_InitStruct);

        RELAY_SET(relay_cfg->port, relay_cfg->pin, RELAY_OFF);
    }

    return 0;
}
TX_INIT_BOARD_EXPORT(BSP_Relay_Init);

/**
 * @brief  relay contrl on
 * @param  relay id
 * @return relay id
*/
int32_t relay_on(int32_t relay_id)
{
    if(relay_id < RELAY_NUM)
    {
        const relay_cfg_t* relay_cfg = &relay_config[relay_id];

        RELAY_SET(relay_cfg->port, relay_cfg->pin, RELAY_ON);
    }

    return relay_id;
}

/**
 * @brief  relay contrl off
 * @param  relay id
 * @return relay id
*/
int32_t relay_off(int32_t relay_id)
{
    if(relay_id < RELAY_NUM)
    {
        const relay_cfg_t* relay_cfg = &relay_config[relay_id];

        RELAY_SET(relay_cfg->port, relay_cfg->pin, RELAY_OFF);
    }

    return relay_id;
}

/**
 * @brief  led contrl
 * @param  led id
 *         0 -> OFF 1-> ON
 * @return led id
*/
int32_t relay_ctr(int32_t relay_id, int8_t relay_state)
{
    if(relay_id < RELAY_NUM)
    {
        const relay_cfg_t* relay_cfg = &relay_config[relay_id];
        
        if (relay_state == 0)
        {
            RELAY_SET(relay_cfg->port, relay_cfg->pin, RELAY_OFF);
        }
        else if (relay_state == 1)
        {
            RELAY_SET(relay_cfg->port, relay_cfg->pin, RELAY_ON);
        }
        
    }

    return relay_id;
}
