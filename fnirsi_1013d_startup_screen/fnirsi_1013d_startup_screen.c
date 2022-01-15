//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "arm32.h"
#include "ccu_control.h"
#include "timer.h"
#include "interrupt.h"
#include "display_control.h"
#include "fss_fpga_control.h"
#include "fss_display_lib.h"
#include "fss_variables.h"
#include "sin_cos_math.h"
#include "sd_card_interface.h"
#include "fnirsi_1013d_startup_screen.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------
//
//This startup screen program is less then 24KB.
//  The main program can there for be on the SD at sector 92
//
//----------------------------------------------------------------------------------------------------------------------------------

#define PROGRAM_START_SECTOR              92

//----------------------------------------------------------------------------------------------------------------------------------

extern IRQHANDLERFUNCION interrupthandlers[];

//----------------------------------------------------------------------------------------------------------------------------------

int main(void)
{
  //Buffer for reading sector from sd card
  uint8  buffer[512];
  uint32 length;
  uint32 blocks;
  uint32 waittime;
  
  //Initialize data in BSS section
  memset(&BSS_START, 0, &BSS_END - &BSS_START);
  
  //Initialize the clock system
  sys_clock_init();
  
  //Instead of full memory management just the caches enabled
  arm32_icache_enable();
  arm32_dcache_enable();
  
  //Clear the interrupt variables
  memset(interrupthandlers, 0, 256);
  
  //Setup timer interrupt
  timer0_setup();
  
  //Enable interrupts only once. In the original code it is done on more then one location.
  arm32_interrupt_enable();
  
  //Initialize display (PORT D + DEBE)
  sys_init_display(SCREEN_WIDTH, SCREEN_HEIGHT, (uint16 *)maindisplaybuffer);
  
  //Setup the display library for the scope hardware
  setup_display_lib();

  //Draw the startup screen
  draw_startup_screen();
  
  //Initialize FPGA (PORT E)
  fpga_init();

  //Wait for the FPGA to become operational
  fpga_check_ready();
  
  //Turn on the display brightness
  fpga_set_backlight_brightness(0xEA60);
  
  //Get the timer ticks here and add 750 to is
  waittime = timer0_get_ticks() + 750;

  //Load the main program to memory
  //Initialize the SD card
  if(sd_card_init() != SD_OK)
  {
    //Show first error as red screen
    display_set_fg_color(0x00FF0000);
    display_fill_rect(0, 0, 800, 480);
    
    //On error just frees
    while(1);
  }
  
  //Load the first program sector from the SD card
  if(sd_card_read(PROGRAM_START_SECTOR, 1, buffer) != SD_OK)
  {
    //Show second error as yellow screen
    display_set_fg_color(0x00FFFF00);
    display_fill_rect(0, 0, 800, 480);
    
    //On error just frees
    while(1);
  }

  //Check if there is a correct brom header there
  if(memcmp(&buffer[4], "eGON.EXE", 8) != 0)
  {
    //Show third error as magenta screen
    display_set_fg_color(0x00FF00FF);
    display_fill_rect(0, 0, 800, 480);
    
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
      //Show fourth error as cyan screen
      display_set_fg_color(0x0000FFFF);
      display_fill_rect(0, 0, 800, 480);
    
      //On error just frees
      while(1);
    }
  }
  
  //Wait until the 750 milliseconds have passed
  while(timer0_get_ticks() < waittime);

  //Start the main program  
  unsigned int address = 0x80000000;

  __asm__ __volatile__ ("mov pc, %0\n" :"=r"(address):"0"(address));
}

//----------------------------------------------------------------------------------------------------------------------------------

