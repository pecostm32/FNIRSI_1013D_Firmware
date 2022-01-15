//--------------------------------------------------------------------------------------

#include "spi_control.h"
#include "ccu_control.h"
#include "gpio_control.h"

#include <string.h>

//--------------------------------------------------------------------------------------

unsigned char sector_buffer[FLASH_SECTOR_SIZE];

//--------------------------------------------------------------------------------------

void sys_spi_flash_init(void)
{
  //Configure PC0, PC1, PC2 and PC3 for SPI0
  *PORTC_CFG0_REG = PORTC_CFG0_PIN_3_SPI0_MOSI | PORTC_CFG0_PIN_2_SPI0_MISO | PORTC_CFG0_PIN_1_SPI0_CS | PORTC_CFG0_PIN_0_SPI0_CLK;

  //De-assert SPI0 reset
  *CCU_BUS_SOFT_RST0 |= CCU_BSRR0_SPI0_RST;

  //Open the SPI0 bus gate
  *CCU_BUS_CLK_GATE0 |= CCU_BCGR0_SPI0_EN;
  
  //16-11-2021
  //The 1014D code uses 5, so way slower then the 1 in the original 1013D code
  //15-11-2021
  //Some FLASH chips seem to have an issue with to high a speed!! Lowered it to 2 instead of 1, which does the trick
  //In the main program init this is written with 0x00001001, so clock seems to be set faster there
  //Tested this and it works so kept on that setting (zero is to fast)
  //Set SPI0 clock rate control register to AHB_CLK divided by 4 = (2 * (1 + 1))
  *SPI0_CCR = SPI_CCR_DRS_DIV_2 | SPI_CCR_CDR2(2);

  //Enable SPI0 in master mode with transmit pause enabled and do a soft reset
  *SPI0_GCR = SPI_GCR_SRST | SPI_GCR_TP_EN | SPI_GCR_MODE_MASTER | SPI_GCR_MODE_EN;

  //Wait for it to be reset  
  while(*SPI0_GCR & SPI_GCR_SRST);

  //In the main program init it is and-ed with 0xFFFFFFFC | 0x44
  //Set slave select level high, and controlled by software with signal polarity active low
  *SPI0_TCR = SPI_TCR_SS_LEVEL_HIGH | SPI_TCR_SS_OWNER_SOFT | SPI_TCR_SPOL_ACTIVE_LOW;
  
  //In the main program init it only resets the fifos. 0x80008000
  //Reset the FIFO's
  *SPI0_FCR = SPI_FCR_TX_FIFO_RST | SPI_FCR_TX_TRIG_LEV_64 | SPI_FCR_RX_FIFO_RST | SPI_FCR_RX_TRIG_LEV_1;
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_exit(void)
{
  //Disable the SPI0 controller and revert back to slave mode
  *SPI0_GCR &= ~(SPI_GCR_MODE_MASTER | SPI_GCR_MODE_EN);
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_read(int addr, unsigned char *buffer, int length)
{
  unsigned char command[4];

  //Fill in the command buffer with the read command and the address to read from
  command[0] = 0x03;
  command[1] = (unsigned char)(addr >> 16);
  command[2] = (unsigned char)(addr >> 8);
  command[3] = (unsigned char)(addr >> 0);
  
  //Assert the pre selected CS0 line
  *SPI0_TCR &= ~SPI_TCR_SS_LEVEL_HIGH;
  
  //Write the read command with the memory address to read from
  sys_spi_write(command, 4);

  //Read the data into the receive buffer
  sys_spi_read(buffer, length);
  
  //De-assert the pre selected CS0 line
  *SPI0_TCR |= SPI_TCR_SS_LEVEL_HIGH;
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_write(int addr, unsigned char *buffer, int length)
{
  int sectoraddr;
  int writesize;
  int sectorstart = addr & FLASH_SECTOR_MASK;
  int count = FLASH_SECTOR_SIZE - sectorstart;
  
  unsigned char *bptr;
  unsigned char *sptr;
  unsigned char *eptr;
  
  //Check if less then the remainder of the sector to do
  if(length < count)
  {
    //Use the input length if so
    count = length;
  }
  
  //Write to the flash until all the bytes are done
  while(length)
  {
    //Verify the flash is ready for action
    sys_spi_flash_wait_while_busy();
    
    //Set the read address to the start of the current sector
    sectoraddr = addr & FLASH_SECTOR_ADDR_MASK;
    
    //Read the data into a buffer for check on already being erased or for write back in case of new data being a partial sector
    sys_spi_flash_read(sectoraddr, sector_buffer, FLASH_SECTOR_SIZE);
    
    //Check if sector needs to be erased
    sptr = sector_buffer + sectorstart;
    eptr = sptr + count;
    
    //Keep the start point for copying the new data to
    bptr = sptr;
    
    //Go through the bytes that need to be written and check if at least one of them is not 0xFF
    while((sptr < eptr) && (*sptr == 0xFF))
    {
      sptr++;
    }
    
    //When not on the end of the data the pointers are not equal and the sector needs to be erased
    if(sptr < eptr)
    {
      //Enable the flash for erasing
      sys_spi_flash_write_enable();
      
      //Wait until writing is enabled
      sys_spi_flash_wait_while_busy();
      
      //Erase the sector
      sys_spi_flash_erase_sector(sectoraddr);

      //Copy the new data into the sector buffer
      memcpy(bptr, buffer, count);
      
      //Set the address to write to
      addr = sectoraddr;
      
      //Set the source for writing to the flash      
      bptr = sector_buffer;
      
      //Set the number of bytes to write to the flash
      writesize = FLASH_SECTOR_SIZE;
    }
    else
    {
      //Set the source for writing to the flash      
      bptr = buffer;
      
      //Set the number of bytes to write
      writesize = count;
    }
    
    //Write the data to the flash
    sys_spi_flash_program_sector(addr, bptr, writesize);
    
    //Calculate the remainder of bytes to do after this sector.
    length -= count;
    
    //Need to see if a next sector needs to be done
    if(length)
    {
      //When still data to do select the next sector
      addr = sectoraddr + FLASH_SECTOR_SIZE;
      
      //Set the sector start for the next round
      sectorstart = 0;
      
      //Point to the next data to write
      buffer += count;
      
      //See if more then a sector still to write
      if(length > FLASH_SECTOR_SIZE)
      {
        //Yes so do just a sector
        count = FLASH_SECTOR_SIZE;
      }
      else
      {
        //No do just the remainder
        count = length;
      }
    }
  }
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_program_sector(int addr, unsigned char *buffer, int length)
{
  int count = FLASH_PAGE_SIZE - (addr & FLASH_PAGE_MASK);
  
  unsigned char *sptr;
  unsigned char *eptr;
  
  //Check if less then the remainder of the page to do
  if(length < count)
  {
    //Use the input length if so
    count = length;
  }
  
  //Write to the flash until all the bytes are done
  while(length)
  {
    //Check if the block holds actual data to write. If all 0xFF it is not needed
    sptr = buffer;
    eptr = buffer + count;
    
    //Go through the bytes that need to be written and check if at least one of them is not 0xFF
    while((sptr < eptr) && (*sptr == 0xFF))
    {
      sptr++;
    }
    
    //When not on the end of the data the pointers are not equal and the page needs to be written
    if(sptr < eptr)
    {
      //Make sure the flash is ready for writing
      sys_spi_flash_wait_while_busy();

      //Enable it for writing a page
      sys_spi_flash_write_enable();

      //Wait until writing is enabled
      sys_spi_flash_wait_while_busy();

      //write the block to the flash
      sys_spi_flash_program_page(addr, buffer, count);
    }
    
    //Calculate the remainder of bytes to do after this page.
    length -= count;
    
    //Need to see if a next page needs to be done
    if(length)
    {
      //When still data to do select the next page
      addr += count;
      
      //Point to the next data to write
      buffer += count;
      
      //See if more then a page still to write
      if(length > FLASH_PAGE_SIZE)
      {
        //Yes so do just a page
        count = FLASH_PAGE_SIZE;
      }
      else
      {
        //No do just the remainder
        count = length;
      }
    }
  }
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_wait_while_busy(void)
{
  unsigned char command = 0x05;
  
  //Assert the pre selected CS0 line
  *SPI0_TCR &= ~SPI_TCR_SS_LEVEL_HIGH;
  
  //Write the read status register command to the flash
  sys_spi_write(&command, 1);
  
  //Wait for the busy flag to become 0. Makes use of the fact the busy flag bit is already set in the read status command
  while(command & 0x01)
  {
    //Read the status register from the flash. No need to resend the command. Repeated read will return the status register again.
    sys_spi_read(&command, 1);
  }
  
  //De-assert the pre selected CS0 line
  *SPI0_TCR |= SPI_TCR_SS_LEVEL_HIGH;
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_write_enable(void)
{
  unsigned char command = 0x06;
  
  //Assert the pre selected CS0 line
  *SPI0_TCR &= ~SPI_TCR_SS_LEVEL_HIGH;
  
  //Write the write enable command to the flash
  sys_spi_write(&command, 1);
  
  //De-assert the pre selected CS0 line
  *SPI0_TCR |= SPI_TCR_SS_LEVEL_HIGH;
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_erase_sector(int addr)
{
  unsigned char command[4];

  //Fill in the command buffer with the erase sector command and the address of the sector to erase
  command[0] = 0x20;
  command[1] = (unsigned char)(addr >> 16);
  command[2] = (unsigned char)(addr >> 8);
  command[3] = (unsigned char)(addr >> 0);
  
  //Assert the pre selected CS0 line
  *SPI0_TCR &= ~SPI_TCR_SS_LEVEL_HIGH;
  
  //Write the erase sector command with the memory address of the sector to erase
  sys_spi_write(command, 4);

  //De-assert the pre selected CS0 line
  *SPI0_TCR |= SPI_TCR_SS_LEVEL_HIGH;
}

//--------------------------------------------------------------------------------------

void sys_spi_flash_program_page(int addr, unsigned char *buffer, int length)
{
  unsigned char command[4];

  //Fill in the command buffer with the program page command and the address of the memory to write to
  command[0] = 0x02;
  command[1] = (unsigned char)(addr >> 16);
  command[2] = (unsigned char)(addr >> 8);
  command[3] = (unsigned char)(addr >> 0);
  
  //Assert the pre selected CS0 line
  *SPI0_TCR &= ~SPI_TCR_SS_LEVEL_HIGH;
  
  //Write the program page command and the address of the memory to write to
  sys_spi_write(command, 4);
  
  //Write the data
  sys_spi_write(buffer, length);

  //De-assert the pre selected CS0 line
  *SPI0_TCR |= SPI_TCR_SS_LEVEL_HIGH;
}

//--------------------------------------------------------------------------------------
//Send a buffer full of data to the SPI, but do it in chunks of max 64 bytes (FIFO length)
//--------------------------------------------------------------------------------------

void sys_spi_write(unsigned char *buffer, int length)
{
  int i;
  int cnt;
  
  //Send all the bytes in smaller chunks as needed
  while(length)
  {
    //Need to do it in chunks of max 64 bytes
    if(length <= 64)
      cnt = length;
    else
      cnt = 64;
    
    //Set the number of bytes to transfer in this burst
    *SPI0_MBC = cnt;

    //Set master transmit count with the number of bytes to transmit
    *SPI0_MTC = cnt;

    //Set the master single mode transmit counter to the same number of bytes to transmit
    *SPI0_BCC = cnt;

    //Load the bytes into the FIFO via the transmit byte register
    for(i=0;i<cnt;++i)
      *SPI0_TXD_BYTE = *buffer++;

    //Start the transfer
    *SPI0_TCR |= SPI_TCR_XCH_START;
    
    //Take of the chunk send
    length -= cnt;

    //Wait till SPI is done with writing
    //Is needed for the control of the CS line. Can't change it's level while the SPI is still busy
    while(*SPI0_TCR & SPI_TCR_XCH_START);

    //Clear the receive FIFO to drop what ever is in there
    //Without this it will not continue. The SPI seems to stop transmission when the receive fifo is full
    *SPI0_FCR |= SPI_FCR_RX_FIFO_RST;

    //Make sure it is cleared
    while(*SPI0_FCR & SPI_FCR_RX_FIFO_RST);
  }
}

//--------------------------------------------------------------------------------------
//Read a buffer of bytes from the SPI, but do it in chunks of max 64 bytes (FIFO length)
//--------------------------------------------------------------------------------------

void sys_spi_read(unsigned char *buffer, int length)
{
  int i;
  int cnt;

  //Clear the receive FIFO to drop what ever is left in there
  *SPI0_FCR |= SPI_FCR_RX_FIFO_RST;
  
  //Make sure it is cleared
  while(*SPI0_FCR & SPI_FCR_RX_FIFO_RST);
  
  //No bytes to transmit
  *SPI0_MTC = 0;
  *SPI0_BCC = 0;
  
  //Receive all the bytes in smaller chunks as needed  
  while(length)
  {
    //Check if more then 64 bytes (FIFO size) to read
    if(length <= 64)
      cnt = length;
    else
      cnt = 64;
    
    //Set the number of bytes to read in this burst
    *SPI0_MBC = cnt;
    
    //Start the transfer
    *SPI0_TCR |= SPI_TCR_XCH_START;
    
    //Wait until all the bytes have been received
    while((*SPI0_FSR & 0xFF) < cnt);
    
    for(i=0;i<cnt;i++)
    {
      *buffer++ = *SPI0_RXD_BYTE;
    }

    length -= cnt;
  }
}

//--------------------------------------------------------------------------------------
