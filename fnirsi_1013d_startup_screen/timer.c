//----------------------------------------------------------------------------------------------------------------------------------

#include "timer.h"
#include "interrupt.h"

//----------------------------------------------------------------------------------------------------------------------------------

volatile uint32 timer0ticks;

//----------------------------------------------------------------------------------------------------------------------------------

void timer0_setup(void)
{
  //Clear the ticks variable
  timer0ticks = 0;
  
  //Set the reload value for 1mS interrupt
  *TMR0_INTV_VALUE_REG = 24000;
  
  //Reset the counter
  *TMR0_CUR_VALUE_REG = 0;
  
  //Setup the interrupt for this timer
  setup_interrupt(TMR0_IRQ_NUM, timer0_irq_handler, 0);
  
  //Configure and enable the timer 
  *TMR0_CTRL_REG = TMR_CLK_SRC_OSC24M | TMR_ENABLE;
  
  //Enable this timers interrupt
  *TMR_IRQ_EN_REG |= 1;
}

//----------------------------------------------------------------------------------------------------------------------------------

void timer0_irq_handler(void)
{
  //Clear the interrupt
  *TMR_IRQ_STA_REG |= 1;
  
  //Add one more milli second to the ticks
  timer0ticks++;
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 timer0_get_ticks(void)
{
  return(timer0ticks);  
}

//----------------------------------------------------------------------------------------------------------------------------------

void timer0_delay(uint32 timeout)
{
  //Calculate the timeout ticks
  timeout += timer0ticks;
  
  //While technically prone to error on timer ticks overflow the scope has to be running for >49 days before it occurs
  //Wait until time has past
  while(timer0ticks < timeout);
}

//----------------------------------------------------------------------------------------------------------------------------------
