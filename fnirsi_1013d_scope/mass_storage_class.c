//----------------------------------------------------------------------------------------------------------------------------------

#include "mass_storage_class.h"
#include "sd_card_interface.h"

//----------------------------------------------------------------------------------------------------------------------------------


const USB_DeviceDescriptor Mass_Storage_DevDesc =
{
  sizeof (USB_DeviceDescriptor),
  DEVICE_DESCRIPTOR,
  0x0200,                 //Version 2.0
  0x00,
  0x00,
  0x00,
  USB_EP0_FIFO_SIZE,      //Ep0 FIFO size
  0x0483,                 //STM
  0x5720,                 //Mass storage device
  0x0200,                 //Release version
  0x01,                   //iManufacturer
  0x02,                   //iProduct
  0x00,                   //ISerial
  0x01
};

const Mass_Storage_Descriptor Mass_Storage_ConfDesc =
{
  {
    sizeof (USB_ConfigDescriptor),
    CONFIGURATION_DESCRIPTOR,
    CONFIG_MASS_STORAGE_DESCRIPTOR_LEN, //Total length of the Configuration descriptor
    0x01, //NumInterfaces
    0x01, //Configuration Value
    0x00, //Configuration Description String Index
    0xC0, //Self Powered, no remote wakeup
    0x32 //Maximum power consumption 500 mA
  },
  {
    sizeof (USB_InterfaceDescriptor),
    INTERFACE_DESCRIPTOR,
    0x00, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x02, //ep number
    0x08, //Interface Class    (Mass storage interface)
    0x06, //Interface Subclass
    0x50, //Interface Protocol (Bulk only transport)
    0x04  //Interface Description String Index
  },
  {
    {
      sizeof (USB_EndPointDescriptor),
      ENDPOINT_DESCRIPTOR,
      0x81, //endpoint 1 IN
      2,    //bulk
      512,  //IN EP FIFO size  512 bytes
      0
    },
    {
      sizeof (USB_EndPointDescriptor),
      ENDPOINT_DESCRIPTOR,
      0x01, //endpoint 1 OUT
      2,    //bulk
      512,  //OUT EP FIFO size  512 bytes
      0
    }
  }
};

//USB String Descriptors
const uint8 StringLangID[4] =
{
  0x04,
  0x03,
  0x09,
  0x04 // LangID = 0x0409: U.S. English
};

const uint8 StringVendor[62] =
{
  0x3E, // Size of Vendor string
  0x03, // bDescriptorType
  0x48, 0x00, 0x65, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x4A, 0x00, 0x65, 0x00, 0x20, 0x00, 0x53, 0x00,
  0x63, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x20, 0x00,
  0x44, 0x00, 0x72, 0x00, 0x69, 0x00, 0x76, 0x00, 0x65, 0x00, 0x73, 0x00, 0x20, 0x00, 0x2D, 0x00,
  0x20, 0x00, 0x48, 0x00, 0x32, 0x00, 0x37, 0x00, 0x35, 0x00, 0x30, 0x00, 
};


const uint8 StringProduct[38] =
{
  0x26, // bLength
  0x03, // bDescriptorType
  0x48, 0x00, 0x32, 0x00, 0x37, 0x00, 0x35, 0x00, 0x30, 0x00, 0x20, 0x00, 0x20, 0x00,
  0x55, 0x00, 0x73, 0x00, 0x62, 0x00, 0x20, 0x00, 0x44, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00,
  0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 
};

const uint8 StringSerial[30] =
{
  0x1E, // bLength
  0x03, // bDescriptorType
  0x43, 0x00, 0x44, 0x00, 0x43, 0x00, 0x20, 0x00, 0x41, 0x00, 0x43, 0x00, 0x4D, 0x00, 0x20, 0x00, 
  0x63, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x66, 0x00, 0x69, 0x00, 0x67, 0x00, 
};

