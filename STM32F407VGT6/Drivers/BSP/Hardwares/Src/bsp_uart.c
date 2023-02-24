/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bsp_uart.c
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
#include "bsp_uart.h"

/* Private includes ----------------------------------------------------------*/
#include "module_init.h"
#include "bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  MX_UART5_Init.
 * @param  none
 * @return none
*/
static void MX_UART5_Init(void)
{
    huart5.Instance = UART5;
    huart5.Init.BaudRate = 115200;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart5) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  MX_UART4_Init.
 * @param  none
 * @return none
*/
static void MX_UART4_Init(void)
{
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 9600;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  MX_USART2_UART_Init.
 * @param  none
 * @return none
*/
static void MX_USART2_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  BSP_Uart_Init.
 * @param  none
 * @return none
*/
int BSP_Uart_Init(void)
{
    MX_UART5_Init();
    MX_UART4_Init();
    MX_USART2_Init();
    
    return 0;
}
TX_INIT_BOARD_EXPORT(BSP_Uart_Init);

/**
 * @brief  UART MSP Init.
 * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
 *                the configuration information for the specified UART module.
 * @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(uartHandle->Instance==UART4)
    {
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**UART4 GPIO Configuration
        PC10     ------> UART4_TX
        PC11     ------> UART4_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* UART4 interrupt Init */
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
    }
    else if(uartHandle->Instance==UART5)
    {

        /* UART5 clock enable */
        __HAL_RCC_UART5_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**UART5 GPIO Configuration
        PC12     ------> UART5_TX
        PD2     ------> UART5_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* UART5 interrupt Init */
        HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART5_IRQn);

    }
    else if(uartHandle->Instance==USART2)
    {

        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PD5     ------> USART2_TX
        PD6     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}


/**
 * @brief  UART MSP DeInit.
 * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
 *                the configuration information for the specified UART module.
 * @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if(uartHandle->Instance==UART4)
    {
        /* Peripheral clock disable */
        __HAL_RCC_UART4_CLK_DISABLE();

        /**UART4 GPIO Configuration
        PC10     ------> UART4_TX
        PC11     ------> UART4_RX
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

        /* UART4 interrupt Deinit */
        HAL_NVIC_DisableIRQ(UART4_IRQn);
    }
    else if(uartHandle->Instance==UART5)
    {
        /* Peripheral clock disable */
        __HAL_RCC_UART5_CLK_DISABLE();

        /**UART5 GPIO Configuration
        PC12     ------> UART5_TX
        PD2     ------> UART5_RX
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

        /* UART5 interrupt Deinit */
        HAL_NVIC_DisableIRQ(UART5_IRQn);

    }
    else if(uartHandle->Instance==USART2)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PD5     ------> USART2_TX
        PD6     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5|GPIO_PIN_6);

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5)
    {
        
    }
}

UART_HandleTypeDef *BSP_Get_Uart_Handle(UART_ID_T UART_ID)
{
    if (UART_ID == BSP_UART2)
    {
        return &huart2;
    }
    else if (UART_ID == BSP_UART4)
    {
        return &huart4;
    }
    else if (UART_ID == BSP_UART5)
    {
        return &huart5;
    }

    return NULL;
}

UART_ID_T BSP_Get_Uart_id(UART_HandleTypeDef *huart)
{
    if (huart == &huart2)
    {
        return BSP_UART2;
    }
    else if (huart == &huart4)
    {
        return BSP_UART4;
    }
    else if (huart == &huart5)
    {
        return BSP_UART5;
    }
    
    return BSP_UART_NULL;
}

