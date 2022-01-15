//----------------------------------------------------------------------------------------------------------------------------------
//The touch panel on the scope is not connected to a true I2C interface, so bit banging is used
//
//The connections are:
//  PA0:  RESET
//  PA1:  INT
//  PA2:  SDA
//  PA3:  SCL
//
//The touch panel uses a GT911 controller and has a special startup sequence to set the I2C device address
//
//----------------------------------------------------------------------------------------------------------------------------------

#include "variables.h"
#include "touchpanel.h"

//----------------------------------------------------------------------------------------------------------------------------------
//Touch panel configuration for the GT9157 set to 800x480 resolution

#ifdef USE_TP_CONFIG
#define USE_LR_CONFIG

#ifdef USE_LR_CONFIG
uint8 tp_config_data[] =
{
  0xFF, 0x20, 0x03, 0xE0, 0x01, 0x0A, 0xFD, 0x00, 0x01, 0x08, 0x28, 0x08, 0x5A, 0x3C, 0x03, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1A, 0x1E, 0x14, 0x87, 0x29, 0x0A, 0x75, 0x77,
  0xB2, 0x04, 0x00, 0x00, 0x00, 0x9A, 0x01, 0x11, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x50, 0xA0, 0x94, 0xD5, 0x02, 0x08, 0x00, 0x00, 0x04, 0xA1, 0x55, 0x00, 0x8F,
  0x62, 0x00, 0x7F, 0x71, 0x00, 0x73, 0x82, 0x00, 0x69, 0x95, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x04, 0x06, 0x08, 0x0A, 0x0C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x26, 0x28, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01
};
#else
uint8 tp_config_data[] =
{
  0xFF, 0x20, 0x03, 0xE0, 0x01, 0x0A, 0xFD, 0x00, 0x01, 0x08, 0x28, 0x08, 0x5A, 0x3C, 0x03, 0x05,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1A, 0x1E, 0x14, 0x8B, 0x2A, 0x0C, 0x75, 0x77,
  0xB2, 0x04, 0x00, 0x00, 0x00, 0x9A, 0x01, 0x11, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x50, 0xA0, 0x94, 0xD5, 0x02, 0x08, 0x00, 0x00, 0x04, 0xA1, 0x55, 0x00, 0x8F,
  0x62, 0x00, 0x7F, 0x71, 0x00, 0x73, 0x82, 0x00, 0x69, 0x95, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x16, 0x14, 0x12, 0x10, 0x0E, 0x0C, 0x0A, 0x08, 0x06, 0x04, 0x02, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x22,
  0x21, 0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x18, 0x16, 0x13, 0x12, 0x10, 0x0F, 0x0C, 0x0A, 0x08, 0x06,
  0x04, 0x02, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 
};
#endif
#endif

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_setup(void)
{
  uint8 command;

#ifdef USE_TP_CONFIG
  uint8 checksum;
  int   i;
#else  
  uint32 xmax;
  uint32 ymax;
#endif
  
  //Make sure all SCL and SDA are at a high level and RESET and INT  are at a low level before enabling as output 
  *PORT_A_DATA_REG = 0x0000000C;
  
  //Setup all port A pins for output
  *PORT_A_CFG_REG = 0xFFFF1111;
  
  //Wait for 100uS
  tp_delay(200);

  //Set INT high for device address 0x14 select
  *PORT_A_DATA_REG = 0x0000000E;
  
  //Wait for 20mS
  tp_delay(20000);
  
  //Set RESET high for device activation
  *PORT_A_DATA_REG = 0x0000000F;

  //Wait for ~5mS
  tp_delay(10000);
  
  //Set INT as input and keep the rest as output
  *PORT_A_CFG_REG = 0xFFFF1101;

  //Wait for ~50mS
  tp_delay(100000);

  //Start communication by sending 2 to the command register
  command = 2;
  tp_i2c_send_data(TP_CMD_REG, &command, 1);

#ifdef USE_TP_CONFIG
  //Clear the checksum before calculating it
  checksum = 0;  
  
  //Calculate checksum over the configuration data (first 184 bytes)
  for(i=0;i<184;i++)
  {
    checksum += tp_config_data[i];
  }
  
  //Do the last action to make it the correct checksum
  checksum = ~checksum + 1;
  
  //Put the checksum in the buffer
  tp_config_data[184] = checksum;
  
  //Send the configuration data
  tp_i2c_send_data(TP_CFG_VERSION_REG, tp_config_data, sizeof(tp_config_data));
#else
  //Read the touch panel configuration to calculate resolution scalers
  tp_i2c_read_data(TP_CFG_VERSION_REG, tp_config_data, sizeof(tp_config_data));
  
  //Get maximum x and y value from configuration
  xmax = tp_config_data[1] | (tp_config_data[2] << 8);
  ymax = tp_config_data[3] | (tp_config_data[4] << 8);
  
  //Calculate scalers based on the set x and y max output values
  xscaler = (800 << 20) / xmax;
  yscaler = (480 << 20) / ymax;
#endif
  
  //Wait for ~100mS
  tp_delay(200000);

  //Start scanning by sending 0 to the command register
  command = 0;
  tp_i2c_send_data(TP_CMD_REG, &command, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_wait_for_touch_release(void)
{
  //Wait until touch is released
  while(havetouch)
  {
    //Read the touch panel status
    tp_i2c_read_status();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_read_status(void)
{
  uint8  status;
  uint8  data[4];

  //Read the status of the touch panel
  tp_i2c_read_data(TP_STATUS_REG, &status, 1);
  
  //Check if there is touch
  if(status & 0x80)
  {
    //Clear the status
    data[0] = 0;
    tp_i2c_send_data(TP_STATUS_REG, data, 1);
    
    status &= 0x0F;
    
    //Check if single touch point to handle
    if(status == 1)
    {
      //Get the touch point data
      tp_i2c_read_data(TP_COORD1_REG, data, 4);

      //Store the result in the global coordinate variables
      xtouch = data[0] | (data[1] << 8);
      ytouch = data[2] | (data[3] << 8);
      
#ifndef USE_TP_CONFIG
      //Scale the coordinates based on the x and y max values
      xtouch = (xscaler * xtouch) >> 20;
      ytouch = (yscaler * ytouch) >> 20;
#endif
      
      //Signal touch active
      havetouch = 1;
    }
    else
    {
      //Original code checks on a second touch point active, but not sure if it does anything with it.
      //No or to many points so set out of range touch
      xtouch = 800;
      ytouch = 480;
      
      //Signal no touch active
      havetouch = 0;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_send_data(uint16 reg_addr, uint8 *buffer, uint32 size)
{
  //Start a communication sequence
  tp_i2c_send_start();
  
  //Send the device address for writing
  tp_i2c_send_byte(TP_DEVICE_ADDR_WRITE);

  //Send the register address high byte first
  tp_i2c_send_byte(reg_addr >> 8);

  //Send the low byte second
  tp_i2c_send_byte(reg_addr);
  
  //Send the data bytes
  while(size)
  {
    //Send a byte
    tp_i2c_send_byte(*buffer);
    
    //Point to the next one
    buffer++;
    
    //One byte done
    size--;
  }
  
  //Stop the communication sequence
  tp_i2c_send_stop();
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_read_data(uint16 reg_addr, uint8 *buffer, uint32 size)
{
  //Start a communication sequence
  tp_i2c_send_start();

  //Send the device address for writing
  tp_i2c_send_byte(TP_DEVICE_ADDR_WRITE);

  //Send the register address high byte first
  tp_i2c_send_byte(reg_addr >> 8);

  //Send the low byte second
  tp_i2c_send_byte(reg_addr);

  //Re start the communication sequence for reading
  tp_i2c_send_start();
  
  //Send the device address for writing
  tp_i2c_send_byte(TP_DEVICE_ADDR_READ);
  
  //Read all the requested bytes
  while(size)
  {
    //Read a byte from the device
    *buffer = tp_i2c_read_byte();
    
    //Point to the next entry in the buffer
    buffer++;
    
    //One byte done
    size--;
    
    //Check if more data coming
    if(size)
    {
      //Send an ack if so
      tp_i2c_clock_ack();
    }
    else
    {
      //Send a nack if finished
      tp_i2c_clock_nack();
    }
  }
  
  //Stop the communication sequence
  tp_i2c_send_stop();
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_send_start(void)
{
  //Make sure INT is set as input and the rest of the signals as output
  *PORT_A_CFG_REG = 0xFFFF1101;

  //Make SDA high
  *PORT_A_DATA_REG |= 0x00000004;

  //Wait for a while
  tp_delay(TP_DATA_DELAY);
  
  //Make SCL high
  *PORT_A_DATA_REG |= 0x00000008;
  
  //Wait for a while
  tp_delay(TP_CLOCK_DELAY);
  
  //Make SDA low
  *PORT_A_DATA_REG &= 0x0000000B;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);

  //Make SCL low
  *PORT_A_DATA_REG &= 0x00000007;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_send_stop(void)
{
  //Make sure INT is set as input and the rest of the signals as output
  *PORT_A_CFG_REG = 0xFFFF1101;

  //Make SDA low
  *PORT_A_DATA_REG &= 0x0000000B;

  //Wait for a while
  tp_delay(TP_DATA_DELAY);
  
  //Make SCL high
  *PORT_A_DATA_REG |= 0x00000008;
  
  //Wait for a while
  tp_delay(TP_CLOCK_DELAY);
  
  //Make SDA high
  *PORT_A_DATA_REG |= 0x00000004;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_clock_ack(void)
{
  //Make sure INT is set as input and the rest of the signals as output
  *PORT_A_CFG_REG = 0xFFFF1101;
  
  //Make SDA low
  *PORT_A_DATA_REG &= 0x0000000B;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);

  //Make SCL high
  *PORT_A_DATA_REG |= 0x00000008;
  
  //Wait for a while
  tp_delay(TP_CLOCK_DELAY);

  //Make SCL low
  *PORT_A_DATA_REG &= 0x00000007;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_clock_nack(void)
{
  //Make sure INT is set as input and the rest of the signals as output
  *PORT_A_CFG_REG = 0xFFFF1101;
  
  //Make SDA high
  *PORT_A_DATA_REG |= 0x00000004;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);

  //Make SCL high
  *PORT_A_DATA_REG |= 0x00000008;
  
  //Wait for a while
  tp_delay(TP_CLOCK_DELAY);

  //Make SCL low
  *PORT_A_DATA_REG &= 0x00000007;
  
  //Wait for a while
  tp_delay(TP_DATA_DELAY);
}

//----------------------------------------------------------------------------------------------------------------------------------

void tp_i2c_send_byte(uint8 data)
{
  int i;
  
  //Make sure INT is set as input and the rest of the signals as output
  *PORT_A_CFG_REG = 0xFFFF1101;

  //Send 8 bits
  for(i=0;i<8;i++)
  {
    //Check if bit to send is high or low
    if(data & 0x80)
    {
      //Make SDA high
      *PORT_A_DATA_REG |= 0x00000004;
    }
    else
    {
      //Make SDA low
      *PORT_A_DATA_REG &= 0x0000000B;
    }
  
    //Wait for a while
    tp_delay(TP_DATA_DELAY);
    
    //Make SCL high
    *PORT_A_DATA_REG |= 0x00000008;

    //Wait for a while
    tp_delay(TP_CLOCK_DELAY);

    //Make SCL low
    *PORT_A_DATA_REG &= 0x00000007;

    //Wait for a while
    tp_delay(TP_DATA_DELAY);
    
    //Select the next bit to send
    data <<= 1;
  }
  
  //Clock the ack bit
  tp_i2c_clock_ack();
}

//----------------------------------------------------------------------------------------------------------------------------------

uint8 tp_i2c_read_byte(void)
{
  int   i;
  uint8 data = 0;
  
  //Set SDA for input
  *PORT_A_CFG_REG = 0xFFFF1001;
  
  //Read all the bits
  for(i=0;i<8;i++)
  {
    //Make SCL high
    *PORT_A_DATA_REG |= 0x00000008;

    //Wait for a while
    tp_delay(TP_DATA_DELAY);
    
    //Shift the bits to make room for the next one
    data <<= 1;
    
    //Put in the new bit
    data |= (*PORT_A_DATA_REG & 0x00000004) >> 2;

    //Wait for a while
    tp_delay(TP_DATA_DELAY);
    
    //Make SCL low
    *PORT_A_DATA_REG &= 0x00000007;

    //Wait for a while
    tp_delay(TP_CLOCK_DELAY);
  }
 
  return(data);
}

//----------------------------------------------------------------------------------------------------------------------------------
//A count of 4 is approximately 3uS when running on 600MHz with cache enabled

void tp_delay(uint32 usec)
#if 0
{
  int i;
  
  for(i=0;i<usec;i++);
}
#else
{
  //Lower then 64 does not work properly, because the panel fails to hold the new configuration when coming from the original code
  unsigned int loops = usec * 90;

  __asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"  "bne 1b":"=r"(loops):"0"(loops));
}
#endif
  
//----------------------------------------------------------------------------------------------------------------------------------
