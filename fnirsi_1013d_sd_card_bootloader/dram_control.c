#include "dram_control.h"
#include "ccu_control.h"
#include "gpio_control.h"

void sys_dram_init(void)
{
  dram_parameters params;
  
  params.base        = 0x80000000;
  params.size        = 32;
  params.clk         = PLL_DDR_CLK / 1000000;
  params.access_mode = 1;
  params.cs_num      = 1;
  params.ddr8_remap  = 0;
  params.sdr_ddr     = DRAM_TYPE_DDR;
  params.bwidth      = 16;
  params.col_width   = 10;
  params.row_width   = 13;
  params.bank_size   = 4;
  params.cas         = 0x03;
  
  dram_init(&params);
}

int dram_init(dram_parameters *params)
{
  unsigned int val = 0;
  unsigned int i;

  //Disable PB3
  *PORTB_CFG0_REG |= 0x00007000;
  
  //Wait for 5ms  
  dram_delay(5);
  
  if(((params->cas) >> 3) & 0x1)
  {
    //Set DRAM reference configuration
    *SDR_PAD_PUL |=  ENABLE_INTERNAL_REF | REF_CONF_FACTOR_32;
  }
  
  if((params->clk >= 144) && (params->clk <= 180))
  {
    *SDR_PAD_DRV = 0x0AAA;
  }
  else if(params->clk >= 180)
  {
    *SDR_PAD_DRV = 0x0FFF;
  }
  
  
  if((params->clk) <= 96)
  {
    val = (0x1 << 0) | (0x0 << 4) | (((params->clk * 2) / 12 - 1) << 8) | (0x1u << 31);
  }
  else
  {
    val = (0x0 << 0) | (0x0 << 4) | (((params->clk * 2) / 24 - 1) << 8) | (0x1u << 31);
  }
  
  if(params->cas & (0x1 << 4))
  {
    *CCU_PLL_DDR0_PAT = 0xD1303333;
  }
  else if(params->cas & (0x1 << 5))
  {
    *CCU_PLL_DDR0_PAT = 0xCCE06666;
  }
  else if(params->cas & (0x1 << 6))
  {
    *CCU_PLL_DDR0_PAT = 0xC8909999;
  }
  else if(params->cas & (0x1 << 7))
  {
    *CCU_PLL_DDR0_PAT = 0xc440cccc;
  }
  
  if(params->cas & (0xf << 4))
  {
    val |= 0x1 << 24;
  }
  
  *CCU_PLL_DDR_CTRL = val;
  
  *CCU_PLL_DDR_CTRL |= (0x1 << 20);
  
  //Wait till PLL locked
  while((*CCU_PLL_DDR_CTRL & (1 << 28)) == 0);
  
  //Wait 5ms
  dram_delay(5);
  
  *CCU_BUS_CLK_GATE0 |= (0x1 << 14);
  
  *CCU_BUS_SOFT_RST0 &= ~(0x1 << 14);

  
  for(i = 0; i < 10; i++)
    continue;
  
  *CCU_BUS_SOFT_RST0 |= (0x1 << 14);

  val = *SDR_PAD_PUL;
  
  (params->sdr_ddr == DRAM_TYPE_DDR) ? (val |= (0x1 << 16)) : (val &= ~(0x1 << 16));
  
  *SDR_PAD_PUL = val;

  val = (SDR_T_CAS << 0) | (SDR_T_RAS << 3) | (SDR_T_RCD << 7) | (SDR_T_RP << 10) | (SDR_T_WR << 13) | (SDR_T_RFC << 15) | (SDR_T_XSR << 19) | (SDR_T_RC << 28);
  
  *DRAM_STMG0R = val;
  
  val = (SDR_T_INIT << 0) | (SDR_T_INIT_REF << 16) | (SDR_T_WTR << 20) | (SDR_T_RRD << 22) | (SDR_T_XP << 25);
  
  *DRAM_STMG1R = val;
  
  dram_params_setup(params);
  dram_check_type(params);

  val = *SDR_PAD_PUL;
  
  (params->sdr_ddr == DRAM_TYPE_DDR) ? (val |= (0x1 << 16)) : (val &= ~(0x1 << 16));
  
  *SDR_PAD_PUL = val;

  dram_set_autofresh_cycle(params->clk);
  dram_scan_readpipe(params);
  dram_get_dram_size(params);

  for(i = 0; i < 128; i++)
  {
    *((volatile unsigned int *)(params->base + 4 * i)) = params->base + 4 * i;
  }

  for(i = 0; i < 128; i++)
  {
    if(*((volatile unsigned int *)(params->base + 4 * i)) != (params->base + 4 * i))
      return 0;
  }
  
  return 1;
}

