//----------------------------------------------------------------------------------------------------------------------------------

#ifndef INTERRUPT_H
#define INTERRUPT_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

//Interrupt controller registers
#define INTC_VECTOR_REG         ((volatile uint32 *)(0x01C20400))
#define INTC_BASE_ADDR_REG      ((volatile uint32 *)(0x01C20404))

#define NMI_INT_CTRL_REG        ((volatile uint32 *)(0x01C2040C))

#define INTC_PEND_REG0          ((volatile uint32 *)(0x01C20410))
#define INTC_PEND_REG1          ((volatile uint32 *)(0x01C20414))

#define INTC_EN_REG0            ((volatile uint32 *)(0x01C20420))
#define INTC_EN_REG1            ((volatile uint32 *)(0x01C20424))

#define INTC_MASK_REG0          ((volatile uint32 *)(0x01C20430))
#define INTC_MASK_REG1          ((volatile uint32 *)(0x01C20434))

#define INTC_RESP_REG0          ((volatile uint32 *)(0x01C20440))
#define INTC_RESP_REG1          ((volatile uint32 *)(0x01C20444))

#define INTC_FF_REG0            ((volatile uint32 *)(0x01C20450))
#define INTC_FF_REG1            ((volatile uint32 *)(0x01C20454))

#define INTC_PRIO_REG0          ((volatile uint32 *)(0x01C20460))
#define INTC_PRIO_REG1          ((volatile uint32 *)(0x01C20464))
#define INTC_PRIO_REG2          ((volatile uint32 *)(0x01C20468))
#define INTC_PRIO_REG3          ((volatile uint32 *)(0x01C2046C))


//----------------------------------------------------------------------------------------------------------------------------------

#define TMR0_IRQ_NUM           13
#define TMR1_IRQ_NUM           14
#define TMR2_IRQ_NUM           15

#define USB_IRQ_NUM            26

#define PORTE_EINT_IRQ        0x27

//----------------------------------------------------------------------------------------------------------------------------------

//Function pointer for irq handlers
typedef void (*IRQHANDLERFUNCION)(void);

//----------------------------------------------------------------------------------------------------------------------------------

void setup_interrupt(uint32 irq, IRQHANDLERFUNCION function, uint32 priority);

//----------------------------------------------------------------------------------------------------------------------------------

void irq_handler(void);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* INTERRUPT_H */