const uint8 StringInterface[30] =
{
  0x1E, // bLength
  0x03, // bDescriptorType
  0x43, 0x00, 0x44, 0x00, 0x43, 0x00, 0x20, 0x00, 0x41, 0x00, 0x43, 0x00, 0x4D, 0x00, 0x20, 0x00, 
  0x63, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x66, 0x00, 0x69, 0x00, 0x67, 0x00, 
};

//----------------------------------------------------------------------------------------------------------------------------------

const uint8 scsi_inquiry_string[36] =
{
  0x00, 0x80, 0x02, 0x02, 0x20, 0x00, 0x00, 0x00, 0x46, 0x31, 0x43, 0x31, 0x30, 0x30, 0x53, 0x20, 
  0x20, 0x58, 0x69, 0x61, 0x6F, 0x54, 0x61, 0x6F, 0x51, 0x69, 0x20, 0x20, 0x44, 0x69, 0x73, 0x6B, 
  0x20, 0x31, 0x2E, 0x30
};

const uint8 scsi_sense_data[4] =
{
  0x03, 0x00, 0x00, 0x00
};

//----------------------------------------------------------------------------------------------------------------------------------

extern uint32 cardsectorsize;
extern uint32 cardsectors;

volatile uint32 scsi_start_lba;
volatile uint32 scsi_block_count;

volatile uint32 scsi_byte_count;
volatile uint32 scsi_bytes_received;


volatile uint8 *scsi_data_in_ptr;
volatile uint8 *scsi_data_end_ptr;

volatile uint32 scsi_available_blocks;

uint8 scsi_capacity[8];

volatile uint32 msc_state = MSC_WAIT_COMMAND;

