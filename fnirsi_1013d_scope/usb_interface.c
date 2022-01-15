//----------------------------------------------------------------------------------------------------------------------------------

#include "ccu_control.h"
#include "usb_interface.h"
#include "interrupt.h"

#include "mass_storage_class.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_irq_handler(void);

void usb_mass_storage_standard_request(void *dat);

int32 usb_device_write_data_ep_pack(int32 ep, uint8 * databuf, int32 len);

//----------------------------------------------------------------------------------------------------------------------------------

uint8 current_speed = USB_SPEED_UNKNOWN;
uint32 usb_connect = 0;
int32 usb_ep0_state = EP0_IDLE;

extern volatile uint32 msc_state;

volatile uint32 usb_set_faddr = 0;
volatile uint32 usb_faddr = 0;

volatile uint32  ep0_data_length = 0;
volatile uint8  *ep0_data_pointer;

union
{
  uint32           data[2];
  USB_Setup_Packet packet;
} usb_setup_packet;

//----------------------------------------------------------------------------------------------------------------------------------
//In original code the register is written as byte, which might be wrong since some data is shifted 8 bits to the left
//So using 32 bit register here
static void usb_phy_setup(int32 addr, int32 data, int32 len)
{
  int32 j;

  //Clear the clock bit
  *USBC_REG_CSR &= 0xFFFFFFFE;
  
  for(j=0;j<len;j++)
  {
    //set the bit address to be written
    *USBC_REG_CSR &= 0xFFFF00FF;
    *USBC_REG_CSR |= ((addr + j) << 8);
    
    //set or clear data bit
    if(data & 0x1)
    {
      *USBC_REG_CSR |= 0x00000080;
    }
    else
    {
      *USBC_REG_CSR &= 0xFFFFFF7F;
    }

    //Create some sort of clock pulse
    *USBC_REG_CSR |= 0x00000001;
    *USBC_REG_CSR &= 0xFFFFFFFE;
    
    //Next data bit
    data >>= 1;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_init(void)
{
  //Enable and clock the USB PHY
  *CCU_USBPHY_CFG |= (CCU_USBPHY_CFG_RST | CCU_USBPHY_CFG_GAT);
  
  //Enable the clock to the USB interface
  *CCU_BUS_CLK_GATE0 |= CCU_BCGR1_USBOTG_EN;
  
  //Lift the reset of the USB interface
  *CCU_BUS_SOFT_RST0 |= CCU_BSRR1_USBOTG_EN;
  
  //Regulation 45 ohms
  usb_phy_setup(0x0c, 0x01, 1);

  //Adjust PHY's magnitude and rate
  usb_phy_setup(0x20, 0x14, 5);

  //Threshold adjustment disconnect
  usb_phy_setup(0x2a, 3, 2);
  
  //Configurate the FIFO base
  *SYS_CNTRL_USB_FIFO = (*SYS_CNTRL_USB_FIFO & 0xFFFFFFFC) | 0x00000001;
  
  //Enable pull up resistors and force id high and bus valid.
  *USBC_REG_ISCR = USBC_BP_ISCR_DPDM_PULLUP_EN | USBC_BP_ISCR_ID_PULLUP_EN | USBC_BP_ISCR_FORCE_ID | USBC_BP_ISCR_FORCE_VBUS_VALID;  //Original code has 0x0C00 also set (USBC_BP_ISCR_VBUS_VALID_SRC)
  
  //Set interface to pio mode and use FIFO
  *USBC_REG_VEND0 = 0x00;

  //Disable the device and it's interrupts
  usb_device_disable();
  
  //Setup the interrupt handler for the USB interface
  setup_interrupt(USB_IRQ_NUM, usb_device_irq_handler, 2);
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_enable(void)
{
  //Start with unknown speed
  current_speed = USB_SPEED_UNKNOWN;

  //Setup for bulk by clearing the iso bit
  *USBC_REG_PCTL &= ~USBC_BP_POWER_D_ISO_UPDATE_EN;
  
  //Enable the high speed mode
  *USBC_REG_PCTL |= USBC_BP_POWER_D_HIGH_SPEED_EN;

  //Enable the needed interrupts
  *USBC_REG_INTUSBE = USBC_BP_INTUSB_SUSPEND | USBC_BP_INTUSB_RESUME | USBC_BP_INTUSB_RESET | USBC_BP_INTUSB_DISCONNECT;

  //Enable EP0 interrupt
  *USBC_REG_INTTXE = 1;

  //Switch the interface on
  *USBC_REG_PCTL |= USBC_BP_POWER_D_SOFT_CONNECT;
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_disable(void)
{
  //Disable all miscellaneous interrupts
  *USBC_REG_INTUSBE = 0;
  
  //Disable all receive interrupts
  *USBC_REG_INTRXE = 0;

  //Disable all transmit interrupts
  *USBC_REG_INTTXE = 0;

  //Clear all pending miscellaneous interrupts
  *USBC_REG_INTUSB = 0xFF;
  
  //Clear all pending receive interrupts
  *USBC_REG_INTRX = 0xFFFF;
  
  //Clear all pending transmit interrupts
  *USBC_REG_INTTX = 0xFFFF;
  
  //Switch the interface off
  *USBC_REG_PCTL &= ~USBC_BP_POWER_D_SOFT_CONNECT;
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_write_to_fifo(void *FIFO, void *buffer, uint32 length)
{
  //Based on the input buffer alignment different copy methods need to be used
  uint32 alignment = (uint32)buffer & 3;

  //Check on 32 bit aligned input buffer
  if(alignment == 0)
  {
    //Get the number of 32 bit items to do
    uint32 count = length / 4;

    //Take of the number of bytes that will be handled as 32 bit items
    length -= (count * 4);
    
    //Write the 32 bit items first
    while(count--)
    {
      //Write a 32 bit item
      (*(volatile uint32 *)FIFO) = *(uint32 *)buffer;

      //Skip to the next item in the buffer
      buffer += 4;
    }

    //Check if 16 bit item to write
    if(length > 1)
    {
      //Take of the number of bytes handled as 16 bit item
      length -= 2;
      
      //Write the 16 bit item
      (*(volatile uint16 *)FIFO) = *(uint16 *)buffer;

      //Skip to the next item in the buffer
      buffer += 2;
    }

    //Check if 8 bit item to write
    if(length & 1)
    {
      //Write the 8 bit item
      (*(volatile uint8 *)FIFO) = *(uint8 *)buffer;
    }
  }
  //Check if 16 bit aligned
  else if(alignment == 2)
  {
    //Get the number of 16 bit items to do
    uint32 count = length / 2;

    //Write the 16 bit items first
    while(count--)
    {
      //Write a 16 bit item
      (*(volatile uint16 *)FIFO) = *(uint16 *)buffer;

      //Skip to the next item in the buffer
      buffer += 2;
    }

    //Check if 8 bit item to write
    if(length & 1)
    {
      //Write the 8 bit item
      (*(volatile uint8 *)FIFO) = *(uint8 *)buffer;
    }
  }
  else
  {
    //Write the items as 8 bit ones
    while(length--)
    {
      //Write a 8 bit item
      (*(volatile uint8 *)FIFO) = *(uint8 *)buffer;

      //Skip to the next item in the buffer
      buffer++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_read_from_fifo(void *FIFO, void *buffer, uint32 length)
{
  //Based on the input buffer alignment different copy methods need to be used
  uint32 alignment = (uint32)buffer & 3;

  //Check on 32 bit aligned input buffer
  if(alignment == 0)
  {
    //Get the number of 32 bit items to do
    uint32 count = length / 4;

    //Take of the number of bytes that will be handled as 32 bit items
    length -= (count * 4);
    
    //Read the 32 bit items first
    while(count--)
    {
      //Read a 32 bit item
      *(uint32 *)buffer = (*(volatile uint32 *)FIFO);

      //Skip to the next item in the buffer
      buffer += 4;
    }

    //Check if 16 bit item to read
    if(length > 1)
    {
      //Take of the number of bytes handled as 16 bit item
      length -= 2;
      
      //Read the 16 bit item
      *(uint16 *)buffer = (*(volatile uint16 *)FIFO);

      //Skip to the next item in the buffer
      buffer += 2;
    }

    //Check if 8 bit item to read
    if(length & 1)
    {
      //Read the 8 bit item
      *(uint8 *)buffer = (*(volatile uint8 *)FIFO);
    }
  }
  //Check if 16 bit aligned
  else if(alignment == 2)
  {
    //Get the number of 16 bit items to do
    uint32 count = length / 2;

    //Read the 16 bit items first
    while(count--)
    {
      //Read a 16 bit item
      *(uint16 *)buffer = (*(volatile uint16 *)FIFO);

      //Skip to the next item in the buffer
      buffer += 2;
    }

    //Check if 8 bit item to read
    if(length & 1)
    {
      //Read the 8 bit item
      *(uint8 *)buffer = (*(volatile uint8 *)FIFO);
    }
  }
  else
  {
    //Read the items as 8 bit ones
    while(length--)
    {
      //Read a 8 bit item
      *(uint8 *)buffer = (*(volatile uint8 *)FIFO);

      //Skip to the next item in the buffer
      buffer++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_stall_tx_ep(void)
{
  *USBC_REG_TXCSR |= USBC_BP_TXCSR_D_SEND_STALL;
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_stall_rx_ep(void)
{
  *USBC_REG_RXCSR |= USBC_BP_RXCSR_D_SEND_STALL;
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_write_ep1_data(void *buffer, uint32 length)
{
  usb_write_to_fifo((void *)USBC_REG_EPFIFO1, buffer, length);
  
  //Clear possible under run error and signal packet send
  //This bit needs to be cleared first, otherwise it fails for some reason
  *USBC_REG_TXCSR &= ~USBC_BP_TXCSR_D_UNDER_RUN;
  *USBC_REG_TXCSR |= USBC_BP_TXCSR_D_TX_READY;
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_irq_handler(void)
{
  //Reading these registers clear all the active interrupt bits, but they still need active clearing
  //So using the registers directly in the if statements does not work
  register uint32 usbirq = *USBC_REG_INTUSB;
  register uint32 txirq  = *USBC_REG_INTTX;
  register uint32 rxirq  = *USBC_REG_INTRX;
  
  //Check on a RESET interrupt
  if(usbirq & USBC_BP_INTUSB_RESET)
  {
    //Clear all pending miscellaneous interrupts
    *USBC_REG_INTUSB = 0xFF;
    
    //Clear all pending receive interrupts
    *USBC_REG_INTRX = 0xFFFF;

    //Clear all pending transmit interrupts
    *USBC_REG_INTTX = 0xFFFF;
    
    //Need to check if this needs to be this way
    usb_connect = 1;
    
    usb_ep0_state = EP0_IDLE;
    
    //Set EP0 as active one
    *USBC_REG_EPIND = 0;
    
    //Switch to default device address
    *USBC_REG_FADDR = 0;
    
    return;
  }

  //Check on a RESUME interrupt
  if(usbirq & USBC_BP_INTUSB_RESUME)
  {
    //Clear the interrupt
    *USBC_REG_INTUSB = USBC_BP_INTUSB_RESUME;
    
    //Back to connected state
    usb_connect = 1;
    usb_ep0_state = EP0_IDLE;
  }

  //Check on a SUSPEND interrupt
  if(usbirq & USBC_BP_INTUSB_SUSPEND)
  {
    //Clear the interrupt
    *USBC_REG_INTUSB = USBC_BP_INTUSB_SUSPEND;
    
    //No longer connected
    usb_connect = 0;
    usb_ep0_state = EP0_IDLE;
  }

  //Check on a DISCONNECT interrupt
  if(usbirq & USBC_BP_INTUSB_DISCONNECT)
  {
    //Clear the interrupt
    *USBC_REG_INTUSB = USBC_BP_INTUSB_DISCONNECT;

    //No longer connected
    usb_connect = 0;
    usb_ep0_state = EP0_IDLE;
    
    //Need to add code here to force the scope to go back to its normal mode???
    //Use a flag that is checked in the touch scan for the usb on/off
  }

  //Check on an EP0 interrupt
  if(txirq & USBC_INTTX_FLAG_EP0)
  {
    //Clear the interrupt
    *USBC_REG_INTTX = USBC_INTTX_FLAG_EP0;

    //When speed not previously set check what is negotiated by the host
    if(current_speed == USB_SPEED_UNKNOWN)
    {
      //Check the speed negotiated by the host
      if(*USBC_REG_PCTL & USBC_BP_POWER_D_HIGH_SPEED_FLAG)
      {
        current_speed = USB_SPEED_HIGH;
      }
      else
      {
        //Things will fail for low speed due to the end point size not being adjusted
        //Think it would also need a different device descriptor or configuration
        //Question is how big is the change it will be connected to a full speed only device
        current_speed = USB_SPEED_FULL;
      }
    }
    
    //Set EP0 as active one
    *USBC_REG_EPIND = 0;
    
    //Check if previous setup package was a set address command
    if(usb_set_faddr)
    {
      //If so set the received device address in the USB device
      *USBC_REG_FADDR = usb_faddr;
      
      //Only do this when requested
      usb_set_faddr = 0;
    }

    //Clear stall status if needed
    if(*USBC_REG_CSR0 & USBC_BP_CSR0_D_SENT_STALL)
    {
      //Clear the stall state. Is the only bit cleared when a 0 is written to it.
      *USBC_REG_CSR0 = 0x00;

      //Make sure system is back in idle state
      usb_ep0_state = EP0_IDLE;
    }
    else
    {
      //Clear setup end when needed
      if(*USBC_REG_CSR0 & USBC_BP_CSR0_D_SETUP_END)
      {
        //Signal setup end is serviced
        *USBC_REG_CSR0 = USBC_BP_CSR0_D_SERVICED_SETUP_END;

        //Make sure system is back in idle state
        usb_ep0_state = EP0_IDLE;
      }

      //Handle the data based on the current state
      switch(usb_ep0_state)
      {
        case EP0_IDLE:
          //Check if a packet has been received
          if(*USBC_REG_CSR0 & USBC_BP_CSR0_D_RX_PKT_READY)
          {
            //Count is only valid when receive packet ready bit is set
            //A setup packet is always 8 bytes in length
            if(*USBC_REG_COUNT0 == 8)
            {
              //Get 8 bytes of data from the FIFO into the setup packet structure
              //Using a union for alignment to 32 bits
              usb_setup_packet.data[0] = *USBC_REG_EPFIFO0;
              usb_setup_packet.data[1] = *USBC_REG_EPFIFO0;
              
              //Signal packet has been serviced
              *USBC_REG_CSR0 = USBC_BP_CSR0_D_SERVICED_RX_PKT_READY;
              
              //Handle the packet based on the type
              switch(usb_setup_packet.packet.bRequestType & USB_TYPE_MASK)
              {
                case USB_TYPE_STANDARD:
                  switch(usb_setup_packet.packet.bRequest)
                  {
                    case USB_REQ_GET_DESCRIPTOR:
                    {
                      //No data to send yet
                      ep0_data_length = 0;
                      
                      //Handle the request based on the type of descriptor
                      switch(usb_setup_packet.packet.wValue >> 8)
                      {
                        case DEVICE_DESCRIPTOR:
                          ep0_data_length  = sizeof(USB_DeviceDescriptor);
                          ep0_data_pointer = (uint8 *)&Mass_Storage_DevDesc;
                          break;

                        case CONFIGURATION_DESCRIPTOR:
                          ep0_data_length  = sizeof(Mass_Storage_ConfDesc);
                          ep0_data_pointer = (uint8 *)&Mass_Storage_ConfDesc;
                          break;

                        case STRING_DESCRIPTOR:
                        {
                          switch(usb_setup_packet.packet.wValue & 0xFF)
                          {
                            case 0:
                              ep0_data_length  = sizeof(StringLangID);
                              ep0_data_pointer = (uint8 *)&StringLangID;
                              break;

                            case 1:
                            case 3:
                            case 0xEE:
                              ep0_data_length  = sizeof(StringVendor);
                              ep0_data_pointer = (uint8 *)&StringVendor;
                              break;

                            case 2:
                              ep0_data_length  = sizeof(StringProduct);
                              ep0_data_pointer = (uint8 *)&StringProduct;
                              break;

                            case 4:
                              ep0_data_length  = sizeof(StringInterface);
                              ep0_data_pointer = (uint8 *)&StringInterface;
                              break;

                            default:
                              ep0_data_length  = sizeof(StringSerial);
                              ep0_data_pointer = (uint8 *)&StringSerial;
                              break;
                          }
                        }
                        break;
                      }
                      
                      //Check if there is data to send
                      if(ep0_data_length)
                      {
                        //Limit the length on what the host allows
                        if(ep0_data_length > usb_setup_packet.packet.wLength)
                        {
                          ep0_data_length = usb_setup_packet.packet.wLength;
                        }
                        
                        //Need a separate variable for checking if the data length is more then the end point FIFO size
                        uint32 length = ep0_data_length;
                        
                        //Check if more data then the FIFO can hold
                        if(length > USB_EP0_FIFO_SIZE)
                        {
                          //Limit it to the FIFO size
                          length = USB_EP0_FIFO_SIZE;
                        }
                        
                        //Write the data to the FIFO
                        usb_write_to_fifo((void *)USBC_REG_EPFIFO0, (void *)ep0_data_pointer, length);

                        //Take of the number of bytes done
                        ep0_data_length -= length;

                        //Signal a package ready to send
                        *USBC_REG_CSR0 = USBC_BP_CSR0_D_TX_PKT_READY;
                        
                        //Check if there is more data to send to the host
                        if(ep0_data_length)
                        {
                          //Point to the next bit of the data that needs to be send in the next packet
                          ep0_data_pointer += length;
                          
                          //Switch to the state to send the remainder of the data
                          usb_ep0_state = EP0_IN_DATA_PHASE;
                        }
                      }
                    }
                    break;

                    case USB_REQ_SET_ADDRESS:
                      //Signal device address needs to be set on the next interrupt
                      usb_set_faddr = 1;
                      
                      //Save the received device address to be set in the USB device
                      usb_faddr = usb_setup_packet.packet.wValue & 0x7F;
                      break;

                    case USB_REQ_SET_CONFIGURATION:
                      //This needs attention to allow for full speed host connection
                      //Need to check if the double FIFO setup is needed. The rest of the code does not make use of it for as far as I can tell
                      //For the mass storage device two endpoints are used
                      //EP1 OUT for receiving data from the host
                      //EP1 IN for sending data to the host
                    
                      //Select the end point registers used for the mass storage
                      *USBC_REG_EPIND = 1;

                      //EP OUT configuration                      
                      //Reset the end point to bulk end point and clear the FIFO
                      *USBC_REG_RXCSR = USBC_BP_RXCSR_D_CLEAR_DATA_TOGGLE | USBC_BP_RXCSR_D_FLUSH_FIFO;
                      
                      //For double buffering clear the FIFO again
                      *USBC_REG_RXCSR = USBC_BP_RXCSR_D_FLUSH_FIFO;
                        
                      //Max 512 bytes per transaction
                      //This setting is dependent on the negotiated device speed
                      //Needs extra code to cope with that
                      *USBC_REG_RXMAXP = 512;
    
                      //The FIFO size is set based on 2^n * 8, so for 512 bytes it is 6
                      //As double buffering is used this bit is also set
                      *USBC_REG_RXFIFOSZ = USBC_BP_RXFIFOSZ_DPB | 6;
    
                      //The base address of the first FIFO is set to zero
                      *USBC_REG_RXFIFOAD = 0;
    
                      //EP IN configuration
                      //Reset the end point to bulk and clear the FIFO
                      *USBC_REG_TXCSR = USBC_BP_TXCSR_D_MODE | USBC_BP_TXCSR_D_CLEAR_DATA_TOGGLE | USBC_BP_TXCSR_D_FLUSH_FIFO;

                      //For double buffering clear the FIFO again
                      *USBC_REG_TXCSR = USBC_BP_TXCSR_D_FLUSH_FIFO;

                      //Max 512 bytes per transaction
                      //This setting is dependent on the negotiated device speed
                      //Needs extra code to cope with that
                      *USBC_REG_TXMAXP = 512;

                      //The FIFO size is set based on 2^n * 8, so for 512 bytes it is 6
                      //As double buffering is used this bit is also set
                      *USBC_REG_TXFIFOSZ = USBC_BP_TXFIFOSZ_DPB | 6;
                      
                      //The base address of the second FIFO is set to the first free address (twice the receive FIFO size / 8)
                      *USBC_REG_TXFIFOAD = 128;

                      //Enable the endpoint interrupts
                      *USBC_REG_INTRXE |= USBC_INTRX_FLAG_EP1;
                      *USBC_REG_INTTXE |= USBC_INTTX_FLAG_EP1;
  
                      //Clear the SCSI state to wait for command
                      msc_state = MSC_WAIT_COMMAND;
  
                      //Switch back to EP0
                      *USBC_REG_EPIND = 0;

                      //Send a null packet to acknowledge configuration
                      *USBC_REG_CSR0 = USBC_BP_CSR0_D_TX_PKT_READY;
                      break;

                    case USB_REQ_CLEAR_FEATURE:
                      //Might need more filtering. Looking at tinyusb project things are done differently
                      //This is extra in the scope. Not sure if it is really needed
                      
                      //Select the intended endpoint
                      *USBC_REG_EPIND = usb_setup_packet.packet.wIndex & 0x03;
                      
                      //Check the type of endpoint to halt
                      if(usb_setup_packet.packet.wIndex & 0x80)
                      {
                        //Transmit endpoint
                        *USBC_REG_TXCSR = USBC_BP_TXCSR_D_MODE | USBC_BP_TXCSR_D_CLEAR_DATA_TOGGLE | USBC_BP_TXCSR_D_FLUSH_FIFO;
                      }
                      else
                      {
                        //Receive endpoint
                        *USBC_REG_RXCSR = USBC_BP_RXCSR_D_CLEAR_DATA_TOGGLE | USBC_BP_RXCSR_D_FLUSH_FIFO;
                      }
                      
                      //Switch back to EP0
                      *USBC_REG_EPIND = 0;
                      break;
                  }
                  break;
                  
                case USB_TYPE_CLASS:
                  //For mass storage check if this is a max LUN request
                  if(usb_setup_packet.packet.bRequest == 0xFE)
                  {
                    //If so check if host allows for the needed data
                    if(usb_setup_packet.packet.wLength >= 1)
                    {
                      //Device has only one logical unit so send a zero
                      *(uint8 *)USBC_REG_EPFIFO0 = 0;
                      
                      //Signal data send and done with this packet
                      *USBC_REG_CSR0 = USBC_BP_CSR0_D_TX_PKT_READY;
                    }
                  }
//                  //Check if mass storage reset
//                  else if(usb_setup_packet.packet.bRequest == 0xFF)
//                  {
//                    //Mass storage reset might also be needed
//                  }
                  break;
              }

              //Check if done with data. This is only the case when not switched to another state
              if(usb_ep0_state == EP0_IDLE)
              {
                //Signal done with this packet
                *USBC_REG_CSR0 = USBC_BP_CSR0_D_DATA_END;
              }
            }
            else
            {
              //Signal packet has been serviced and stall the end point
              *USBC_REG_CSR0 = USBC_BP_CSR0_D_SERVICED_RX_PKT_READY | USBC_BP_CSR0_D_SEND_STALL;
            }
          }
          break;

        case EP0_IN_DATA_PHASE:
          if(ep0_data_length)
          {
            uint32 length = ep0_data_length;

            //Check if more data to send then the end point FIFO can handle
            if(ep0_data_length > USB_EP0_FIFO_SIZE)
            {
              //If so limit it to the FIFO size
              length = USB_EP0_FIFO_SIZE;
            }

            //Load the data into the FIFO
            usb_write_to_fifo((void *)USBC_REG_EPFIFO0, (void *)ep0_data_pointer, length);
            
            //Take of the data length just send
            ep0_data_length -= length;

            //Signal data packet ready to send
            *USBC_REG_CSR0 = USBC_BP_CSR0_D_TX_PKT_READY;
            
            //Check if there is more data that needs to be send
            if(ep0_data_length)
            {
              //Point to the next data to send
              ep0_data_pointer += length;
            }
            else
            {
              //Signal done with sending
              *USBC_REG_CSR0 = USBC_BP_CSR0_D_DATA_END;

              //Switch back to idle state when done
              usb_ep0_state = EP0_IDLE;
            }
          }
          break;
      }
    }
  }    

  //Check on data received for EP1
  if(rxirq & USBC_INTRX_FLAG_EP1)
  {
    //Clear the interrupt
    *USBC_REG_INTRX = USBC_INTTX_FLAG_EP1;

    //Set EP1 as active one
    *USBC_REG_EPIND = 1;

    //Check if the end point is in a stall
    if(*USBC_REG_RXCSR & USBC_BP_RXCSR_D_SENT_STALL)
    {
      //Clear the stall state
      *USBC_REG_RXCSR &= ~(USBC_BP_RXCSR_D_SENT_STALL | USBC_BP_RXCSR_D_SEND_STALL);
    }
  
    //Check if there is data to handle
    if(*USBC_REG_RXCSR & USBC_BP_RXCSR_D_RX_PKT_READY)
    {
      //Handle the received data in the mass storage code
      usb_mass_storage_out_ep_callback((void *)USBC_REG_EPFIFO1, *USBC_REG_RXCOUNT);

      //Signal done with the packet and clear possible errors
      *USBC_REG_RXCSR &= ~(USBC_BP_RXCSR_D_RX_PKT_READY | USBC_BP_RXCSR_D_OVERRUN);
    }
  }
  
  //Check on data transmitted for EP1
  if(txirq & USBC_INTTX_FLAG_EP1)
  {
    //Clear the interrupt
    *USBC_REG_INTTX = USBC_INTTX_FLAG_EP1;
    
    //Set EP1 as active one
    *USBC_REG_EPIND = 1;

    //Check if the end point is stalled
    if(*USBC_REG_TXCSR & USBC_BP_TXCSR_D_SENT_STALL)
    {
      //Clear the stall state if so
      *USBC_REG_TXCSR &= ~(USBC_BP_TXCSR_D_SENT_STALL | USBC_BP_TXCSR_D_SEND_STALL);
    }

    //Check if the FIFO is ready for more data
    if((*USBC_REG_TXCSR & USBC_BP_TXCSR_D_TX_READY) == 0)
    {
      //Have the mass storage code handle the request for data
      usb_mass_storage_in_ep_callback();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
