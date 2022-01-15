//--------------------------------------------------------------------------------------

#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

//--------------------------------------------------------------------------------------

#define PORTA_CFG0_REG        ((volatile unsigned int *)(0x01C20800))
#define PORTA_DATA_REG        ((volatile unsigned int *)(0x01C20810))

#define PORTB_CFG0_REG        ((volatile unsigned int *)(0x01C20824))
#define PORTB_DATA_REG        ((volatile unsigned int *)(0x01C20834))

#define PORTC_CFG0_REG        ((volatile unsigned int *)(0x01C20848))
#define PORTC_DATA_REG        ((volatile unsigned int *)(0x01C20858))

#define PORTD_CFG0_REG        ((volatile unsigned int *)(0x01C2086C))
#define PORTD_CFG1_REG        ((volatile unsigned int *)(0x01C20870))
#define PORTD_CFG2_REG        ((volatile unsigned int *)(0x01C20874))
#define PORTD_DATA_REG        ((volatile unsigned int *)(0x01C2087C))

//Allwinner F1C100s SDRAM control registers
#define SDR_PAD_DRV           ((volatile unsigned int *)(0x01C20AC0))
#define SDR_PAD_PUL           ((volatile unsigned int *)(0x01C20AC4))

//--------------------------------------------------------------------------------------
//Port C settings
#define PORTC_CFG0_PIN_0_IN            0x00000000
#define PORTC_CFG0_PIN_0_OUT           0x00000001
#define PORTC_CFG0_PIN_0_SPI0_CLK      0x00000002
#define PORTC_CFG0_PIN_0_SDC1_CLK      0x00000003
#define PORTC_CFG0_PIN_0_DISABLED      0x00000007

#define PORTC_CFG0_PIN_1_IN            0x00000000
#define PORTC_CFG0_PIN_1_OUT           0x00000010
#define PORTC_CFG0_PIN_1_SPI0_CS       0x00000020
#define PORTC_CFG0_PIN_1_SDC1_CMD      0x00000030
#define PORTC_CFG0_PIN_1_DISABLED      0x00000070

#define PORTC_CFG0_PIN_2_IN            0x00000000
#define PORTC_CFG0_PIN_2_OUT           0x00000100
#define PORTC_CFG0_PIN_2_SPI0_MISO     0x00000200
#define PORTC_CFG0_PIN_2_SDC1_D0       0x00000300
#define PORTC_CFG0_PIN_2_DISABLED      0x00000700

#define PORTC_CFG0_PIN_3_IN            0x00000000
#define PORTC_CFG0_PIN_3_OUT           0x00001000
#define PORTC_CFG0_PIN_3_SPI0_MOSI     0x00002000
#define PORTC_CFG0_PIN_3_UART0_TX      0x00003000
#define PORTC_CFG0_PIN_3_DISABLED      0x00007000

//--------------------------------------------------------------------------------------

#endif /* GPIO_CONTROL_H */

