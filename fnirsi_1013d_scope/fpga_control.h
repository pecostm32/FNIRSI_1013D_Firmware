//----------------------------------------------------------------------------------------------------------------------------------

#ifndef FPGA_CONTROL_H
#define FPGA_CONTROL_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------

//FPGA port registers (Port E on the F1C100s)
#define FPGA_BUS_CFG_REG        ((volatile uint32 *)(0x01C20890))
#define FPGA_CTRL_CFG_REG       ((volatile uint32 *)(0x01C20894))
#define FPGA_DATA_REG           ((volatile uint32 *)(0x01C208A0))

//Initialize the control lines for communication with the FPGA (PE8:10 output)
#define FPGA_CTRL_INIT()        *FPGA_CTRL_CFG_REG = (*FPGA_CTRL_CFG_REG & 0xFFFFF000) | 0x00000111

//Initialize the clock line to high
#define FPGA_CLK_INIT()         (*FPGA_DATA_REG |= 0x00000100)

//Set the different target options for communication with the FPGA
#define FPGA_CMD_WRITE()        (*FPGA_DATA_REG = (*FPGA_DATA_REG & 0xFFFFF9FF) | 0x00000600)
#define FPGA_CMD_READ()         (*FPGA_DATA_REG = (*FPGA_DATA_REG & 0xFFFFF9FF) | 0x00000400)
#define FPGA_DATA_WRITE()       (*FPGA_DATA_REG = (*FPGA_DATA_REG & 0xFFFFF9FF) | 0x00000200)
#define FPGA_DATA_READ()        (*FPGA_DATA_REG = (*FPGA_DATA_REG & 0xFFFFF9FF) | 0x00000000)

//Clock control
#define FPGA_PULSE_CLK()        (*FPGA_DATA_REG &= 0xFFFFFEFF);(*FPGA_DATA_REG |= 0x00000100)

//Control the direction of the FPGA databus
#define FPGA_BUS_DIR_IN()       *FPGA_BUS_CFG_REG  = 0x00000000
#define FPGA_BUS_DIR_OUT()      *FPGA_BUS_CFG_REG  = 0x11111111

//Put data on or get data from the FPGA databus
#define FPGA_SET_DATA(x)        (*FPGA_DATA_REG = (*FPGA_DATA_REG & 0xFFFFFF00) | (x & 0x000000FF))
#define FPGA_GET_DATA()         (*FPGA_DATA_REG & 0x000000FF)

//----------------------------------------------------------------------------------------------------------------------------------

void   fpga_init(void);

void   fpga_write_cmd(uint8 command);
uint8  fpga_read_cmd(void);

void   fpga_write_byte(uint8 data);
uint8  fpga_read_byte(void);

void   fpga_write_short(uint16 data);
uint16 fpga_read_short(void);

void   fpga_write_int(uint32 data);

void   fpga_set_backlight_brightness(uint16 brightness);
void   fpga_set_translated_brightness(void);

uint16 fpga_get_version(void);

void   fpga_check_ready(void);

void   fpga_enable_system(void);


void   fpga_set_channel_enable(PCHANNELSETTINGS settings);
void   fpga_set_channel_coupling(PCHANNELSETTINGS settings);
void   fpga_set_channel_voltperdiv(PCHANNELSETTINGS settings);
void   fpga_set_channel_offset(PCHANNELSETTINGS settings);

void   fpga_set_sample_rate(uint32 samplerate);
void   fpga_set_time_base(uint32 timebase);

void   fpga_set_trigger_channel(void);
void   fpga_set_trigger_edge(void);
void   fpga_swap_trigger_channel(void);
void   fpga_set_trigger_level(void);
void   fpga_set_trigger_mode(void);

void   fpga_do_conversion(void);

uint16 fpga_prepare_for_transfer(void);

void   fpga_read_sample_data(PCHANNELSETTINGS settings, uint32 triggerpoint);
void   fpga_read_adc_data(PCHANNELSETTINGS settings);



void   fpga_set_battery_level(void);



void   fpga_delay(uint32 usec);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* FPGA_CONTROL_H */

