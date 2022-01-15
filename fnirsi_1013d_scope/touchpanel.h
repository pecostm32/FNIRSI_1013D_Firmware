//----------------------------------------------------------------------------------------------------------------------------------

#ifndef TOUCHPANEL_H
#define TOUCHPANEL_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

//Port A registers
#define PORT_A_CFG_REG          ((volatile uint32 *)(0x01C20800))
#define PORT_A_DATA_REG         ((volatile uint32 *)(0x01C20810))

//----------------------------------------------------------------------------------------------------------------------------------

#define TP_CMD_REG              0x00008040
#define TP_CFG_VERSION_REG      0x00008047
#define TP_STATUS_REG           0x0000814E
#define TP_COORD1_REG           0x00008150

#define TP_XMAX_LOW_REG         0x00008048

//----------------------------------------------------------------------------------------------------------------------------------

#define TP_DEVICE_ADDR_WRITE    0x28
#define TP_DEVICE_ADDR_READ     0x29

//----------------------------------------------------------------------------------------------------------------------------------

#define TP_CLOCK_DELAY          4
#define TP_DATA_DELAY           2

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_setup(void);

void tp_i2c_read_status(void);

void tp_i2c_wait_for_touch_release(void);

void tp_i2c_send_data(uint16 reg_addr, uint8 *buffer, uint32 size);
void tp_i2c_read_data(uint16 reg_addr, uint8 *buffer, uint32 size);

void tp_i2c_send_start(void);
void tp_i2c_send_stop(void);

void tp_i2c_clock_ack(void);
void tp_i2c_clock_nack(void);

void tp_i2c_send_byte(uint8 data);
uint8 tp_i2c_read_byte(void);

void tp_delay(uint32 usec);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* TOUCHPANEL_H */

