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
#include "tx_api.h"
#include "tx_user.h"
#include "module_init.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct _led_cfg
{
    GPIO_TypeDef* port;
    uint16_t pin;
} led_cfg_t;

typedef struct _led_status
{
    uint8_t  state;
    uint8_t  blink_en;// 10Hz
    uint8_t  blink_on;
    uint8_t  blink_off;
    uint32_t blink_counter;
    uint32_t blink_count;
} led_status_t;

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

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static TX_TIMER led_timer;

/* Note:If you need to add LED pins, please add them below */
static const led_cfg_t led_config[LED_NUM] =
{
    {LED_R_GPIO_Port,  LED_R_Pin},
    {LED_G1_GPIO_Port, LED_G1_Pin},
    {LED_G2_GPIO_Port, LED_G2_Pin},
};

static led_status_t led_status[LED_NUM];
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  led timer callback
 * @param  id
 * @return none
*/
static void led_tmr_cb(ULONG id)
{
    for(int led_id = 0; led_id < LED_NUM; led_id++)
    {
        const led_cfg_t* led_cfg = &led_config[led_id];
        led_status_t* led_status = &led_status[led_id];

        if(led_status->blink_en)
        {
            led_status->blink_counter++;

            if(led_status->blink_counter <= led_status->blink_on)
            {
                LED_SET(led_cfg->port, led_cfg->pin, LED_ON);
            }
            else if(led_status->blink_counter <= led_status->blink_off)
            {
                LED_SET(led_cfg->port, led_cfg->pin, LED_OFF);
            }
            else
            {
                led_status->blink_counter = 0;

                if(led_status->blink_count != LED_BLINK_FOREVER)
                {
                    if(led_status->blink_count == 0)
                    {
                        led_status->blink_en = 0;
                        led_status->blink_on = 0;
                        led_status->blink_off = 0;
                        led_status->blink_count = 0;

                        LED_SET(led_cfg->port, led_cfg->pin, led_status->state);
                    }
                    else
                    {
                        led_status->blink_count -= 1;
                    }
                }
            }
        }
    }
}

/**
 * @brief  create an led timer
 * @param  none
 * @return 0
*/
static int32_t bsp_led_tmr_init(void)
{
    tx_timer_create(&led_timer, 
                    "led_timer", 
                    led_tmr_cb,
                    0, 
                    TX_MS_TO_TICKS(100), 
                    TX_MS_TO_TICKS(100), 
                    TX_AUTO_ACTIVATE);

    return 0;
}

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

    bsp_led_tmr_init();

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
 * @brief  led contrl
 * @param  led id
 *         0 -> OFF 1-> ON
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
        
    }

    return led_id;
}

/**
 * @brief  led blink
 * @param  led id 
 *         state
 *         blink on
 *         blink off
 *         blink count
 * @return led id
*/
int32_t led_bink(int32_t led_id, uint8_t state, uint8_t blink_on, uint8_t blink_off, uint32_t blink_count)
{
    if(led_id < LED_NUM)
    {
        led_status_t* led_status = &led_status[led_id];

        led_status->state = state;
        led_status->blink_on = blink_on;
        led_status->blink_off = blink_off;
        led_status->blink_count = blink_count;

        led_status->blink_en = 1;
        led_status->blink_counter = 0;
    }

    return led_id;
}

/* Instructions for use --------------------------------------------------------*/
/**
    Note: -> 1.You need to call "led_init" to initialize before using it;
          -> 2.led_bink(0, LED_OFF, 5, 10, LED_BLINK_FOREVER);
          -> 3.led_on(0);
          -> 4.led_off(0).
*/
