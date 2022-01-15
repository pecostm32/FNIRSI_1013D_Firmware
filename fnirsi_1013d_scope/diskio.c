//----------------------------------------------------------------------------------------------------------------------------------
//Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019       
//----------------------------------------------------------------------------------------------------------------------------------
//If a working storage control module is available, it should be       
//attached to the FatFs via a glue function rather than modifying it.  
//This is an example of glue functions to attach various existing     
//storage control modules to the FatFs module with a defined API.      
//----------------------------------------------------------------------------------------------------------------------------------

#include "ff.h"        //Obtains integer types
#include "diskio.h"    //Declarations of disk functions

#include "sd_card_interface.h"

//Definitions of physical drive number for each drive
#define DEV_SD     0 

//Number of sectors on the card
extern uint32 cardsectors;

//----------------------------------------------------------------------------------------------------------------------------------
//Get Drive Status                                
//
//Return:
//  DSTATUS
//
//Input:
//  Physical drive number to identify the drive
//
//----------------------------------------------------------------------------------------------------------------------------------
DSTATUS disk_status(BYTE pdrv)
{
  //Disk is always ok
  return(RES_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------
//Initialize a Drive                                                   
//
//Return:
// DSTATUS
//
//Input:
//  Physical drive number to identify the drive
//
//----------------------------------------------------------------------------------------------------------------------------------
DSTATUS disk_initialize(BYTE pdrv)
{
  //Check if the SD card device is addressed
  if(pdrv == DEV_SD)
  {
    //Try to initialize the interface and the card
    if(sd_card_init() != SD_OK)
    {
      //Some error then signal no disk
      return(STA_NODISK);
    }
  }
  else
  {
    //Not the SD card drive then no init
    return(STA_NOINIT);
  }
  
  //All went well
  return(RES_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------
//Read Sector(s)                                                       
//
//Return:
// DRESULT
//
//Input:
//  Physical drive number to identify the drive
//  Data buffer to store read data
//  Start sector in LBA
//  Number of sectors to read
//
//----------------------------------------------------------------------------------------------------------------------------------
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
  //Check if the SD card device is addressed
  if(pdrv == DEV_SD)
  {
    //Read the data from the card
    if(sd_card_read(sector, count, buff) != SD_OK)
    {
      //Error while reading
      return(RES_ERROR);
    }
  }
  else
  {
    //Not the SD card drive selected
    return(RES_PARERR);
  }
  
  //All went well
  return(RES_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

#if FF_FS_READONLY == 0

//----------------------------------------------------------------------------------------------------------------------------------
//Write Sector(s)                                                      
//
//Return:
//  DRESULT
//
//Input:
//  Physical drive number to identify the drive
//  Data buffer to be written
//  Start sector in LBA
//  Number of sectors to read
//
//----------------------------------------------------------------------------------------------------------------------------------
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
  //Check if the SD card device is addressed
  if(pdrv == DEV_SD)
  {
    //Write the data to the card
    if(sd_card_write(sector, count, (BYTE *)buff) != SD_OK)
    {
      //Error while writing
      return(RES_ERROR);
    }
  }
  else
  {
    //Not the SD card drive selected
    return(RES_PARERR);
  }
  
  //All went well
  return(RES_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

#endif

//----------------------------------------------------------------------------------------------------------------------------------
//Miscellaneous Functions                                              
//----------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------
//IO control
//
//Return:
//  DRESULT
//
//Input:
//  Physical drive number to identify the drive
//  Control code
//  Buffer to send/receive control data
//
//----------------------------------------------------------------------------------------------------------------------------------
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
  //Check if the SD card device is addressed
  if(pdrv == DEV_SD)
  {
    if(cmd == CTRL_SYNC)
    {
      return(RES_OK);
    }
    else if(cmd == GET_SECTOR_COUNT)
    {
      //Check if buffer is valid
      if(buff)
      {
        *(uint32 *)buff = cardsectors;

        return(RES_OK);
      }
    }
  }
  
  //Invalid input parameters given
  return(RES_PARERR);
}

//----------------------------------------------------------------------------------------------------------------------------------
//get a fixed value for the date and time                                            
//
//Return:
//  Date time value
//
//Input:
//
//----------------------------------------------------------------------------------------------------------------------------------
DWORD get_fattime (void)
{
  //Some date and time value
  return(1349957149);
}