int dram_params_setup(dram_parameters *params)
{
  unsigned int val = 0;

  val = (params->ddr8_remap) | (0x1 << 1) | ((params->bank_size >> 2) << 3) | ((params->cs_num >> 1) << 4) | ((params->row_width - 1) << 5) | ((params->col_width - 1) << 9) | ((params->sdr_ddr ? (params->bwidth >> 4) : (params->bwidth >> 5)) << 13) | (params->access_mode << 15) | (params->sdr_ddr << 16);

  *DRAM_SCONR = val;
  
  *DRAM_SCTLR |= (0x1 << 19);
  
  return dram_initial();
}


int dram_initial(void)
{
  unsigned int time = 0xffffff;

  *DRAM_SCTLR |= 0x1;
  
  while((*DRAM_SCTLR & 0x1) && time--)
  {
    if(time == 0)
      return 0;
  }
  return 1;
}

unsigned int dram_check_type(dram_parameters *params)
{
  unsigned int times = 0;
  unsigned int i;

  for(i = 0; i < 8; i++)
  {
    *DRAM_SCTLR = (*DRAM_SCTLR & (~(0x7 << 6))) | (i << 6);

    dram_delay_scan();
    
    if(*DRAM_DDLYR & 0x30)
      times++;
  }

  if(times == 8)
  {
    params->sdr_ddr = DRAM_TYPE_SDR;
    return 0;
  }
  else
  {
    params->sdr_ddr = DRAM_TYPE_DDR;
    return 1;
  }
}

int dram_delay_scan(void)
{
  unsigned int time = 0xffffff;

  *DRAM_DDLYR |= 0x1;
  
  while((*DRAM_DDLYR & 0x1) && time--)
  {
    if(time == 0)
      return 0;
  }
  
  return 1;
}

void dram_set_autofresh_cycle(unsigned int clk)
{
  unsigned int val = 0;
  unsigned int row = 0;
  unsigned int temp = 0;

  row = *DRAM_SCONR;
  row &= 0x1e0;
  row >>= 0x5;

  if(row == 0xc)
  {
    if(clk >= 1000000)
    {
      temp = clk + (clk >> 3) + (clk >> 4) + (clk >> 5);
      while(temp >= (10000000 >> 6))
      {
        temp -= (10000000 >> 6);
        val++;
      }
    }
    else
    {
      val = (clk * 499) >> 6;
    }
  }
  else if(row == 0xb)
  {
    if(clk >= 1000000)
    {
      temp = clk + (clk >> 3) + (clk >> 4) + (clk >> 5);
      while(temp >= (10000000 >> 7))
      {
        temp -= (10000000 >> 7);
        val++;
      }
    }
    else
    {
      val = (clk * 499) >> 5;
    }
  }
  
 *DRAM_SREFR = val;
}

unsigned int dram_scan_readpipe(dram_parameters *params)
{
  unsigned int i, rp_best = 0, rp_val = 0;
  unsigned int val = 0;
  unsigned int readpipe[8];

  if(params->sdr_ddr == DRAM_TYPE_DDR)
  {
    for(i = 0; i < 8; i++)
    {
      val = *DRAM_SCTLR;
      val &= ~(0x7 << 6);
      val |= (i << 6);
      *DRAM_SCTLR = val;
      
      dram_delay_scan();
      
      readpipe[i] = 0;
      
      if((((*DRAM_DDLYR >> 4) & 0x3) == 0x0) && (((*DRAM_DDLYR >> 4) & 0x1) == 0x0))
      {
        readpipe[i] = dram_check_delay(params->bwidth);
      }
      
      if(rp_val < readpipe[i])
      {
        rp_val = readpipe[i];
        rp_best = i;
      }
    }
    
    *DRAM_SCTLR = (*DRAM_SCTLR & (~(0x7 << 6))) | (rp_best << 6);
    
    dram_delay_scan();
  }
  else
  {
    *DRAM_SCONR &= ~0x00016000;  //(~(0x1 << 16)) and (~(0x3 << 13)) combined
    
    rp_best = sdr_readpipe_select();

    *DRAM_SCTLR = (*DRAM_SCTLR & (~(0x7 << 6))) | (rp_best << 6);
  }
  
  
  return 0;
}

