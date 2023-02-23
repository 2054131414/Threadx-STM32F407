/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lx_nor_flash_user_port.c
  * @brief          : Use LevelX to control Norflash
  ******************************************************************************
  * @attention        Controls flash read and write equalization
  * 
  * @note             W25Q64:一页256字节;
  *                          4K(4096 字节)为一个扇区;
  *                          16个扇区为1块;
  *                          容量为8M字节;
  *                          共有128个块;
  *                          2048个扇区
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "lx_api.h"
#include "bsp_w25qxx.h"
#include "lx_nor_flash_user_port.h"
/* Exported defines ----------------------------------------------------------*/
#define FLASH_SIZE_PER_BLOCK         64                                         /* Kbyte */
#define FLASH_SECTORS_PER_BLOCK      (FLASH_SIZE_PER_BLOCK/4)                   /* Sector size per block */

#define FLASH_TOTAL_SECTORS          128                                        /* W25Q64 total 128 sectors */

#define FLASH_NAME                   "W25Q64FV"                                 /* Flash type */
#define FLASH_BASE_ADDR              0x00                                       /* Flash base address */
#define FLASH_TOTAL_BLOCKS           FLASH_TOTAL_SECTORS                        /* Total block */

#define FLASH_SIZE_KB_PER_SECTOR     4                                          /* Each sector size (unit:Kbyte) */
#define FLASH_SIZE_BYTE_PER_SECTOR   (FLASH_SIZE_KB_PER_SECTOR * 1024)          /* Byte size per sector */

#define FLASH_WORDS_PER_SECTOR       (FLASH_SIZE_BYTE_PER_SECTOR/sizeof(ULONG)) /* word = 4 byte */
/* Exported variables --------------------------------------------------------*/
LX_NOR_FLASH W25Q64FV;
/* Exported functions --------------------------------------------------------*/

static ULONG sector_buffer[FLASH_WORDS_PER_SECTOR];   /* Creat ram buffer */

/**
 * @brief lx read norflash
*/
static UINT lx_nor_flash_driver_read(ULONG *flash_address, ULONG *destination, ULONG words)
{

    
    return (LX_SUCCESS);
}

/**
 * @brief lx write norflash
*/
static UINT lx_nor_flash_driver_write(ULONG *flash_address, ULONG *source, ULONG words)
{

    
    return (LX_SUCCESS);
}

/**
 * @brief lx erase norflash
*/
static UINT lx_nor_flash_driver_block_erase(ULONG block, ULONG erase_count)
{
    LX_PARAMETER_NOT_USED(erase_count);
    

    
    return (LX_SUCCESS);
}

/**
 * @brief lx check erase norflash success or not 
*/
static UINT lx_nor_flash_driver_block_erased_verify(ULONG block)
{
    LX_PARAMETER_NOT_USED(block);
    
    return (LX_SUCCESS);
}

/**
 * @brief check systerm error, depend driver programmer
*/
static UINT lx_nor_flash_driver_system_error(UINT error_code)
{
    LX_PARAMETER_NOT_USED(error_code);
    
    return (LX_ERROR);
}   

/**
 * @brief levelx nor driver initialize
*/
static UINT nor_driver_initialize(LX_NOR_FLASH *nf)
{
    /* Setup the base address of the flash memory.  */
    nf->lx_nor_flash_base_address               = (ULONG *)FLASH_BASE_ADDR;

    /* Setup geometry of the flash.  */
    nf->lx_nor_flash_total_blocks               = FLASH_TOTAL_BLOCKS;
    nf->lx_nor_flash_words_per_block            = FLASH_WORDS_PER_SECTOR;
    
    /* Setup  driver’s initialization function */
    nf->lx_nor_flash_driver_read                = lx_nor_flash_driver_read;
    nf->lx_nor_flash_driver_write               = lx_nor_flash_driver_write;
    nf->lx_nor_flash_driver_block_erase         = lx_nor_flash_driver_block_erase;
    nf->lx_nor_flash_driver_block_erased_verify = lx_nor_flash_driver_block_erased_verify;
    nf->lx_nor_flash_driver_system_error        = lx_nor_flash_driver_system_error;

    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nf->lx_nor_flash_sector_buffer              = sector_buffer;

    return(LX_SUCCESS);
}

#if 1

void LX_NOR_FLASH_Init(void)
{
    _lx_nor_flash_initialize();

    _lx_nor_flash_open(&W25Q64FV, FLASH_NAME, nor_driver_initialize);
}

void LX_NOR_FLASH_ReadWrite_Test(void)
{

    // printf("lx read %s\r\n", rbuf);
}

#endif
