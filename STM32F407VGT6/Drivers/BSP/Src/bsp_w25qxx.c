/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_w25qxx.c
  * @brief          : Flash driver module
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "bsp_w25qxx.h"
/* Private includes ----------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "bsp.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SPI1_CLK_Pin       GPIO_PIN_3
#define SPI1_CLK_GPIO_Port GPIOB

#define SPI1_CS_Pin        GPIO_PIN_6
#define SPI1_CS_GPIO_Port  GPIOB
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO unsigned short W25QXX_TYPE = 0x00;
/* Private function prototypes -----------------------------------------------*/
SPI_HandleTypeDef hspi1;

/**
 * @brief  SPI init function
 * @param  none
 * @return none
*/
void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  Config the hardware resources
 * @param  none
 * @return none
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance==SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = SPI1_CLK_Pin|GPIO_PIN_4|GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    if(spiHandle->Instance==SPI1)
    {
        __HAL_RCC_SPI1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, SPI1_CLK_Pin|GPIO_PIN_4|GPIO_PIN_5);
    }
}


/**
 * @brief  SPI1_ReadWriteByte
 * @param  TxData : write a byte
 * @return RxData : read a byte
*/
unsigned char SPI1_ReadWriteByte(unsigned char TxData)
{
    unsigned char RxData;
    HAL_SPI_TransmitReceive(&hspi1,&TxData,&RxData,1,1000);
    return RxData;
}

/**
 * @brief  SPI_FLASH_CS
 * @param  none
 * @return none
*/
void SPI_FLASH_CS(uint8_t cs)
{
    if (cs == 0)
    {
        HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
    }
}

/**
 * @brief  SPI_FLASH_InitPort
 * @param  none
 * @return none
*/
void SPI_FLASH_InitPort(void)
{
    MX_SPI1_Init();
}

/**
 * @brief  SPI_ReadWriteByte
 * @param  none
 * @return none
*/
uint8_t SPI_ReadWriteByte(uint8_t byte)
{
    return SPI1_ReadWriteByte(byte);
}

/**
 * @brief  SPI_FLASH_Write_SR
 * @param  none
 * @return none
*/
void SPI_FLASH_Write_SR(uint8_t sr)
{
    SPI_FLASH_CS(0);                            //enable spi cs
    SPI_ReadWriteByte(W25X_WriteStatusReg);     //Sends a write status register command
    SPI_ReadWriteByte(sr);                      //write a byte
    SPI_FLASH_CS(1);                            //disable spi cs
}

/**
 * @brief  Read the status register of the SPI_FLASH
 * @param  BIT7  6   5   4   3   2   1   0
                SPR   RV  TB BP2 BP1 BP0 WEL BUSY
                SPR:Default 0, status register protection bit, used with WP
                TB,BP2,BP1,BP0:FLASH area write protection settings
                WEL:Write enable locking
                BUSY:Busy flag bits (1, busy; 0, idle)
                Default:0x00
 * @return none
*/
uint8_t SPI_Flash_ReadSR(void)
{
    uint8_t byte;

    SPI_FLASH_CS(0);                        //Enable the device
    SPI_ReadWriteByte(W25X_ReadStatusReg);  //Sends a read status register command
    byte = SPI_ReadWriteByte(0xFF);         //Read a byte
    SPI_FLASH_CS(1);                        //Disable the device

    return byte;
}

/**
 * @brief  SPI_FLASH_Write_Enable
 * @param  none
 * @return none
 * @note   Set WEL in place
*/
void SPI_FLASH_Write_Enable(void)
{
    SPI_FLASH_CS(0);                        //Enable the device
    SPI_ReadWriteByte(W25X_WriteEnable);    //Send write enable
    SPI_FLASH_CS(1);                        //Disable the device
}

/**
 * @brief  SPI_FLASH_Write_Disable
 * @param  none
 * @return none
 * @note   SPI_FLASH write prohibited
           Clear WEL to zero
*/
void SPI_FLASH_Write_Disable(void)
{
    SPI_FLASH_CS(0);                        //Enable the device
    SPI_ReadWriteByte(W25X_WriteDisable);   //Send write forbidden instructions
    SPI_FLASH_CS(1);                        //Disable the device
}

/**
 * @brief  SPI_Flash_ReadID
 * @param  0XEF13 -> W25Q80
           0XEF14 -> W25Q16
           0XEF15 -> W25Q32
           0XEF16 -> W25Q64
 * @return none
 * @note   read flash ic id number
*/
uint16_t SPI_Flash_ReadID(void)
{
    uint16_t Temp = 0;
    SPI_FLASH_CS(0);
    SPI_ReadWriteByte(0x90); //Send the read ID command
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);
    SPI_ReadWriteByte(0x00);

    Temp |= SPI_ReadWriteByte(0xFF) << 8;
    Temp |= SPI_ReadWriteByte(0xFF);
    SPI_FLASH_CS(1);

    return Temp;
}

