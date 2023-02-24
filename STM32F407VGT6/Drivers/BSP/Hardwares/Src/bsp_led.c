/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_led.c
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
#include "bsp_led.h"

/* Private includes ----------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "app_azure_rtos.h"
#include "module_init.h"
//#include "tx_api.h"
//#include "tx_user.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct _led_cfg
{
    GPIO_TypeDef* port;
    uint16_t pin;
} led_cfg_t;

/* Private define ------------------------------------------------------------*/
/* Define the number of LEDs and pins */
#define LED_NUM          3

#define LED_R_Pin        GPIO_PIN_3
#define LED_R_GPIO_Port  GPIOC

#define LED_G1_Pin       GPIO_PIN_2
#define LED_G1_GPIO_Port GPIOC

#define LED_G2_Pin       GPIO_PIN_0
#define LED_G2_GPIO_Port GPIOC

#define LED_GPIO_PORT_CLK_EN()     \
        __HAL_RCC_GPIOC_CLK_ENABLE()

#define LED_SET(port, pin, status) \
        HAL_GPIO_WritePin(port, pin, ((status) ? GPIO_PIN_RESET : GPIO_PIN_SET));

#define LED_TOGGLE(port, pin)             \
        HAL_GPIO_TogglePin(port, pin);

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Note:If you need to add LED pins, please add them below */
static const led_cfg_t led_config[LED_NUM] =
{
    {LED_R_GPIO_Port,  LED_R_Pin},
    {LED_G1_GPIO_Port, LED_G1_Pin},
    {LED_G2_GPIO_Port, LED_G2_Pin},
};

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  led gpio initialization
 * @param  none
 * @return 0
*/
int BSP_Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    LED_GPIO_PORT_CLK_EN();

    for(int i = 0; i < LED_NUM; i++)
    {
        const led_cfg_t* led_cfg = &led_config[i];

        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull  = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Pin   = led_cfg->pin;

        HAL_GPIO_Init(led_cfg->port, &GPIO_InitStruct);

        LED_SET(led_cfg->port, led_cfg->pin, LED_OFF);
    }

    return 0;
}
TX_INIT_DEVICE_EXPORT(BSP_Led_Init);

/**
 * @brief  led contrl on
 * @param  led id
 * @return led id
*/
int32_t led_on(int32_t led_id)
{
    if(led_id < LED_NUM)
    {
        const led_cfg_t* led_cfg = &led_config[led_id];

        LED_SET(led_cfg->port, led_cfg->pin, LED_ON);
    }

    return led_id;
}

/**
 * @brief  led contrl off
 * @param  led id
 * @return led id
*/
int32_t led_off(int32_t led_id)
{
    if(led_id < LED_NUM)
    {
        const led_cfg_t* led_cfg = &led_config[led_id];

        LED_SET(led_cfg->port, led_cfg->pin, LED_OFF);
    }

    return led_id;
}

/**
 * @brief  led blink
 * @param  led id, led blink frequency
 * @return led id
*/
int32_t led_blink(int32_t led_id, uint32_t ms)
{
    if (led_id < LED_NUM)
    {
        const led_cfg_t* led_cfg = &led_config[led_id];
        LED_TOGGLE(led_cfg->port, led_cfg->pin);
        TX_Delay(ms);
    }
    
    return led_id;
}

/**
 * @brief  led contrl
 * @param  led id
 *         0 -> OFF 1-> ON 2-> BLINK
 * @return led id
*/
int32_t led_ctr(int32_t led_id, int8_t led_state)
{
    if(led_id < LED_NUM)
    {
        const led_cfg_t* led_cfg = &led_config[led_id];
        
        if (led_state == 0)
        {
            LED_SET(led_cfg->port, led_cfg->pin, LED_OFF);
        }
        else if (led_state == 1)
        {
            LED_SET(led_cfg->port, led_cfg->pin, LED_ON);
        }
        else if (led_state == 2)
        {
            LED_TOGGLE(led_cfg->port, led_cfg->pin);
        }
    }

    return led_id;
}



/* Instructions for use --------------------------------------------------------*/
/**
    Note: -> 1.You need to call "led_init" to initialize before using it;
          -> 2.led_bink(0, 1000);
          -> 3.led_on(0);
          -> 4.led_off(0).
*/
