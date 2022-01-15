#include "ccu_control.h"

void sys_clock_init(void)
{
  int time;
  
  //Set PLL stable time registers
  *CCU_PLL_STABLE_TIME0 = 0x01FF;     //Number of micro second for the PLL to become stable
  *CCU_PLL_STABLE_TIME1 = 0x01FF;

  //Switch to CPU PLL clock
  *CCU_CPU_CLK_SRC = CPU_CLK_SRC_PLL;
    
  //Set HCLK = CPUCLK / 2. AHB_CLK = CPUCLK / 2. ABP_CLK = AHB_CLK / 2. (HCLK = 300MHz, AHB_CLK = 300MHz, ABP_CLK = 150MHz)
  *CCU_AHB_APB_CFG = CCU_HCLKC_DIV_2 | CCU_AHB_CLK_SRC_CPUCLK | CCU_APB_CLK_RATIO_2 | CCU_AHB_CLK_DIV_RATIO_2;
  
  //Set the CPU PLL to 600MHz (24MHz * 25 * 1) / (1 * 1)
  *CCU_PLL_CPU_CTRL = CCU_PLL_ENABLE | CCU_PLL_CPU_FACTOR_N(25);
  
  //Max checks on PLL becoming stable
  time = 4000;
  
  //Wait for the PLL to become stable, but not endlessly
  while(time && ((*CCU_PLL_CPU_CTRL & CCU_PLL_LOCKED) == 0))
    time--;
  
  //Disable the peripheral PLL to be able to change it
  *CCU_PLL_PERIPH_CTRL = 0;
  
  //Set the peripheral PLL to 576MHz to allow the SD card clock to become 48MHz
  *CCU_PLL_PERIPH_CTRL = CCU_PLL_ENABLE | CCU_PLL_PERIPH_24M_OUT_EN | CCU_PLL_PERIPH_FACTOR_N(24);
  
  //Max checks on PLL becoming stable
  time = 4000;
  
  //Wait for the PLL to become stable, but not endlessly
  while(time && ((*CCU_PLL_PERIPH_CTRL & CCU_PLL_LOCKED) == 0))
    time--;
}