MSC_Command_Wrapper scsi_cbw;
MSC_Status_Wrapper  scsi_csw;

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scsi_check_read10_write10(uint32 check)
{
  //Check on possible no data transfer
  if(scsi_cbw.total_bytes == 0)
  {
    //Check on SCSI case 2 (Hn < Di) or case 3 (Hn < Do)
    if(scsi_block_count)
    {
      //Signal an error
      scsi_csw.status = MSC_CSW_STATUS_ERROR;
    }
  }
  else
  {
    //Check if wrong direction specified (SCSI case 10 (Ho <> Di) or case 8 (Hi <> Do))
    if((scsi_cbw.dir & 0x80) == check)
    {
      //Signal an error
      scsi_csw.status = MSC_CSW_STATUS_ERROR;
    }
    //Check on SCSI case 4 (Hi > Dn) (READ10) or case 9 (Ho > Dn) (WRITE10)
    else if(scsi_block_count == 0)
    {
      //Signal a fail
      scsi_csw.status = MSC_CSW_STATUS_FAIL;
    }
    //Check on computed block size (SCSI case 7 (Hi < Di) (READ10) or case 13 (Ho < Do) (WRIT10))
    else if((scsi_cbw.total_bytes / scsi_block_count) == 0)
    {
      //Signal an error
      scsi_csw.status = MSC_CSW_STATUS_ERROR;
    }
  }

  //Check if there is a fault condition
  if(scsi_csw.status != MSC_CSW_STATUS_OK)
  {
    //Set data residue to total bytes since no data has been transfered yet
    scsi_csw.data_residue = scsi_cbw.total_bytes;

    //Send the error status
    usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);

    //Signal there was an error
    return(1);
  }
  
  //Signal all is OK
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_mass_storage_out_ep_callback(void *fifo, int length)
{
  //usb_handle_mass_storage_write in Ghidra

  
  //Data coming from the host contains a CBW and needs to be handled here

  
  //Need a state machine to handle the data from the host. On startup the system is in an idle state
  //When data is received in this state it should be a CBW that needs to be checked
  
  //State_Command
    //Check if the length is that of a CBW
    //Check if the signature matches
    //When invalid stall the endpoints and switch to need reset stage
  
    //When valid setup a CSW for the response
    //Handle the command given in the CBW


  //In case of a command failure
    //signal the failure status??                                                         (csw.status)
    //need to determine how many bytes still need to be send (total_bytes - transfered??) (csw.data_residue)
    //When there is still data that needs to be transfered the corresponding end point needs to be stalled (cbw.dir bit 7)
  
    //For the status there are two options, FAIL or ERROR
    //FAIL is the most used one
    //ERROR is in case the CBW states no data (total_bytes == 0) and the scsi command has a block count
    //ERROR is also when the command is read10 and the CBW states write (dir == 0x00)
    //ERROR is also when the command is write10 and the CBW states read (dir == 0x80)
    //ERROR is when the CBW total_bytes / block_count is zero
    //FAIL is when CBW states data while the block_count is zero
  
  //Handle host data based on the current state
  switch(msc_state)
  {
    case MSC_WAIT_COMMAND:
    {
      //LUN should be zero since this device only supports the single unit

      //The direction bit tells if the host will send data or the device needs to send data

      //Command length needs to be from 1 to 16, otherwise it is an error

      //Check if the data is valid
      if(length == MSC_CBW_LENGTH)
      {
        //If so read the CBW from the FIFO
        usb_read_from_fifo(fifo, (void *)&scsi_cbw, length);
        
        //Check if the signature matches
        if(scsi_cbw.signature == MSC_CBW_SIGNATURE)
        {
          //If so process the command

          //First 
          //Copy the tag to the CSW and set the CSW signature (can be done in a setup routine)
          scsi_csw.signature    = MSC_CSW_SIGNATURE;
          scsi_csw.tag          = scsi_cbw.tag;
          scsi_csw.data_residue = 0;
          scsi_csw.status       = MSC_CSW_STATUS_OK;

          //Parse the command
          switch(scsi_cbw.command[0])
          {
            case SCSI_CMD_INQUIRY:
              usb_write_ep1_data((void *)scsi_inquiry_string, sizeof(scsi_inquiry_string));

              //Switch to status state (No more data to send)
              msc_state = MSC_SEND_STATUS;
              break;

            case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
              //Use this command to block the return to scope mode if the host says so!!!

            //Ignore these commands for now
            case SCSI_CMD_TEST_UNIT_READY:
            case SCSI_CMD_START_STOP_UNIT:
              //Needs to be done here since the data needs to be ready before the in request is send from the host
              //Send the ok status
              usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);
              break;

            case SCSI_CMD_READ_CAPACITY_10:
              {
                uint32 sectors = cardsectors - 1;

                //Need to return the last logical block address and the block size
                scsi_capacity[0] = sectors >> 24;
                scsi_capacity[1] = sectors >> 16;
                scsi_capacity[2] = sectors >> 8;
                scsi_capacity[3] = sectors;

                scsi_capacity[4] = cardsectorsize >> 24;
                scsi_capacity[5] = cardsectorsize >> 16;
                scsi_capacity[6] = cardsectorsize >> 8;
                scsi_capacity[7] = cardsectorsize;

                usb_write_ep1_data((void *)scsi_capacity, sizeof(scsi_capacity));

                //Switch to status state (No more data to send)
                msc_state = MSC_SEND_STATUS;
              }
              break;

            case SCSI_CMD_MODE_SENSE_6:
              usb_write_ep1_data((void *)scsi_inquiry_string, sizeof(scsi_inquiry_string));

              //Switch to status state (No more data to send)
              msc_state = MSC_SEND_STATUS;
              break;

            case SCSI_CMD_READ_10:
              //This requires a bit more checking!!!!!
              //Get the lba to access
              scsi_start_lba = (scsi_cbw.command[2] << 24) | (scsi_cbw.command[3] << 16) | (scsi_cbw.command[4] << 8) | scsi_cbw.command[5];

              //get the block count
              scsi_block_count = (scsi_cbw.command[7] << 8) | scsi_cbw.command[8];

              //Check on errors
              if(scsi_check_read10_write10(0x00) != 0)
              {
                //Bail out when there is an error
                break;
              }
            
              //Use the thumbnail buffer for the SCSI data
              scsi_data_in_ptr = (uint8 *)viewthumbnaildata;

              //Check if more data than what fits the buffer
              if(scsi_block_count > SCSI_MAX_BLOCK_COUNT)
              {
                //Limit to the max
                scsi_available_blocks = SCSI_MAX_BLOCK_COUNT;
              }
              else
              {
                //Otherwise just load them all
                scsi_available_blocks = scsi_block_count;
              }

              //Read the data from the card
              //Need to handle error here. Need to calculate the size already done
              if(sd_card_read(scsi_start_lba, scsi_available_blocks, (uint8 *)scsi_data_in_ptr) != SD_OK)
              {
                //When the SD card fails send a FAIL
                scsi_csw.status = MSC_CSW_STATUS_FAIL;

                //Calculate the residual data length
                scsi_csw.data_residue = scsi_block_count * 512; 
                
                //Send the status
                usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);
              }

              //Write the first block to the FIFO
              usb_write_ep1_data((void *)scsi_data_in_ptr, cardsectorsize);

              //One full buffer done
              scsi_block_count -= scsi_available_blocks;

              //select the next sector to read
              scsi_start_lba += scsi_available_blocks;

              //Point to next data to transfer
              scsi_data_in_ptr += cardsectorsize;

              //Already one block done
              scsi_available_blocks--;
              
              //Switch to the send data state
              msc_state = MSC_SEND_DATA;
              break;


            case SCSI_CMD_WRITE_10:
              //This requires a bit more checking!!!!!
              //Get the lba to access
              scsi_start_lba = (scsi_cbw.command[2] << 24) | (scsi_cbw.command[3] << 16) | (scsi_cbw.command[4] << 8) | scsi_cbw.command[5];

              //get the block count
              scsi_block_count = (scsi_cbw.command[7] << 8) | scsi_cbw.command[8];

              //Check on errors
              if(scsi_check_read10_write10(0x80) != 0)
              {
                //Bail out when there is an error
                break;
              }
              
              //Need the number of bytes to receive
              scsi_byte_count = scsi_block_count * 512;
              scsi_bytes_received = 0;

              //Point to the start of the buffer to receive the payload data into
              scsi_data_in_ptr = (uint8 *)viewthumbnaildata;
              scsi_data_end_ptr = scsi_data_in_ptr + sizeof(viewthumbnaildata);

              //Next out transaction holds the payload data
              msc_state = MSC_RECEIVE_DATA;
              break;
              
            default:
              //For unsupported commands send a FAIL
              scsi_csw.status = MSC_CSW_STATUS_FAIL;
              
              //Send the status
              usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);
              break;
          }
        }
        else
        {
          //Not a valid command block wrapper signature then stall the endpoints
          usb_device_stall_tx_ep();
          usb_device_stall_rx_ep();
        }
      }
      else
      {
        //Not a valid command block wrapper length then stall the endpoints
        usb_device_stall_tx_ep();
        usb_device_stall_rx_ep();
      }
    }
    break;
      
    case MSC_RECEIVE_DATA:
    {
      //Calculate the last location when this data is written to the buffer. Needs to stay below the end of the buffer
      register uint8 *tptr = (uint8 *)scsi_data_in_ptr + length;
      
      //Check if there is still room in the buffer
      if(tptr < scsi_data_end_ptr)
      {
        //Need to load the data into a buffer before writing to the card
        usb_read_from_fifo(fifo, (void *)scsi_data_in_ptr, length);

        scsi_bytes_received += length;
        
        //Need to determine here if this was the last data
        if(length >= scsi_byte_count)
        {
          //Last payload data received so write it to the card and send the status
          //Get the number of sectors to write to the card to free the buffer
          uint32 sectors = scsi_bytes_received / 512;
          uint32 bytesdone = sectors * 512;

          //Check if there is a non full sector at the end
          if(bytesdone < scsi_bytes_received)
          {
            //If so do one sector extra
            sectors++;
          }
          
          //Write the data to the card and check on errors
          if(sd_card_write(scsi_start_lba, sectors, (uint8 *)viewthumbnaildata) != SD_OK)
          {
            //When there is an error signal it to the host
            scsi_csw.status = MSC_CSW_STATUS_FAIL;
          }
          
          //Next action is send the status to the host
          usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);

          //switch to wait for command state
          msc_state = MSC_WAIT_COMMAND;
        }
        else
        {
          //Take of the received bytes to see if more needs to come
          scsi_byte_count -= length;

          //Point to the next location in the buffer to store the next load
          scsi_data_in_ptr = tptr;
        }
      }
      else
      {
        //Get the number of sectors to write to the card to free the buffer
        uint32 sectors = scsi_bytes_received / 512;
        uint32 bytesdone = sectors * 512;
        
        //Start on the beginning of the buffer
        scsi_data_in_ptr = (uint8 *)viewthumbnaildata;
        
        //Write the data to the card and check on errors
        if(sd_card_write(scsi_start_lba, sectors, (uint8 *)scsi_data_in_ptr) != SD_OK)
        {
          //When there is an error signal it to the host
          scsi_csw.status = MSC_CSW_STATUS_FAIL;
            
          //Calculate the residual data length
          scsi_csw.data_residue = scsi_cbw.total_bytes - scsi_bytes_received; 
        }
        
        //Point to next logical block address to write to
        scsi_start_lba += sectors;
        
        //Check if there is data left
        if(bytesdone < scsi_bytes_received)
        {
          uint32 bytesleft = scsi_bytes_received - bytesdone;
          
          //If so copy the remainder to the start of the buffer
          memcpy((uint8 *)scsi_data_in_ptr, (uint8 *)scsi_data_in_ptr + bytesdone, bytesleft);
          
          //Set pointer to the end of this left over data
          scsi_data_in_ptr += bytesleft;
          
          //At this moment still data received
          scsi_bytes_received = bytesleft;
        }
        else
        {
          //Reset the number of received bytes for handling remainder of payload data
          scsi_bytes_received = 0;
        }
      }
    } 
    break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void usb_mass_storage_in_ep_callback(void)
{
  //Handle the data based on the mass storage state
  switch(msc_state)
  {
    case MSC_SEND_DATA:
      //Check if still more data to send to the host
      //Needs to be adapted to variable end point size!!
      if(scsi_available_blocks)
      {
        //Write the data to the FIFO
        usb_write_ep1_data((void *)scsi_data_in_ptr, cardsectorsize);

        //One more block done
        scsi_available_blocks--;

        //Point to next data to transfer
        scsi_data_in_ptr += cardsectorsize;
        
        //Check if more data needs to be read from the SD card
        if((scsi_available_blocks == 0) && scsi_block_count)
        {
          //Reset the pointer to the beginning of the buffer
          scsi_data_in_ptr = (uint8 *)viewthumbnaildata;

          //Check if that what still needs to be done is bigger then the buffer
          if(scsi_block_count > SCSI_MAX_BLOCK_COUNT)
          {
            //Limit to the size of the buffer
            scsi_available_blocks = SCSI_MAX_BLOCK_COUNT;
          }
          else
          {
            //Otherwise do the lot
            scsi_available_blocks = scsi_block_count;
          }
          
          //Read the next block as needed and check on errors
          if(sd_card_read(scsi_start_lba, scsi_available_blocks, (uint8 *)scsi_data_in_ptr) != SD_OK)
          {
            //When there is an error signal it to the host
            scsi_csw.status = MSC_CSW_STATUS_FAIL;

            //Calculate the residual data length
            scsi_csw.data_residue = scsi_block_count * 512; 
          }
          
          //One full buffer done
          scsi_block_count -= scsi_available_blocks;

          //Select the next sector to read
          scsi_start_lba += scsi_available_blocks;
        }        
        break;
      }
      
      //No more data to send then fall through to status
      
    case MSC_SEND_STATUS:
      //Send the status to the host
      usb_write_ep1_data((void *)&scsi_csw, MSC_CSW_LENGTH);
      
      //Switch to wait for command state
      msc_state = MSC_WAIT_COMMAND;
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