unsigned int dram_get_dram_size(dram_parameters *params)
{
  unsigned int colflag = 10, rowflag = 13;
  unsigned int i = 0;
  unsigned int count = 0;
  unsigned int *ptr1, *ptr2;

  params->col_width = colflag;
  params->row_width = rowflag;
  dram_params_setup(params);
  dram_scan_readpipe(params);
  
  ptr1 = (unsigned int *)0x80000200;
  ptr2 = (unsigned int *)0x80000600;
  
  for(i=0;i<32;i++)
  {
    ptr1[i] = 0x11111111;
    ptr2[i] = 0x22222222;
  }
  
  for(i=0;i<32;i++)
  {
    if(ptr1[i] == 0x22222222)
      count++;
  }
  
  if(count == 32)
  {
    colflag = 9;
  }
  else
  {
    colflag = 10;
  }
  
  count = 0;
  params->col_width = colflag;
  params->row_width = rowflag;
  
  dram_params_setup(params);
  
  if(colflag == 10)
  {
    ptr1 = (unsigned int *)0x80400000;
    ptr2 = (unsigned int *)0x80c00000;
  }
  else
  {
    ptr1 = (unsigned int *)0x80200000;
    ptr2 = (unsigned int *)0x80600000;
  }
  
  for(i=0;i<32;i++)
  {
    ptr1[i] = 0x33333333;
    ptr2[i] = 0x44444444;
  }
  
  for(i=0;i<32;i++)
  {
    if(ptr1[i] == 0x44444444)
      count++;
  }
  
  if(count == 32)
  {
    rowflag = 12;
  }
  else
  {
    rowflag = 13;
  }
  
  params->col_width = colflag;
  params->row_width = rowflag;
  
  if(params->row_width != 13)
  {
    params->size = 16;
  }
  else if(params->col_width == 10)
  {
    params->size = 64;
  }
  else
  {
    params->size = 32;
  }
  
  dram_set_autofresh_cycle(params->clk);
  
  params->access_mode = 0;
  
  dram_params_setup(params);

  return 0;
}

unsigned int dram_check_delay(unsigned int bwidth)
{
  unsigned int dsize;
  unsigned int i,j;
  unsigned int num = 0;
  unsigned int dflag = 0;

  dsize = ((bwidth == 16) ? 4 : 2);
  
  for(i=0;i<dsize;i++)
  {
    if(i == 0)
      dflag = *DRAM_DRPTR0;
    else if(i == 1)
      dflag = *DRAM_DRPTR1;
    else if(i == 2)
      dflag = *DRAM_DRPTR2;
    else if(i == 3)
      dflag = *DRAM_DRPTR3;

    for(j=0;j<32;j++)
    {
      if(dflag & 0x1)
        num++;
      
      dflag >>= 1;
    }
  }
  
  return num;
}

int sdr_readpipe_scan(void)
{
  unsigned int k = 0;
  unsigned int *ptr = (unsigned int *)0x80000000;

  for(k=0;k<32;k++)
  {
    ptr[k] = k;
  }
  
  for(k=0;k<32;k++)
  {
    if(ptr[k] != k)
      return(0);
  }
  
  return(1);
}

unsigned int sdr_readpipe_select(void)
{
  unsigned int i;
  
  for(i=0;i<8;i++)
  {
    *DRAM_SCTLR = (*DRAM_SCTLR & (~(0x7 << 6))) | (i << 6);
    
    if(sdr_readpipe_scan())
    {
      return(i);
    }
  }
  
  return(0);
}

void dram_delay(unsigned int ms)
{
  unsigned int loops = ms * 2000;

  __asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"  "bne 1b":"=r"(loops):"0"(loops));
}

