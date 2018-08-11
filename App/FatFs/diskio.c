/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "SDIO.h"
#include "W25Qxx.h"
#include "Malloc.h"
#include "SysTick.h"
#include "UART.h"


/* Definitions of physical drive number for each drive */
#define SD_CARD	 0  //SD卡,卷标为0
#define EX_FLASH 1	//外部flash,卷标为1
#define USB		   2	/* Example: Map USB MSD to physical drive 2 */

#define SD_BLOCKSIZE      512

//对于W25Q128
//前12M字节给fatfs用,12M字节后,用于存放字库,字库占用3.09M.	剩余部分,给客户自己用	 
#define FLASH_SECTOR_SIZE 512
u16 FLASH_SECTOR_COUNT = 2048 * 12;
#define FLASH_BLOCK_SIZE  8

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
  	DSTATUS stat = STA_NOINIT;

	switch (pdrv) {
	case SD_CARD :
    stat &= ~STA_NOINIT;

		break;

	case EX_FLASH :
    stat &= ~STA_NOINIT;

		break;

	case USB :

		break;
	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
  int result;

	switch (pdrv) {
	case SD_CARD :
    
    if(InitSD() == SD_OK)
    {
      stat &= ~STA_NOINIT;
    }
    else
    {
      stat = STA_NOINIT;
    }
    
    break;

	case EX_FLASH :
    
    InitW25Qxx();
    DelayNus(3);
    W25QxxWakeUp();
  
    while((result = W25QxxReadID())!= W25Q128)
    {
      
    }
    
    printf("Flash ID:%d\r\n", result);
    
    stat &= ~STA_NOINIT;
    FLASH_SECTOR_COUNT = 2049 * 12; //前12M字节给fatfs用
    
    break;

	case USB :
      
    break;
	}
  
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;
  u8 SDstatus;
  
	switch (pdrv) {
	case SD_CARD :

    SDstatus = ReadSDDisk(buff, sector, count);
    if(SDstatus == SD_OK)
    {
      res = RES_OK;
    }
    else
    {
      InitSD();
      SDstatus = ReadSDDisk(buff, sector, count);
      if(SDstatus != SD_OK)
      {
        res = RES_ERROR;
      }
    }
      
      break;

	case EX_FLASH :
    
		for(; count > 0; count--)
    {
      W25QxxRead(buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
      sector++;
      buff += FLASH_SECTOR_SIZE;
    }
      
    res = RES_OK;

		break;

	case USB :
		
		break;
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;
  u8 SDstatus;

	switch (pdrv) {
	case SD_CARD :
    
		SDstatus = WriteSDDisk((u8*)buff, sector, count);
  
    if(SDstatus == SD_OK)
    {
      res =RES_OK;
    }
    else
    {
      InitSD();
      SDstatus = WriteSDDisk((u8*)buff, sector, count);
      
      if(SDstatus != SD_OK)
      {
        res = RES_ERROR;
      }
    }
    
    break;

	case EX_FLASH :
		for(; count > 0; count--)
    {
      W25QxxWrite((u8*)buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
			sector++;
			buff += FLASH_SECTOR_SIZE;
    }
    res = RES_OK;

		break;

	case USB :
		
    break;
	}

	return res;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (pdrv) {
	case SD_CARD :
    
    switch(cmd)
    {
      case GET_SECTOR_SIZE:
        *(WORD*)buff = SD_BLOCKSIZE;
      
        break;
      case GET_BLOCK_SIZE:
        *(DWORD*)buff = SDCardInfo.CardBlockSize;
      
        break;
      case GET_SECTOR_COUNT:
        *(DWORD*)buff = SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;
      
        break;
      case CTRL_SYNC:
        break;
    }
    res = RES_OK;
    
    break;

	case EX_FLASH :
    
      switch(cmd)
      {
        case GET_SECTOR_SIZE:
          *(WORD*)buff = FLASH_SECTOR_SIZE;
        
          break;
        case GET_BLOCK_SIZE:
          *(DWORD*)buff = FLASH_BLOCK_SIZE;
        
          break;
        case GET_SECTOR_COUNT:
          *(DWORD*)buff = FLASH_SECTOR_COUNT;
        
          break;
        case CTRL_SYNC:
          break;
      }
      res = RES_OK;
      
      break;

	case USB :

		// Process of the command the USB drive

		break;
	}

	return res;
}
#endif

//获得时间
//User defined function to give a current time to fatfs module      */
//31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */                                                                                                                                                                                                                                          
//15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */  
DWORD get_fattime (void)
{
  return ((DWORD)(2018 - 1980) << 25)
    | ((DWORD)1 << 21)
    | ((DWORD)1 << 16)
    | ((DWORD)0 << 11)
    | ((DWORD)0 << 5)
    | ((DWORD)0 << 1);
}

//动态分配内存
void *ff_memalloc(UINT size)
{
  return (void*)MallocMemExt(USER_DEFINE_SRAMIN, size);
}

//释放内存
void ff_memfree(void* mf)
{
  FreeMemExt(USER_DEFINE_SRAMIN, mf);
}