/**
 * @brief  SPI_Flash_ReadUID
 * @param  none
 * @return none
 * @note   none
*/
void SPI_Flash_ReadUID(uint8_t* p_uid)
{
    uint8_t i;
    SPI_FLASH_CS(0);

    SPI_ReadWriteByte(0x4B); //Send the read UID command

    for( i = 0; i < 4; i++)
    {
        SPI_ReadWriteByte(0x00);
    }

    for( i = 0; i < 8; i++)
    {
        p_uid[i] = SPI_ReadWriteByte(0xFF);
    }

    SPI_FLASH_CS(1);
}

/**
 * @brief  SPI_Flash_Wait_Busy
 * @param  none
 * @return none
 * @note   none
*/
void SPI_Flash_Wait_Busy(void)
{
    while((SPI_Flash_ReadSR() & 0x01) == 0x01); //Wait for the BUS bit to clear
}

/**
 * @brief  SPI_Flash_Write_Page
 * @param  pBuffer        : Data store
           WriteAddr      : Address to start writing (24bit)
           NumByteToWrite :The number of bytes to write (up to 256), which should not exceed the number of bytes remaining on the page
 * @return none
 * @note   none
*/
void SPI_Flash_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;
    SPI_FLASH_Write_Enable();
    SPI_FLASH_CS(0);
    SPI_ReadWriteByte(W25X_PageProgram);
    SPI_ReadWriteByte((uint8_t)((WriteAddr) >> 16));
    SPI_ReadWriteByte((uint8_t)((WriteAddr) >> 8));
    SPI_ReadWriteByte((uint8_t)WriteAddr);
    for(i = 0; i < NumByteToWrite; i++)
    {
        SPI_ReadWriteByte(pBuffer[i]);
    }
    SPI_FLASH_CS(1);
    SPI_Flash_Wait_Busy();
}

/**
 * @brief  SPI_Flash_Write_NoCheck
 * @param  pBuffer        : Data store
           WriteAddr      : Address to start writing (24bit)
           NumByteToWrite : Bytes to write (maximum 65535)
 * @return none
 * @note   WRITE SPI FLASH WITHOUT VERIFICATION
           You must ensure that the data in the address range you write is all 0XFF, otherwise the data written at the non-0XFF will fail!
           With automatic page wrapping
           Start writing data of the specified length at the specified address, but make sure that the address does not cross the bounds
*/
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain = 256 - WriteAddr % 256;

    if(NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;
    }

    while(1)
    {
        SPI_Flash_Write_Page(pBuffer, WriteAddr, pageremain);

        if(NumByteToWrite == pageremain)
        {
            break;
        }
        else
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain;
            if(NumByteToWrite > 256)
            {
                pageremain = 256;
            }
            else
            {
                pageremain = NumByteToWrite;
            }
        }
    }
}

/**
 * @brief  SPI_Flash_Erase_Sector
 * @param  Dst_Addr: Sector address Set according to the actual capacity
 * @return none
 * @note   Erase a sector
           Minimum time to erase a sector: 150ms
*/
void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)
{
    Dst_Addr *= SPI_FLASH_SECTOR_SIZE;

    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS(0);
    SPI_ReadWriteByte(W25X_SectorErase);
    SPI_ReadWriteByte((uint8_t)((Dst_Addr) >> 16));
    SPI_ReadWriteByte((uint8_t)((Dst_Addr) >> 8));
    SPI_ReadWriteByte((uint8_t)Dst_Addr);
    SPI_FLASH_CS(1);
    SPI_Flash_Wait_Busy();
}

/**
 * @brief  SPI_Flash_Erase_Chip
 * @param  none
 * @return none
 * @note   Erase the entire chip,extra long wait times
*/
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_Write_Enable();
    SPI_Flash_Wait_Busy();
    SPI_FLASH_CS(0);
    SPI_ReadWriteByte(W25X_ChipErase);
    SPI_FLASH_CS(1);
    SPI_Flash_Wait_Busy();
}

