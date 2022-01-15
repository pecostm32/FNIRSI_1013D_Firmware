//----------------------------------------------------------------------------------------------------------------------------------

#include "arm32.h"
#include "ccu_control.h"
#include "dram_control.h"
#include "bl_fpga_control.h"
#include "bl_sd_card_interface.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

#define PROGRAM_START_SECTOR      48

//----------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
  //Buffer for reading header from flash
  unsigned char buffer[512];
  unsigned int length;
  unsigned int blocks;
  int i,j;
  
  //Initialize the clock system
  sys_clock_init();
  
  //Initialize the internal DRAM
  sys_dram_init();

  //Instead of full memory management just the caches enabled
  arm32_icache_enable();
  arm32_dcache_enable();

  //Initialize FPGA (PORT E)
  fpga_init();
  
  //Got some time to spare
  for(j=0;j<1000;j++)
  {
    //At 600MHz CPU_CLK 1000 = ~200uS
    for(i=0;i<1000;i++);
  }
  
  //Wait and make sure FPGA is ready
  fpga_check_ready();

  //Turn of the display brightness
  fpga_set_backlight_brightness(0x0000);
  
  //Initialize the SD card
  if(sd_card_init() != SD_OK)
  {
    //On error just frees
    while(1);
  }
  
  //Load the first program sector from the SD card
  if(sd_card_read(PROGRAM_START_SECTOR, 1, buffer) != SD_OK)
  {
    //On error just frees
    while(1);
  }

  //Check if there is a brom header there
  if(memcmp(&buffer[4], "eGON.EXE", 8) != 0)
  {
    //On error just frees
    while(1);
  }
  
  //Get the length from the header
  length = ((buffer[19] << 24) | (buffer[18] << 16) | (buffer[17] << 8) | buffer[16]);
  
  //Calculate the number of sectors to read
  blocks = (length + 511) / 512;
  
  //Copy the first bytes to DRAM
  memcpy((void *)0x80000000, &buffer[32], 480);
  
  //Check if more data needs to be read
  if(blocks > 1)
  {
    //Already read the first block
    blocks--;
    
    //Load the remainder of the program from the SD card
    if(sd_card_read(PROGRAM_START_SECTOR + 1, blocks, (void *)0x800001E0) != SD_OK)
    {
      //On error just frees
      while(1);
    }
  }
 
  //Run the main program
  unsigned int address = 0x80000000;

  __asm__ __volatile__ ("mov pc, %0\n" :"=r"(address):"0"(address));
}
