//----------------------------------------------------------------------------------------------------------------------------------

#include "interrupt.h"

//----------------------------------------------------------------------------------------------------------------------------------

//Interrupt handler function pointer table 
IRQHANDLERFUNCION interrupthandlers[64];

//----------------------------------------------------------------------------------------------------------------------------------

void setup_interrupt(uint32 irq, IRQHANDLERFUNCION function, uint32 priority)
{
  uint32 *ptr;
  
  //Set the handler for this interrupt in the table
  interrupthandlers[irq] = function;
  
  //Set the priority for this interrupt (Compiler does size based conversion on pointers) 
   ptr  = (uint32 *)(INTC_PRIO_REG0 + ((irq & 0x30) >> 4));
  *ptr |= (priority & 0x03) << ((irq & 0x0F) << 1);
  
  //Enable the actual interrupt (Compiler does size based conversion on pointers)
   ptr  = (uint32 *)(INTC_EN_REG0 + (irq >> 5));
  *ptr |= 1 << (irq & 0x1F);  
}

//----------------------------------------------------------------------------------------------------------------------------------
//The actual interrupt handler that takes care of the stacks and registers is in start.s

void irq_handler(void) 
{
  uint32 pending;
  uint32 irq = 0;
  uint32 i;
  
  //Handle the interrupt pending registers
  for(i=0;i<2;i++)
  {
    //Get the interrupts of the first pending register
    pending = INTC_PEND_REG0[i];
  
    //Clear the pending interrupts before servicing
    INTC_PEND_REG0[i] = 0;
  
    //Check if active interrupts
    while(pending)
    {
      //Check if current one is active
      if(pending & 1)
      {
        //Call the handler for this interrupt if one is set
        if(interrupthandlers[irq])
        {
          interrupthandlers[irq]();
        }
      }

      //Select the next interrupt
      pending >>= 1;
      irq++;
    }
    
    //Start on first irq of second range
    irq = 32;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