/**
 * @brief  SPI_Flash_Read
 * @param  pBuffer        : Data store
           WriteAddr      : Address to start writing (24bit)
           NumByteToWrite : Bytes to write (maximum 65535)
 * @return none
 * @note   0XEF13 -> W25Q80
           0XEF14 -> W25Q16
           0XEF15 -> W25Q32
           0XEF16 -> W25Q64
*/
void SPI_Flash_Read(uint32_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    uint16_t i;
    SPI_FLASH_CS(0);
    SPI_ReadWriteByte(W25X_ReadData);
    SPI_ReadWriteByte((uint8_t)((ReadAddr) >> 16));
    SPI_ReadWriteByte((uint8_t)((ReadAddr) >> 8));
    SPI_ReadWriteByte((uint8_t)ReadAddr);
    for( i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = SPI_ReadWriteByte(0xFF);
    }
    SPI_FLASH_CS(1);
}

/**
 * @brief  SPI_Flash_Write
 * @param  pBuffer        : Data store
           WriteAddr      : Address to start writing (24bit)
           NumByteToWrite : Bytes to write (maximum 65535)
 * @return none
 * @note   none
*/
unsigned char spi_buff[SPI_FLASH_SECTOR_SIZE];
void SPI_Flash_Write(uint32_t WriteAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint8_t* SPI_FLASH_BUF = spi_buff;
    if(SPI_FLASH_BUF == NULL)
    {
        return;
    }
    
    secpos = WriteAddr / SPI_FLASH_SECTOR_SIZE;
    secoff = WriteAddr % SPI_FLASH_SECTOR_SIZE;
    secremain = SPI_FLASH_SECTOR_SIZE - secoff;
    
    if(NumByteToWrite <= secremain)
    {
        secremain = NumByteToWrite;
    }
    
    while(1)
    {
        uint16_t i;
        SPI_Flash_Read(secpos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_BUF, SPI_FLASH_SECTOR_SIZE);

        for(i = 0; i < secremain; i++)
        {
            if(SPI_FLASH_BUF[secoff + i] != 0xFF)
            {
                break;
            }
        }
        if(i < secremain)
        {
            SPI_Flash_Erase_Sector(secpos);
            for(i = 0; i < secremain; i++)
            {
                SPI_FLASH_BUF[i + secoff] = pBuffer[i];
            }
            SPI_Flash_Write_NoCheck(SPI_FLASH_BUF, secpos * SPI_FLASH_SECTOR_SIZE, SPI_FLASH_SECTOR_SIZE);
        }
        else
        {
            SPI_Flash_Write_NoCheck(pBuffer, WriteAddr, secremain);
        }

        if(NumByteToWrite == secremain)
        {
            break;
        }
        else
        {
            secpos++;
            secoff = 0;

            pBuffer += secremain;
            WriteAddr += secremain;
            NumByteToWrite -= secremain;
            if(NumByteToWrite > SPI_FLASH_SECTOR_SIZE)
            {
                secremain = SPI_FLASH_SECTOR_SIZE;
            }
            else
            {
                secremain = NumByteToWrite;
            }
        }
    }
}

/**
 * @brief  SPI_Flash_Error
 * @param  none
 * @return none
 * @note   none
*/
void SPI_Flash_Error(void)
{
    while(1)
    {
        Error_Handler(__FILE__, __LINE__);
    }
}

/**
 * @brief  SPI_Flash_Done
 * @param  none
 * @return none
 * @note   none
*/
void SPI_Flash_Done(void)
{
    return;
}

/**
 * @brief  SPI_Flash_Init
 *         4Kbytes is a Sector sector, 16 sectors are 1 Block,
 *         W25Q64 has a capacity of 8M bytes, a total of 128 Blocks, 2048 Sectors
 * @param  none
 * @return none
 * @note   Initialize the IO port of SPI FLASH
*/
void SPI_Flash_Init(void)
{
    uint16_t spi_flash_id;
    SPI_FLASH_InitPort();
    HAL_Delay(1);
    spi_flash_id = SPI_Flash_ReadID();        //Read FLASH ID
    HAL_Delay(1);
    spi_flash_id = SPI_Flash_ReadID();        //Read FLASH ID
    while((spi_flash_id != W25Q80) && \
            (spi_flash_id != W25Q16) && \
            (spi_flash_id != W25Q32) && \
            (spi_flash_id != W25Q64))
    {
        SPI_Flash_Error();                   //SPI_FLASH Init error
    }
    // SPI_Flash_Done();                        //SPI_Flash Init success
}

#if 0
/* flash read */
void Read_Flash_Bytes(uint32_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    __disable_irq();
    SPI_Flash_Read(ReadAddr,pBuffer,NumByteToRead);
    __enable_irq();
}

/* flash read */
void Write_Flash_Bytes(uint32_t WriteAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    __disable_irq();
    SPI_Flash_Write(WriteAddr,pBuffer,NumByteToWrite);
    __enable_irq();
} 
#endif