void setup_display_lib(void)
{
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  
  display_set_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void draw_startup_screen(void)
{
  int i,j,p1,p2,v,x1,x2,y1,y2,yf1;
  
  //Fill the background
  display_set_fg_color(0x00307850);
  display_fill_rect(0, 0, 800, 480);

  //Gradients for the text
  display_set_fg_y_gradient(gradientbuffer, 107, 217, 0x00FF6060, 0x00CC0000);
  display_set_fg_y_gradient(gradientbuffer, 262, 372, 0x00CC0000, 0x00602020);
  
  //Draw the top line of text
  display_copy_icon_fg_color_y_gradient(letter_p_icon, 157, 107,  82, 110);
  display_copy_icon_fg_color_y_gradient(letter_e_icon, 258, 107,  81, 110);
  display_copy_icon_fg_color_y_gradient(letter_c_icon, 353, 107,  93, 110);
  display_copy_icon_fg_color_y_gradient(letter_o_icon, 461, 107, 103, 110);
  display_copy_icon_fg_color_y_gradient(letter_s_icon, 575, 135,  72, 81);

  //Draw the bottom line of text
  display_copy_icon_fg_color_y_gradient(letter_s_icon, 157, 290,  72, 81);
  display_copy_icon_fg_color_y_gradient(letter_c_icon, 237, 262,  93, 110);
  display_copy_icon_fg_color_y_gradient(letter_o_icon, 344, 262, 103, 110);
  display_copy_icon_fg_color_y_gradient(letter_p_icon, 465, 262,  82, 110);
  display_copy_icon_fg_color_y_gradient(letter_e_icon, 566, 262,  81, 110);
  
  //Draw some signals on the screen
  display_set_fg_color(0x00B4FFFE);

  //Draw a sine wave
  for(x1=0;x1<798;x1++)
  {
    x2 = x1 + 1;
    
    //Get point and skip to next
    y1 = getypos(x1 * 19, 120, 80);
    y2 = getypos(x2 * 19, 120, 80);

    display_draw_line(x1, y1, x2, y2);
    display_draw_line(x2, y1, x2+1, y2);
  }

  //Initial y position
  yf1 = 28835840;
  
  //Draw a saw tooth
  for(x1=0;x1<798;x1++)
  {
    x2 = x1 + 1;
    
    //Get point and skip to next
    if((x1 % 190) == 0)
    {
      if(x1)
      {
        display_draw_line(x1, y1, x1, 440);
        display_draw_line(x2, y1, x2, 440);
      }
      
      yf1 = 28835840;
    }
    
    y1 = yf1 >> 16;
    
    yf1 -= 55705;
   
    y2 = yf1 >> 16;

    display_draw_line(x1, y1, x2, y2);
    display_draw_line(x2, y1, x2+1, y2);
  }
  
  //Draw the raster
  display_set_fg_color(0x00000000);
  display_draw_rect(0, 0, 800, 480);
  display_draw_rect(1, 1, 798, 478);
  
  //Draw the horizontal raster lines
  v = 60;
  
  for(i=0;i<7;i++)
  {
    display_draw_horz_line(v, 0, 800);
    display_draw_horz_line(v+1, 0, 800);
    
    v+=60;
  }
  
  //Draw the vertical raster lines
  v=50;

  for(i=0;i<15;i++)
  {
    display_draw_vert_line(v, 0, 480);
    display_draw_vert_line(v+1, 0, 480);

    v+=50;
  }
  
  //Draw the horizontal ticks
  v=0;
  
  for(i=0;i<8;i++)
  {
    v+=6;

    for(j=0;j<9;j++)
    {
      if(j == 4)
      {
        p1 = 390;
        p2 = 410;
      }
      else
      {
        p1 = 394;
        p2 = 406;
      }

      display_draw_horz_line(v, p1, p2);

      v+=6;
    }
  }
  
  //Draw the vertical ticks
  v=0;
  
  for(i=0;i<16;i++)
  {
    v+=5;

    for(j=0;j<9;j++)
    {
      if(j == 4)
      {
        p1 = 230;
        p2 = 250;
      }
      else
      {
        p1 = 234;
        p2 = 246;
      }

      display_draw_vert_line(v, p1, p2);

      v+=5;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
