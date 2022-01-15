#include "ccu_control.h"

void sys_clock_init(void)
{
  int time;
  
  //Set PLL stable time registers
  *CCU_PLL_STABLE_TIME0 = 0x01FF;     //Number of micro second for the PLL to become stable
  *CCU_PLL_STABLE_TIME1 = 0x01FF;

  //Switch to slow 24MHz clock
  *CCU_CPU_CLK_SRC = CPU_CLK_SRC_OSC24M;

  //Wait for the clock switch to be done
  ccu_delay(20);

  //Set the peripheral PLL to 576MHz (For SD card 48MHz clock)
  *CCU_PLL_PERIPH_CTRL = CCU_PLL_ENABLE | CCU_PLL_PERIPH_24M_OUT_EN | CCU_PLL_PERIPH_FACTOR_N(24);

  //Max checks on PLL becoming stable
  time = 4000;
  
  //Wait for the PLL to become stable, but not endlessly
  while(time && ((*CCU_PLL_PERIPH_CTRL & CCU_PLL_LOCKED) == 0))
    time--;
  
  //Set AHB clock source to PLL_PERIPH / 3. ABP CLK = AHB CLK / 2.
//  *CCU_AHB_APB_CFG = CCU_AHB_CLK_SRC_PLL_PERIPH | CCU_APB_CLK_RATIO_2 | CCU_AHB_PRE_DIV_3;
  
  //Set HCLK = CPUCLK / 2. AHB_CLK = CPUCLK / 2. ABP_CLK = AHB_CLK / 2. (HCLK = 300MHz, AHB_CLK = 300MHz, ABP_CLK = 150MHz)
  *CCU_AHB_APB_CFG = CCU_HCLKC_DIV_2 | CCU_AHB_CLK_SRC_CPUCLK | CCU_APB_CLK_RATIO_2 | CCU_AHB_CLK_DIV_RATIO_2;
  
  
  //Give the clock system some time
  ccu_delay(20);

  //Enable BE and FE clock
  *CCU_DRAM_CLK_GATE = CCU_DRAM_BE_DCLK_GATING_PASS | CCU_DRAM_FE_DCLK_GATING_PASS;
  
  ccu_delay(20);

  //Set the CPU PLL to 600MHz (24MHz * 25 * 1) / (1 * 1)
  *CCU_PLL_CPU_CTRL = CCU_PLL_ENABLE | CCU_PLL_CPU_FACTOR_N(25);
  
  //Max checks on PLL becoming stable
  time = 4000;
  
  //Wait for the PLL to become stable, but not endlessly
  while(time && ((*CCU_PLL_CPU_CTRL & CCU_PLL_LOCKED) == 0))
    time--;
  
  //Switch to CPU PLL clock
  *CCU_CPU_CLK_SRC = CPU_CLK_SRC_PLL;
  
  //Clock source switch needs some time
  ccu_delay(20);
}

void ccu_delay(int loops)
{
  while(loops--);
}


