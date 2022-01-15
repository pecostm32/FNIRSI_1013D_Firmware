//----------------------------------------------------------------------------------------------------------------------------------

#ifndef TIMER_H
#define TIMER_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

//Timer registers
#define TMR_IRQ_EN_REG         ((volatile uint32 *)(0x01C20C00))
#define TMR_IRQ_STA_REG        ((volatile uint32 *)(0x01C20C04))

#define TMR0_CTRL_REG          ((volatile uint32 *)(0x01C20C10))
#define TMR0_INTV_VALUE_REG    ((volatile uint32 *)(0x01C20C14))
#define TMR0_CUR_VALUE_REG     ((volatile uint32 *)(0x01C20C18))

#define TMR1_CTRL_REG          ((volatile uint32 *)(0x01C20C20))
#define TMR1_INTV_VALUE_REG    ((volatile uint32 *)(0x01C20C24))
#define TMR1_CUR_VALUE_REG     ((volatile uint32 *)(0x01C20C28))

#define TMR2_CTRL_REG          ((volatile uint32 *)(0x01C20C30))
#define TMR2_INTV_VALUE_REG    ((volatile uint32 *)(0x01C20C34))
#define TMR2_CUR_VALUE_REG     ((volatile uint32 *)(0x01C20C38))

#define AVS_CNT_CTL_REG        ((volatile uint32 *)(0x01C20C80))
#define AVS_CNT0_REG           ((volatile uint32 *)(0x01C20C84))
#define AVS_CNT1_REG           ((volatile uint32 *)(0x01C20C88))
#define AVS_CNT_DIV_REG        ((volatile uint32 *)(0x01C20C8C))

#define WDOG_IRQ_EN_REG        ((volatile uint32 *)(0x01C20CA0))
#define WDOG_IRQ_STA_REG       ((volatile uint32 *)(0x01C20CA4))

#define WDOG_CTRL_REG          ((volatile uint32 *)(0x01C20CB0))
#define WDOG_CFG_REG           ((volatile uint32 *)(0x01C20CB4))
#define WDOG_MODE_REG          ((volatile uint32 *)(0x01C20CB8))

//----------------------------------------------------------------------------------------------------------------------------------

#define TMR_CLK_SRC_OSC24M     0x00000004
#define TMR_RELOAD             0x00000002
#define TMR_ENABLE             0x00000001

//----------------------------------------------------------------------------------------------------------------------------------


void timer0_setup(void);

void timer0_irq_handler(void);

uint32 timer0_get_ticks(void);

void timer0_delay(uint32 timeout);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* TIMER_H */

