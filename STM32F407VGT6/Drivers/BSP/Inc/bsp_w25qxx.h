/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_w25qxx.h
  * @brief          : Flash driver module
  ******************************************************************************
  * @attention      The function must be called before the scheduler is started
  */
/* USER CODE END Header */
#ifndef _BSP_W25QXX_H_
#define _BSP_W25QXX_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
/* W25X/Q chip list */
#define W25Q80  0XEF13
#define W25Q16  0XEF14
#define W25Q32  0XEF15
#define W25Q64  0XEF16

/* command list */
#define W25X_WriteEnable            0x06
#define W25X_WriteDisable           0x04
#define W25X_ReadStatusReg          0x05
#define W25X_WriteStatusReg         0x01
#define W25X_ReadData               0x03
#define W25X_FastReadData           0x0B
#define W25X_FastReadDual           0x3B
#define W25X_PageProgram            0x02
#define W25X_BlockErase             0xD8
#define W25X_SectorErase            0x20
#define W25X_ChipErase              0xC7
#define W25X_PowerDown              0xB9
#define W25X_ReleasePowerDown       0xAB
#define W25X_DeviceID               0xAB
#define W25X_ManufactDeviceID       0x90
#define W25X_JedecDeviceID          0x9F

#define SPI_FLASH_SECTOR_SIZE       4096ul

#define EX_SECTOR 4096ul
#define SECTOR    4096ul
/* Exported variables --------------------------------------------------------*/
typedef unsigned           char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t; 
/* Exported functions --------------------------------------------------------*/
void SPI_Flash_Init(void);

void SPI_FLASH_InitPort(void);

void SPI_Flash_Read(uint32_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);

void SPI_Flash_Write(uint32_t WriteAddr, uint8_t* pBuffer, uint16_t NumByteToWrite);

void SPI_Flash_Erase_Sector(uint32_t Dst_Addr);

void SPI_Flash_Erase_Chip(void);

void SPI_Flash_Write_NoCheck(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);

void Read_Flash_Bytes(uint32_t ReadAddr, uint8_t* pBuffer, uint16_t NumByteToRead);

void Write_Flash_Bytes(uint32_t WriteAddr, uint8_t* pBuffer, uint16_t NumByteToWrite);

#ifdef __cplusplus
}
#endif

#endif /* _BSP_W25QXX_H_ */
