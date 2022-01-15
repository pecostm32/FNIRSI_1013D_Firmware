//----------------------------------------------------------------------------------------------------------------------------------

#ifndef MASS_STORAGE_CLASS_H
#define MASS_STORAGE_CLASS_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "variables.h"
#include "usb_interface.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define CONFIG_MASS_STORAGE_DESCRIPTOR_LEN   (sizeof(USB_ConfigDescriptor) + sizeof(USB_InterfaceDescriptor) + (sizeof(USB_EndPointDescriptor) * 2))

//----------------------------------------------------------------------------------------------------------------------------------

extern const USB_DeviceDescriptor Mass_Storage_DevDesc;

extern const Mass_Storage_Descriptor Mass_Storage_ConfDesc;

extern const uint8 StringLangID[4];
extern const uint8 StringVendor[62];
extern const uint8 StringProduct[38];
extern const uint8 StringSerial[30];
extern const uint8 StringInterface[30];
extern const uint8 scsi_inquiry_string[36];
extern const uint8 scsi_sense_data[4];

//----------------------------------------------------------------------------------------------------------------------------------

#define MSC_WAIT_COMMAND            0
#define MSC_SEND_DATA               1
#define MSC_RECEIVE_DATA            2
#define MSC_SEND_STATUS             3

//----------------------------------------------------------------------------------------------------------------------------------

#define MSC_CSW_STATUS_OK           0
#define MSC_CSW_STATUS_FAIL         1
#define MSC_CSW_STATUS_ERROR        2

//----------------------------------------------------------------------------------------------------------------------------------

#define MSC_CBW_LENGTH              31
#define MSC_CSW_LENGTH              13

//----------------------------------------------------------------------------------------------------------------------------------

#define MSC_CBW_SIGNATURE           0x43425355
#define MSC_CSW_SIGNATURE           0x53425355

//----------------------------------------------------------------------------------------------------------------------------------

#define SCSI_MAX_BLOCK_COUNT        (VIEW_THUMBNAIL_DATA_SIZE / 512)

//----------------------------------------------------------------------------------------------------------------------------------

#define SCSI_CMD_TEST_UNIT_READY                0x00

#define SCSI_CMD_INQUIRY                        0x12

#define SCSI_CMD_MODE_SELECT_6                  0x15

#define SCSI_CMD_MODE_SENSE_6                   0x1A

#define SCSI_CMD_START_STOP_UNIT                0x1B

#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL   0x1E

#define SCSI_CMD_READ_FORMAT_CAPACITY           0x23

#define SCSI_CMD_READ_CAPACITY_10               0x25

#define SCSI_CMD_READ_10                        0x28

#define SCSI_CMD_WRITE_10                       0x2A

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint32 signature;     //Signature that helps identify this data packet as a CBW. The signature field shall contain the value 43425355h (little endian), indicating a CBW.
  uint32 tag;           //Tag sent by the host. The device shall echo the contents of this field back to the host in the dCSWTagfield of the associated CSW.
  uint32 total_bytes;   //The number of bytes of data that the host expects to transfer on the Bulk-In or Bulk-Out endpoint (as indicated by the Direction bit) during the execution of this command.
  uint8  dir;           //Bit 7 of this field define transfer direction 0: Data-Out from host to the device. 1: Data-In from the device to the host.
  uint8  lun;           //The device Logical Unit Number (LUN) to which the command block is being sent.
  uint8  cmd_len;       //The valid length of the CBWCBin bytes. This defines the valid length of the command block. The only legal values are 1 through 16
  uint8  command[16];   //The command block to be executed by the device.
} __attribute__ ((packed)) MSC_Command_Wrapper;

typedef struct
{
  uint32 signature;     //Signature that helps identify this data packet as a CSW. The signature field shall contain the value 53425355h (little endian), indicating CSW.
  uint32 tag;           //The device shall set this field to the value received in the dCBWTag of the associated CBW.
  uint32 data_residue;  //For Data-Out the device shall report in the dCSWDataResiduethe difference between the amount of data expected as stated in the dCBWDataTransferLength, and the actual amount of data processed by the device.
  uint8  status;        //Indicates the success or failure of the command.
} __attribute__ ((packed)) MSC_Status_Wrapper;

//----------------------------------------------------------------------------------------------------------------------------------

void usb_mass_storage_out_ep_callback(void *fifo, int length);

void usb_mass_storage_in_ep_callback(void);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* MASS_STORAGE_CLASS_H */

