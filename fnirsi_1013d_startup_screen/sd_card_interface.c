//----------------------------------------------------------------------------------------------------------------------------------

#include "sd_card_interface.h"
#include "ccu_control.h"
#include "timer.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

//need some global variables for type of card indication
uint32 cardtype = SD_CARD_TYPE_NONE;
uint32 cardrca = 0;

uint32 cardsectorsize = 512;
uint32 cardsectors = 0;
uint32 cardsize = 0;

uint8 cardmid = 0;
uint8 cardpnm[5];

uint32 cardpsn = 0;

uint32 cardcid[4] = { 0, 0, 0, 0 };
uint32 cardcsd[4] = { 0, 0, 0, 0 };

SD_CARD_COMMAND sd_command;
SD_CARD_DATA    sd_data;

uint32 sd_buffer[1024];  //4KB data buffer. Defined as uint32 to assure dword alignment

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_init(void)
{
  int32 result;
  
  //Clear the command and data structure
  memset(&sd_command, 0, sizeof(SD_CARD_COMMAND));
  memset(&sd_data, 0, sizeof(SD_CARD_DATA));
  
  //Configure the port F IO pins since the SD card is connected there
  *PORT_F_CFG_REG = 0xFF222222;
  
  //De-assert SPI0 reset
  *CCU_BUS_SOFT_RST0 |= CCU_BSRR0_SD0_RST;

  //Open the SPI0 bus gate
  *CCU_BUS_CLK_GATE0 |= CCU_BCGR0_SD0_EN;
  
  //Wait a short wile for the device to become active
  sd_card_delay(1);
  
  //Setup the SD card clock at 400KHz
  sd_card_clk_init(400000);
  
  //Reset the SD card interface
  *SD0_HWRST &= ~SD_HWRST_ACTIVE;
  
  //Wait a while for the device to be reset
  sd_card_delay(50);

  //Make the SD card interface active again
  *SD0_HWRST |= SD_HWRST_ACTIVE;

  //Wait a while for the device to become active again
  sd_card_delay(50);
  
  //Reset the DMA, FIFO and controller
  *SD0_GCTL |= (SD_GCTL_DMA_RST | SD_GCTL_FIFO_RST | SD_GCTL_SOFT_RST);
  
  //Wait a while for the system to be done resetting
  sd_card_delay(50);
  
  //Disable card detect de-bounce
  *SD0_GCTL &= ~SD_GCTL_CD_DBC_ENB;
  
  //Turn card clock of when FSM in idle state
  *SD0_CKCR |= SD_CKCR_CCLK_CTRL;
  
  //Set the hardware to use only a single bit for data transfer
  *SD0_BWDR = SD_BWDR_1_BIT_WIDTH;
  
  //Set max timeout for data and response.
  *SD0_TMOR = 0xFFFFFFFF;

  //Turn the card clock on  
  *SD0_CKCR |= SD_CKCR_CCLK_ENB;
  
  //Update the card clock
  sd_card_update_clock();

  //Wait a while for the clock to stabilize 
  sd_card_delay(50);
  
  //Send reset command to the card
  sd_command.cmdidx    = 0;
  sd_command.cmdarg    = 0;
  sd_command.resp_type = SD_RESPONSE_NONE;
  sd_card_send_command(&sd_command, 0);

  //Wait a while for the card to reset
  sd_card_delay(50);
  
  //Send card interface condition command to the card
  sd_command.cmdidx    = 8;
  sd_command.cmdarg    = 0x00000155;        //31:12 reserved (0x00000), 11:8 supply voltage (0x1 = 2.7 - 3.6V), 7:0 check pattern (0x55)
  sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);
  
  //Check if version 2.00 or later SD memory card
  //On result OK (0) it probably is a SDSC, SDHC or SDXC card
  if(result == SD_OK)
  {
    //Might need to check on returned information of command 8
    //Response should match the given command arg
    //Card could be not usable???
    
    //Get the card type
    do
    {
      //Send application specific command follows command to the card
      sd_command.cmdidx    = 55;
      sd_command.cmdarg    = 0;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      sd_card_send_command(&sd_command, 0);

      //Send host capacity support information command
      sd_command.cmdidx    = 41;
      sd_command.cmdarg    = 0x40FF8000;                      //Need to figure out these settings
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);
      
    //0 means still initializing  
    } while((sd_command.response[3] & 0x80000000) == 0);

    //Check card capacity status. 1 means SDHC or SDXC. 0 means SDSC
    if(sd_command.response[3] & 0x40000000)
    {
      //Signal TYPE_SD_HIGH...
      cardtype = SD_CARD_TYPE_SDHC;
    }
    else
    {
      //Signal TYPE_SD_LOW...
      cardtype = SD_CARD_TYPE_SDSC;
    }
  }
  else
  {
    //Version 1.1 or earlier card
    //Send reset command to the card
    sd_command.cmdidx    = 0;
    sd_command.cmdarg    = 0;
    sd_command.resp_type = SD_RESPONSE_NONE;
    sd_card_send_command(&sd_command, 0);

    //Wait a wile for the card to reset
    sd_card_delay(500);
    
    //Send application specific command follows command to the card
    sd_command.cmdidx    = 55;
    sd_command.cmdarg    = 0;
    sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, 0);
    
    //Check if there was a timeout condition
    //In the original code the wrong value is used. The sunxi code uses -ETIMEDOUT (0xFFFFFF68). In the scope code they check on ETIMEDOUT (0x98)
    //Is to determine the type of card being used. Not sure if it is actually needed since mmc cards don't fit in micro sd slots. (At least for what I found about it)
    if(result == SD_ERROR_TIMEOUT)
    {
      //Send reset command to the card
      sd_command.cmdidx    = 0;
      sd_command.cmdarg    = 0;
      sd_command.resp_type = SD_RESPONSE_NONE;
      sd_card_send_command(&sd_command, 0);
    
      //Wait a wile for the card to reset
      sd_card_delay(50);

      //Send multi media card command to the card
      sd_command.cmdidx    = 1;
      sd_command.cmdarg    = 0x80FF8000;                           //Need to figure out these settings
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //On timeout there is no valid card inserted
      if(result == SD_ERROR_TIMEOUT)
      {
        //No valid card so signal this in the type variable
        cardtype = SD_CARD_TYPE_NONE;
        
        return(SD_ERROR);
      }

      //Wait for the card to become active
      while((sd_command.response[3] & 0x80000000) == 0)
      {
        //Send multi media card command to the card
        sd_command.cmdidx    = 1;
        sd_command.cmdarg    = 0x80FF8000;                           //Need to figure out these settings
        sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
        sd_card_send_command(&sd_command, 0);
      }

      //Signal TYPE_MMC...
      cardtype = SD_CARD_TYPE_MMC;
    }
    else if(result)
    {
      //No valid card so signal this in the type variable
      cardtype = SD_CARD_TYPE_NONE;

      return(SD_ERROR);
    }
    else
    {
      //Send host capacity support information command
      sd_command.cmdidx    = 41;
      sd_command.cmdarg    = 0x00FF8000;                      //Need to figure out these settings
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //Wait for the card to become active
      while((sd_command.response[3] & 0x80000000) == 0)
      {
        //Send application specific command follows command to the card
        sd_command.cmdidx    = 55;
        sd_command.cmdarg    = 0;
        sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
        sd_card_send_command(&sd_command, 0);
       
        //Send host capacity support information command
        sd_command.cmdidx    = 41;
        sd_command.cmdarg    = 0x80FF8000;                      //Need to figure out these settings
        sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
        result = sd_card_send_command(&sd_command, 0);
      }      
      
      //Signal TYPE_SD_LOW...
      cardtype = SD_CARD_TYPE_SDSC;
    }
  }
  
  //Only if a card has been detected
  if(cardtype)
  {
    //Send get CID numbers command to the card
    sd_command.cmdidx    = 2;
    sd_command.cmdarg    = 0;
    sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
    sd_card_send_command(&sd_command, 0);
  
    //Save the numbers
    cardcid[0] = sd_command.response[3];
    cardcid[1] = sd_command.response[2];
    cardcid[2] = sd_command.response[1];
    cardcid[3] = sd_command.response[0];
    
    //Check if the card is a mmc type
    if(cardtype == SD_CARD_TYPE_MMC)
    {
      //Send publish RCA command to the card
      sd_command.cmdidx    = 3;
      sd_command.cmdarg    = 0x10000;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);
      
      if(result)
      {
        return(SD_ERROR);
      }
      
      sd_command.response[3] = 0x10000;
    }
    else
    {
      //Send publish RCA command to the card
      sd_command.cmdidx    = 3;
      sd_command.cmdarg    = 0;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);
      
      if(result)
      {
        return(SD_ERROR);
      }
    }
    
    cardrca = sd_command.response[3];
  }

  //Get the card specifications  
  if(sd_card_get_specifications())
  {
    //Signal error on failure
    return(SD_ERROR);
  }

  //Set the card clock to 48MHz and use the 4 bit bus if supported
  if(sd_card_set_clock_and_bus(1))
  {
    //Signal error on failure
    return(SD_ERROR);
  }

  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_read(uint32 sector, uint32 blocks, uint8 *buffer)
{
  int32 result;
  
  //Check if valid buffer given
  if(buffer == 0)
    return(SD_ERROR_INVALID_BUFFER);
  
  //This might be wrong. Need testing with last sector!!!!!
  //Check if last bytes to read in range of the card sectors
  if((sector + blocks - 1) > cardsectors)
    return(SD_ERROR_SECTOR_OUT_OF_RANGE);
  
  //Send card select command
  sd_command.cmdidx    = 7;
  sd_command.cmdarg    = cardrca;
  sd_command.resp_type = SD_RESPONSE_BUSY | SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);

  //Only continue when card selected without errors
  if(result == SD_OK)
  {
    //Prepare data buffer for reading
    sd_data.blocks    = blocks;
    sd_data.blocksize = 512;
    sd_data.flags     = SD_DATA_READ;
    sd_data.data      = buffer;
    
    //Send read command based on number of blocks
    if(blocks == 1)
    {
      //Set read single block command
      sd_command.cmdidx = 17;
    }
    else
    {
      //Set read multiple blocks command
      sd_command.cmdidx = 18;
    }
    
    //Indicate which sector to start reading from
    if(cardtype != SD_CARD_TYPE_SDHC)
    {
      //For non HC type cards use the byte address
      sd_command.cmdarg = sector << 9;
    }
    else
    {
      //For HC type card use the sector address
      sd_command.cmdarg = sector;
    }
    
    //Card allowed to be busy.
    sd_command.resp_type = SD_RESPONSE_BUSY | SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, &sd_data);
    
    //Deselect the card if all went well
    if(result == SD_OK)
    {
      //Send deselect card command to the card
      sd_command.cmdidx    = 7;
      sd_command.cmdarg    = 0;
      sd_command.resp_type = SD_RESPONSE_NONE;
      result = sd_card_send_command(&sd_command, 0);
    }
  }
  
  return(result);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_write(uint32 sector, uint32 blocks, uint8 *buffer)
{
  int32 result;
  
  //Check if valid buffer given
  if(buffer == 0)
    return(SD_ERROR_INVALID_BUFFER);
  
  //Check if last bytes to read in range of the card sectors
  if((sector + blocks - 1) > cardsectors)
    return(SD_ERROR_SECTOR_OUT_OF_RANGE);
  
  //Send card select command
  sd_command.cmdidx    = 7;
  sd_command.cmdarg    = cardrca;
  sd_command.resp_type = SD_RESPONSE_BUSY | SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);

  //Only continue when card selected without errors
  if(result == SD_OK)
  {
    //Prepare data buffer for writing
    sd_data.blocks    = blocks;
    sd_data.blocksize = 512;
    sd_data.flags     = SD_DATA_WRITE;
    sd_data.data      = buffer;
    
    //Send write command based on number of blocks
    if(blocks == 1)
    {
      //Set write single block command
      sd_command.cmdidx = 24;
    }
    else
    {
      //Set write multiple blocks command
      sd_command.cmdidx = 25;
    }
    
    //Indicate which sector to start reading from
    if(cardtype != SD_CARD_TYPE_SDHC)
    {
      //For non HC type cards use the byte address
      sd_command.cmdarg = sector << 9;
    }
    else
    {
      //For HC type card use the sector address
      sd_command.cmdarg = sector;
    }
    
    //Card allowed to be busy.
    sd_command.resp_type = SD_RESPONSE_BUSY | SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, &sd_data);
    
    //Deselect the card if all went well
    if(result == SD_OK)
    {
      //Send deselect card command to the card
      sd_command.cmdidx    = 7;
      sd_command.cmdarg    = 0;
      sd_command.resp_type = SD_RESPONSE_NONE;
      result = sd_card_send_command(&sd_command, 0);
    }
  }
  
  return(result);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_get_specifications(void)
{
  int32 result;
  
  //Send get card specific data command to the card
  sd_command.cmdidx    = 9;
  sd_command.cmdarg    = cardrca;
  sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);
  
  //Check if card failed
  if(result)
  {
    return(result);
  }
  
  //Save the CSD
  cardcsd[0] = sd_command.response[3];
  cardcsd[1] = sd_command.response[2];
  cardcsd[2] = sd_command.response[1];
  cardcsd[3] = sd_command.response[0];
  
  //Check the version of the CSD_STRUCTURE
  if(((cardcsd[0] & 0xC0000000) == 0) || (cardtype == SD_CARD_TYPE_MMC))
  {
    //Version 1.0 or MMC card
    //Card size is calculated based on (C_SIZE + 1) * (2^(C_SIZE_MULT + 2)) * (2^READ_BL_LEN)
    //Response bits 73:64, 49:47, 83:80
    //Number of 512 byte blocks the card has
    cardsectors = ((((cardcsd[1] & 0x03FF) << 2) | (cardcsd[2] >> 30)) + 1) << (((cardcsd[2] >> 15) & 0x07) + 2) << ((cardcsd[1] >> 16) & 0x0F) >> 9;
    
    //Number of KBytes the card has
    cardsize = cardsectors >> 2;
  }
  else
  {
    //Version 2.0 or higher type card
    //Number of KBytes the card has
    cardsize = ((((cardcsd[1] & 0x03F) << 16) | (cardcsd[2] >> 16)) + 1) * 512;
    
    //Number of 512 byte blocks the card has
    cardsectors = cardsize * 2;
  }

  //Send get card identification data command to the card
  sd_command.cmdidx    = 10;
  sd_command.cmdarg    = cardrca;
  sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_136 | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);
  
  //Check if card failed
  if(result)
  {
    return(result);
  }
  
  //Save the CID
  cardcid[0] = sd_command.response[3];
  cardcid[1] = sd_command.response[2];
  cardcid[2] = sd_command.response[1];
  cardcid[3] = sd_command.response[0];
  
  //Decode the CID
  //Get the manufacturer ID
  cardmid = cardcid[0] >> 24;
  
  //Get the product name
  cardpnm[0] = cardcid[0] & 0xFF;
  cardpnm[1] = (cardcid[1] >> 24) & 0xFF;
  cardpnm[2] = (cardcid[1] >> 16) & 0xFF;
  cardpnm[3] = (cardcid[1] >> 8) & 0xFF;
  cardpnm[4] = cardcid[1] & 0xFF;
  
  //Get the product serial number
  cardpsn = (cardcid[2] << 8) | ((cardcid[3] >> 24) & 0xFF);

  //The original code uses data from it seems an invalid location so this is my interpretation of what is needed, if needed at all.
  
  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_set_clock_and_bus(int32 usewidebus)
{
  int32 result;
  
  //Send select card command to the card
  sd_command.cmdidx    = 7;
  sd_command.cmdarg    = cardrca;
  sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);
  
  //Check if card not selected
  if(result)
  {
    return(result);
  }

  //This code can be simplified since parts of the code for the two types are the same
  //The usewidebus select could also be dropped since it is always used with this (only called from init function!!!
  
  //For SDHC cards perform some dedicated steps
  if(cardtype == SD_CARD_TYPE_SDHC)
  {
    //Send application specific command follows command to the card
    sd_command.cmdidx    = 55;
    sd_command.cmdarg    = cardrca;
    sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, 0);

    //Fail if card does not respond properly    
    if(result)
    {
      return(result);
    }
    
    //Prepare data buffer for reading
    sd_data.blocks    = 1;
    sd_data.blocksize = 8;
    sd_data.flags     = SD_DATA_READ;
    sd_data.data      = (uint8 *)sd_buffer;
    
    //Send read sd configuration register command
    sd_command.cmdidx    = 51;
    sd_command.cmdarg    = 0;
    sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, &sd_data);
    
    //Fail if card does not respond properly    
    if(result)
    {
      return(result);
    }
    
    //Check on the specification version (SD_SPEC) being 2
    if((*sd_data.data & 0x0F) == 2)
    {
      //See if the card allows switching to high speed mode
      if(sd_card_check_switchable_function() == SD_OK)
      {
        //Yes so switch it to 48MHz
        if((result = sd_card_change_clk(48000000)))
        {
          //When this failed signal that
          return(result);
        }
      }
    }

    //Check if 4 bit bus should be used
    if(usewidebus)
    {
      //Send application specific command follows command to the card
      sd_command.cmdidx    = 55;
      sd_command.cmdarg    = cardrca;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //Fail if card does not respond properly    
      if(result)
      {
        return(result);
      }
      
      //Send set bus width command
      sd_command.cmdidx    = 6;
      sd_command.cmdarg    = 2;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //Fail if card does not respond properly    
      if(result)
      {
        return(result);
      }
    }
  }
  //Check if SDSC type and set clock for it if so
  else if(cardtype == SD_CARD_TYPE_SDSC)
  {
    //Yes so switch it to 48MHz
    if((result = sd_card_change_clk(48000000)))
    {
      //When this failed signal that
      return(result);
    }
    
    //Check if 4 bit bus should be used
    if(usewidebus)
    {
      //Send application specific command follows command to the card
      sd_command.cmdidx    = 55;
      sd_command.cmdarg    = cardrca;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //Fail if card does not respond properly    
      if(result)
      {
        return(result);
      }
      
      //Send set bus width command
      sd_command.cmdidx    = 6;
      sd_command.cmdarg    = 2;
      sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
      result = sd_card_send_command(&sd_command, 0);

      //Fail if card does not respond properly    
      if(result)
      {
        return(result);
      }
    }
  }
  
  //Check if 4 bit bus should be used
  if(usewidebus)
  {
    //Switch to 4 bit bus width
    *SD0_BWDR = 1;
  }
  
  //Send set block size to 512 bytes command
  sd_command.cmdidx    = 16;
  sd_command.cmdarg    = 0x200;
  sd_command.resp_type = SD_RESPONSE_BUSY | SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, 0);

  //Fail if card does not respond properly    
  if(result)
  {
    return(result);
  }
  
  //Send deselect card command to the card
  sd_command.cmdidx    = 7;
  sd_command.cmdarg    = 0;
  sd_command.resp_type = SD_RESPONSE_NONE;
  result = sd_card_send_command(&sd_command, 0);
  
  //Check if card not selected
  if(result)
  {
    return(result);
  }
  
  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_check_switchable_function(void)
{
  int32 result;
  
  //Prepare data buffer for reading
  sd_data.blocks    = 1;
  sd_data.blocksize = 64;
  sd_data.flags     = SD_DATA_READ;
  sd_data.data      = (uint8 *)sd_buffer;

  //Send read sd configuration register command
  sd_command.cmdidx    = 6;
  sd_command.cmdarg    = 0x00FFFF01;
  sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
  result = sd_card_send_command(&sd_command, &sd_data);

  //Fail if card does not respond properly    
  if(result)
  {
    return(result);
  }
  
  //Check if capable of switching
  if((sd_data.data[0] | sd_data.data[1]) && ((sd_data.data[28] | sd_data.data[29]) == 0))
  {
    //Need to check if this needs to be done again!!!! (Check if not modified in any way by other code)
    //Prepare data buffer for reading
    sd_data.blocks    = 1;
    sd_data.blocksize = 64;
    sd_data.flags     = SD_DATA_READ;
    sd_data.data      = (uint8 *)sd_buffer;

    //Can also reduce on the setting of the other two variables since they are already set before!!!
    //Send read sd configuration register command with additional bit in argument
    sd_command.cmdidx    = 6;
    sd_command.cmdarg    = 0x80FFFF01;
    sd_command.resp_type = SD_RESPONSE_CRC | SD_RESPONSE_PRESENT;
    result = sd_card_send_command(&sd_command, &sd_data);
    
    //Fail if card does not respond properly    
    if(result)
    {
      return(result);
    }

    //Check again if still capable???
    if((sd_data.data[0] | sd_data.data[1]) && ((sd_data.data[28] | sd_data.data[29]) == 0))
    {
      return(SD_OK);
    }
  }
  
  return(SD_NOT_CAPABLE);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_clk_init(uint32 frequency)
{
  uint32 clock;
  uint32 div;
  uint32 n_ratio;
  
	uint32 oclk_dly = 0;
	uint32 sclk_dly = 0;
  
  //Check if pll clock needs to be used for the SD card interface
  if(frequency > 24000000)
  {
    //Set clock source to PLL_PERIPH and reset the other settings
    *CCU_SDMMC0_CLK = CCU_SDMMC0_CLK_SRC_PLL_PERIPH;

    //The input clock is 576MHz (Set in ccu_control)
    clock = 576000000;
  }
  else
  {
    //Set clock source to OSC24M and reset the other settings too
    *CCU_SDMMC0_CLK = CCU_SDMMC0_CLK_SRC_OSC24M;
    
    //Set the clock frequency to 24MHz
    clock = 24000000;
  }
  
  //Calculate the clock divider
  div = clock / frequency;
  
  //For an exact multiple one needs to be added since it is subtracted later
  if(clock % frequency)
    div++;
  
  //Start with pre divide of 1
  n_ratio = 0;
  
  //Divider can't be bigger then 16, so for each multiple increase the pre divide ratio
  while(div > 16)
  {
    n_ratio++;
    div = (div + 1) / 2;
  }
  
  //If the ratio is more then 3 the clock can't be set (could when the card clock divider is used)
  if(n_ratio > 3)
    return(SD_ERROR);
  
  //Determine delays
  //Not sure what these do. Manual is not clear on this, but the original code uses it.
  if(frequency > 4000000)
  {
    if(frequency > 25000000)
    {
      if(frequency > 50000000)
      {
        //frequency > 50MHz
        oclk_dly = 1;
      }
      else
      {
        //frequency <= 50MHz
        oclk_dly = 3;
      }
      
      //For frequency above 25MHz this needs to be 4
      sclk_dly = 4;
    }
    else
    {
      //frequency <= 25MHz but > 400KHz
      sclk_dly = 5;
    }
  }
  
  //Set the clock parameters and enable it
  *CCU_SDMMC0_CLK |= (CCU_SDMMC0_CLK_ENABLE | (sclk_dly << 20) | (n_ratio << 16) | (oclk_dly << 8) | (div - 1));
  
  //Nothing left to do and no errors so say it is ok
  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_change_clk(uint32 frequency)
{
  //Disable the card clock
  *SD0_CKCR &= ~SD_CKCR_CCLK_ENB;
  
  //Update the interface
  if(sd_card_update_clock() == SD_OK)
  {
    //No error then set the new clock value
    if(sd_card_clk_init(frequency) == SD_OK)
    {
      //Still no error then clear the card clock divider
      *SD0_CKCR &= SD_CKCR_CCLK_CLR_DIV;
      
      //Re-enable the clock
      *SD0_CKCR |= SD_CKCR_CCLK_ENB;
      
      //Update the interface again
      if(sd_card_update_clock() == SD_OK)
      {
        //On success return success
        return(SD_OK);
      }
    }
  }
  
  //Failed so signal that
  return(SD_ERROR);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_update_clock(void)
{
  uint32 timeout;
  
  //Update the card clock
  *SD0_CMDR = SD_CMD_START | SD_CMD_UPCLK_ONLY | SD_CMD_WAIT_PRE_OVER;
  
  //Setup for max 2 second wait
  timeout = timer0_get_ticks() + 2000;

  //Wait for the clock update to be done
  while(*SD0_CMDR & SD_CMD_START)
  {
    //Check on timeout
    if(timer0_get_ticks() > timeout)
    {
      return(SD_ERROR_TIMEOUT);
    }
  }
  
  //All went well
  return(SD_OK);  
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_send_command(PSD_CARD_COMMAND command, PSD_CARD_DATA data)
{
	uint32 cmdval = SD_CMD_START;
  int32  error = SD_OK;
  uint32 timeout;
  
  //A command is always needed
  if(command == 0)
  {
    return(SD_ERROR);
  }
  
  //Don't send stop transmission command ???
  if(command->cmdidx == 12)
  {
    return(SD_OK);
  }
  
  if(command->cmdidx == 0)
  {
    cmdval |= SD_CMD_SEND_INIT_SEQ;
  }
  
  if(command->resp_type & SD_RESPONSE_PRESENT)
  {
		cmdval |= SD_CMD_RESP_EXPIRE;
  }
  
  if(command->resp_type & SD_RESPONSE_136)
  {
		cmdval |= SD_CMD_LONG_RESPONSE;
  }

  if(command->resp_type & SD_RESPONSE_CRC)
  {
		cmdval |= SD_CMD_CHK_RESPONSE_CRC;
  }
  
  //Check if there is data in the mix
  if(data)
  {
    cmdval |= (SD_CMD_DATA_EXPIRE | SD_CMD_WAIT_PRE_OVER);
    
    if(data->flags & SD_DATA_WRITE)
    {
      cmdval |= SD_CMD_WRITE;
    }
    
    if(data->blocks > 1)
    {
      cmdval |= SD_CMD_AUTO_STOP;
    }
    
    *SD0_BKSR = data->blocksize;
    *SD0_BYCR = data->blocks * data->blocksize;
  }

  //Load the SD interface command argument and command register
  *SD0_CAGR = command->cmdarg;
  *SD0_CMDR = command->cmdidx | cmdval;

  //See if data needs to be written or read
  if(data)
  {
    //The original code is different!!!!!
    //For larger blobs of data it uses DMA
    
    //Send or receive the data using the cpu
    if((error = sd_send_data(data)))
    {
      goto out;
    }
  }
  
  //Wait for the command to finish
  if((error = sd_rint_wait(1000, SD_RINT_COMMAND_DONE)))
  {
    goto out;
  }

  //See if data is involved
  if(data)
  {
    //Depending on the number of blocks wait for either auto command done or data transfered
    if(data->blocks > 1)
      cmdval = SD_RINT_AUTO_COMMAND_DONE;
    else
      cmdval = SD_RINT_DATA_OVER;
      
    //Wait for the data to finish
    if((error = sd_rint_wait(120, cmdval)))
    {
      goto out;
    }
  }

  //Check if card is allowed to send busy
  if(command->resp_type & SD_RESPONSE_BUSY)
  {
    //Setup for max 2 second wait
    timeout = timer0_get_ticks() + 2000;
    
    //Wait for the card to be done
    while(*SD0_STAR & SD_STATUS_CARD_DATA_BUSY)
    {
      //Check on timeout
      if(timer0_get_ticks() > timeout)
      {
        error = SD_ERROR_TIMEOUT;
        goto out;
      }
    }
  }

  //Get the short response (48) bits 
  command->response[0] = *SD0_RESP0;
  command->response[1] = *SD0_RESP1;
  
  //Check if expected response is 136 bits
	if(command->resp_type & SD_RESPONSE_136)
  {
    //Get the remainder of the response bits if so
		command->response[2] = *SD0_RESP2;
		command->response[3] = *SD0_RESP3;
	}

out:
  //Check if there was an error
  if(error < 0)
  {
    //Reset the DMA, FIFO and controller
    *SD0_GCTL |= (SD_GCTL_DMA_RST | SD_GCTL_FIFO_RST | SD_GCTL_SOFT_RST);
    
    sd_card_update_clock();
  }
  
  //Clear all raw interrupts
  *SD0_RISR = 0xFFFFFFFF;

  //Reset the FIFO
  *SD0_GCTL |= SD_GCTL_FIFO_RST;
    
  return(error);  
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_send_data(PSD_CARD_DATA data)
{
  uint32  reading;
  uint32  status_bit;
  uint32  count;
  uint32  timeout;
  
  //Check if data being read
  if(data->flags & SD_DATA_READ)
  {
    //If so check on fifo empty status
    status_bit = SD_STATUS_FIFO_EMPTY;
    
    //Signal reading
    reading = 1;
  }
  else
  {
    //If writing check on fifo full status
    status_bit = SD_STATUS_FIFO_FULL;
    
    //Signal writing
    reading = 0;
  }
  
  //Data is transfered in words while input count is in bytes, so divide by 4
  count = (data->blocksize * data->blocks) >> 2;
  
  //Set the timeout based on the number of 256 byte blocks
  timeout = count >> 6;
  
  //Make sure it is not less then 2 seconds
  if(timeout < 2000)
  {
    timeout = 2000;
  }
  
  //Make sure fifo is accessible for the cpu
  *SD0_GCTL |= SD_GCTL_FIFO_ACCESS_AHB;
    
  //Setup timeout for checking against the timer ticks
  timeout += timer0_get_ticks();

    //To solve buffer alignment problem this part needs to be improved upon
    //The original code used malloc to get a really big buffer and copy the data into that
    //Another way of doing it would be to modify the code here to have three handling parts
    //1 for if the data is byte aligned (bit 1 being either 0 or 1 and bit 0 being 1)
    //2 for if the data is short aligned (bit 1 being 1 and bit 0 being 0)
    //3 for if the data is dword aligned (bit 1 being 0 and bit 0 being 0)
    
    //This can be done in the sd_card_send_command function calling three different send_data functions
  
  
  //Handle the data based on alignment
  switch((int32)data->data & 3)
  {
    //32 bit aligned
    case 0:
      {
        uint32 *buffer = (uint32 *)data->data;
        
        //Process all the words
        while(count)
        {
          //Wait for the fifo to be ready
          while(*SD0_STAR & status_bit)
          {
            //Check on timeout
            if(timer0_get_ticks() > timeout)
            {
              return(SD_ERROR_TIMEOUT);
            }
          }

          //Check if reading or writing of data
          if(reading)
          {
            //Reading so get data from the fifo
            *buffer = *SD0_FIFO;
          }
          else
          {
            //Writing so write to the fifo
            *SD0_FIFO = *buffer;
          }

          //Update pointer and counter
          buffer++;
          count--;
        }
      }
      break;

    //16 bit aligned
    case 2:
      {
        uint16 *buffer = (uint16 *)data->data;
        uint32  dwdata;
        
        //Process all the words
        while(count)
        {
          //Wait for the fifo to be ready
          while(*SD0_STAR & status_bit)
          {
            //Check on timeout
            if(timer0_get_ticks() > timeout)
            {
              return(SD_ERROR_TIMEOUT);
            }
          }

          //Check if reading or writing of data
          if(reading)
          {
            //Reading so get data from the fifo
            dwdata = *SD0_FIFO;
            
            //Copy the data into the buffer
            buffer[0] =  dwdata        & 0xFFFF;
            buffer[1] = (dwdata >> 16) & 0xFFFF;
          }
          else
          {
            //Gather the data for 32 bit writing into the fifo
            dwdata = buffer[0] | (buffer[1] << 16);
            
            //Writing so write to the fifo
            *SD0_FIFO = dwdata;
          }

          //Update pointer and counter
          buffer += 2;
          count--;
        }
      }
      break;
      
    //8 bit aligned
    case 1:
    case 3:
      {
        uint8  *buffer = (uint8 *)data->data;
        uint32  dwdata;
        
        //Process all the words
        while(count)
        {
          //Wait for the fifo to be ready
          while(*SD0_STAR & status_bit)
          {
            //Check on timeout
            if(timer0_get_ticks() > timeout)
            {
              return(SD_ERROR_TIMEOUT);
            }
          }

          //Check if reading or writing of data
          if(reading)
          {
            //Reading so get data from the fifo
            dwdata = *SD0_FIFO;
            
            //Copy the data into the buffer
            buffer[0] =  dwdata         & 0xFF;
            buffer[1] = (dwdata >>  8)  & 0xFF;
            buffer[2] = (dwdata >> 16)  & 0xFF;
            buffer[3] = (dwdata >> 24)  & 0xFF;
          }
          else
          {
            //Gather the data for 32 bit writing into the fifo
            dwdata = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
            
            //Writing so write to the fifo
            *SD0_FIFO = dwdata;
          }

          //Update pointer and counter
          buffer += 4;
          count--;
        }
      }
      break;
  }
  
  //All went well so return ok
  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_rint_wait(uint32 timeout, uint32 status_bit)
{
  //Setup timeout for checking against the timer ticks
  timeout += timer0_get_ticks();
  
  //Wait for the card to be ready
  while(!(*SD0_RISR & status_bit))
  {
    //Check on timeout or error
    if((timer0_get_ticks() > timeout) || (*SD0_RISR & SD_RINT_INTERRUPT_ERROR_BITS))
    {
      return(SD_ERROR_TIMEOUT);
    }
  }
  
  return(SD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

void sd_card_delay(uint32 delay)
{
  uint32 loops = delay * 40000;

  __asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"  "bne 1b":"=r"(loops):"0"(loops));
}

//----------------------------------------------------------------------------------------------------------------------------------
