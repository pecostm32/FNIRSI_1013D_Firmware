//----------------------------------------------------------------------------------------------------------------------------------

#ifndef USB_INTERFACE_H
#define USB_INTERFACE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define USBC_REG_FADDR        ((volatile uint8  *)(0x01c13098))
#define USBC_REG_PCTL         ((volatile uint8  *)(0x01c13040))
#define USBC_REG_INTTX        ((volatile uint16 *)(0x01c13044))
#define USBC_REG_INTRX        ((volatile uint16 *)(0x01c13046))
#define USBC_REG_INTTXE       ((volatile uint16 *)(0x01c13048))
#define USBC_REG_INTRXE       ((volatile uint16 *)(0x01c1304A))
#define USBC_REG_INTUSB       ((volatile uint8  *)(0x01c1304C))
#define USBC_REG_INTUSBE      ((volatile uint8  *)(0x01c13050))
#define USBC_REG_FRNUM        ((volatile uint32 *)(0x01c13054))
#define USBC_REG_EPIND        ((volatile uint8  *)(0x01c13042))
#define USBC_REG_TMCTL        ((volatile uint32 *)(0x01c1307C))

#define USBC_REG_TXMAXP       ((volatile uint16 *)(0x01c13080))
#define USBC_REG_CSR0         ((volatile uint16 *)(0x01c13082))
#define USBC_REG_TXCSR        ((volatile uint16 *)(0x01c13082))
#define USBC_REG_RXMAXP       ((volatile uint16 *)(0x01c13084))
#define USBC_REG_RXCSR        ((volatile uint16 *)(0x01c13086))
#define USBC_REG_COUNT0       ((volatile uint16 *)(0x01c13088))
#define USBC_REG_RXCOUNT      ((volatile uint16 *)(0x01c13088))
#define USBC_REG_EP0TYPE      ((volatile uint32 *)(0x01c1308C))
#define USBC_REG_TXTYPE       ((volatile uint32 *)(0x01c1308C))
#define USBC_REG_NAKLIMIT0    ((volatile uint32 *)(0x01c1308D))
#define USBC_REG_TXINTERVAL   ((volatile uint32 *)(0x01c1308D))
#define USBC_REG_RXTYPE       ((volatile uint32 *)(0x01c1308E))
#define USBC_REG_RXINTERVAL   ((volatile uint32 *)(0x01c1308F))

#define USBC_REG_CONFIGDATA   ((volatile uint32 *)(0x01c130c0))

#define USBC_REG_EPFIFO0      ((volatile uint32 *)(0x01c13000))
#define USBC_REG_EPFIFO1      ((volatile uint32 *)(0x01c13004))
#define USBC_REG_EPFIFO2      ((volatile uint32 *)(0x01c13008))
#define USBC_REG_EPFIFO3      ((volatile uint32 *)(0x01c1300C))
#define USBC_REG_EPFIFO4      ((volatile uint32 *)(0x01c13010))
#define USBC_REG_EPFIFO5      ((volatile uint32 *)(0x01c13014))

#define USBC_REG_DEVCTL       ((volatile uint32 *)(0x01c13041))

#define USBC_REG_TXFIFOSZ     ((volatile uint8  *)(0x01c13090))
#define USBC_REG_RXFIFOSZ     ((volatile uint8  *)(0x01c13094))
#define USBC_REG_TXFIFOAD     ((volatile uint16 *)(0x01c13092))
#define USBC_REG_RXFIFOAD     ((volatile uint16 *)(0x01c13096))

#define USBC_REG_VEND0        ((volatile uint8 *)(0x01c13043))
#define USBC_REG_VEND1        ((volatile uint32 *)(0x01c1307D))
#define USBC_REG_VEND3        ((volatile uint32 *)(0x01c1307E))

#define USBC_REG_EPINFO       ((volatile uint32 *)(0x01c13078))
#define USBC_REG_RAMINFO      ((volatile uint32 *)(0x01c13079))
#define USBC_REG_LINKINFO     ((volatile uint32 *)(0x01c1307A))
#define USBC_REG_VPLEN        ((volatile uint32 *)(0x01c1307B))
#define USBC_REG_HSEOF        ((volatile uint32 *)(0x01c1307C))
#define USBC_REG_FSEOF        ((volatile uint32 *)(0x01c1307D))
#define USBC_REG_LSEOF        ((volatile uint32 *)(0x01c1307E))

#define USBC_REG_RPCOUNT      ((volatile uint32 *)(0x01c1308A))

//new
#define USBC_REG_ISCR         ((volatile uint32 *)(0x01c13400))
#define USBC_REG_PHYCTL       ((volatile uint32 *)(0x01c13404))
#define USBC_REG_PHYBIST      ((volatile uint32 *)(0x01c13408))
#define USBC_REG_PHYTUNE      ((volatile uint32 *)(0x01c1340c))

#define USBC_REG_CSR          ((volatile uint32 *)(0x01c13410))

#define USBC_REG_PMU_IRQ      ((volatile uint32 *)(0x01c13800))

//----------------------------------------------------------------------------------------------------------------------------------

#define SYS_CNTRL             ((volatile uint32 *)(0x01c00000))
#define SYS_CNTRL_USB_FIFO    ((volatile uint32 *)(0x01c00004))

//----------------------------------------------------------------------------------------------------------------------------------

#define USB_EP0_FIFO_SIZE                       64

//----------------------------------------------------------------------------------------------------------------------------------

#define USB_SPEED_UNKNOWN                       0
#define USB_SPEED_LOW                           1       //usb 1.1
#define USB_SPEED_FULL                          2       //usb 1.1
#define USB_SPEED_HIGH                          3       //usb 2.0

//----------------------------------------------------------------------------------------------------------------------------------

#define EP0_IDLE                                0
#define EP0_IN_DATA_PHASE                       1
#define EP0_OUT_DATA_PHASE                      2
#define EP0_END_XFER                            3
#define EP0_STALL                               4

//----------------------------------------------------------------------------------------------------------------------------------

#define USB_TYPE_MASK                           0x60

#define USB_TYPE_STANDARD                       0x00
#define USB_TYPE_CLASS                          0x20
#define USB_TYPE_VENDOR                         0x40
#define USB_TYPE_RESERVED                       0x60

//----------------------------------------------------------------------------------------------------------------------------------

#define USB_RECIP_DEVICE                        0x00
#define USB_RECIP_INTERFACE                     0x01
#define USB_RECIP_ENDPOINT                      0x02
#define USB_RECIP_OTHER                         0x03

//----------------------------------------------------------------------------------------------------------------------------------
//Standard requests

#define USB_REQ_GET_STATUS                      0x00
#define USB_REQ_CLEAR_FEATURE                   0x01
#define USB_REQ_SET_FEATURE                     0x03
#define USB_REQ_SET_ADDRESS                     0x05
#define USB_REQ_GET_DESCRIPTOR                  0x06
#define USB_REQ_SET_DESCRIPTOR                  0x07
#define USB_REQ_GET_CONFIGURATION               0x08
#define USB_REQ_SET_CONFIGURATION               0x09
#define USB_REQ_GET_INTERFACE                   0x0A
#define USB_REQ_SET_INTERFACE                   0x0B
#define USB_REQ_SYNCH_FRAME                     0x0C

//----------------------------------------------------------------------------------------------------------------------------------

#define USBC_BP_ISCR_DPDM_CHANGE_DETECT         0x00000010
#define USBC_BP_ISCR_ID_CHANGE_DETECT           0x00000020
#define USBC_BP_ISCR_VBUS_CHANGE_DETECT         0x00000040

#define USBC_BP_ISCR_VBUS_VALID_SRC             0x00000C00
#define USBC_BP_ISCR_FORCE_VBUS_VALID           0x00003000
#define USBC_BP_ISCR_FORCE_ID                   0x0000C000
#define USBC_BP_ISCR_DPDM_PULLUP_EN             0x00010000
#define USBC_BP_ISCR_ID_PULLUP_EN               0x00020000

//----------------------------------------------------------------------------------------------------------------------------------
//USB Power Control for device only
#define USBC_BP_POWER_D_ISO_UPDATE_EN           0x80
#define USBC_BP_POWER_D_SOFT_CONNECT            0x40
#define USBC_BP_POWER_D_HIGH_SPEED_EN           0x20
#define USBC_BP_POWER_D_HIGH_SPEED_FLAG         0x10
#define USBC_BP_POWER_D_RESET_FLAG              0x08
#define USBC_BP_POWER_D_RESUME                  0x04
#define USBC_BP_POWER_D_SUSPEND                 0x02
#define USBC_BP_POWER_D_ENABLE_SUSPENDM         0x01

//----------------------------------------------------------------------------------------------------------------------------------
//USB interrupt
#define USBC_BP_INTUSB_VBUS_ERROR               0x80
#define USBC_BP_INTUSB_SESSION_REQ              0x40
#define USBC_BP_INTUSB_DISCONNECT               0x20
#define USBC_BP_INTUSB_CONNECT                  0x10
#define USBC_BP_INTUSB_SOF                      0x08
#define USBC_BP_INTUSB_RESET                    0x04
#define USBC_BP_INTUSB_RESUME                   0x02
#define USBC_BP_INTUSB_SUSPEND                  0x01

//----------------------------------------------------------------------------------------------------------------------------------
//Control and Status Register for EP0 for device only
#define USBC_BP_CSR0_D_FLUSH_FIFO               0x0100
#define USBC_BP_CSR0_D_SERVICED_SETUP_END       0x0080
#define USBC_BP_CSR0_D_SERVICED_RX_PKT_READY    0x0040
#define USBC_BP_CSR0_D_SEND_STALL               0x0020
#define USBC_BP_CSR0_D_SETUP_END                0x0010
#define USBC_BP_CSR0_D_DATA_END                 0x0008
#define USBC_BP_CSR0_D_SENT_STALL               0x0004
#define USBC_BP_CSR0_D_TX_PKT_READY             0x0002
#define USBC_BP_CSR0_D_RX_PKT_READY             0x0001

//----------------------------------------------------------------------------------------------------------------------------------

#define USBC_INTTX_FLAG_EP5               0x20
#define USBC_INTTX_FLAG_EP4               0x10
#define USBC_INTTX_FLAG_EP3               0x08
#define USBC_INTTX_FLAG_EP2               0x04
#define USBC_INTTX_FLAG_EP1               0x02
#define USBC_INTTX_FLAG_EP0               0x01

//----------------------------------------------------------------------------------------------------------------------------------

#define USBC_INTRX_FLAG_EP5               0x20
#define USBC_INTRX_FLAG_EP4               0x10
#define USBC_INTRX_FLAG_EP3               0x08
#define USBC_INTRX_FLAG_EP2               0x04
#define USBC_INTRX_FLAG_EP1               0x02

//----------------------------------------------------------------------------------------------------------------------------------
//TX EP Control and Status Register for Device only

#define USBC_BP_TXCSR_D_AUTOSET            0x8000
#define USBC_BP_TXCSR_D_ISO                0x4000
#define USBC_BP_TXCSR_D_MODE               0x2000
#define USBC_BP_TXCSR_D_DMA_REQ_EN         0x1000
#define USBC_BP_TXCSR_D_FORCE_DATA_TOGGLE  0x0800
#define USBC_BP_TXCSR_D_DMA_REQ_MODE       0x0400
#define USBC_BP_TXCSR_D_INCOMPLETE         0x0080
#define USBC_BP_TXCSR_D_CLEAR_DATA_TOGGLE  0x0040
#define USBC_BP_TXCSR_D_SENT_STALL         0x0020
#define USBC_BP_TXCSR_D_SEND_STALL         0x0010
#define USBC_BP_TXCSR_D_FLUSH_FIFO         0x0008
#define USBC_BP_TXCSR_D_UNDER_RUN          0x0004
#define USBC_BP_TXCSR_D_FIFO_NOT_EMPTY     0x0002
#define USBC_BP_TXCSR_D_TX_READY           0x0001

//----------------------------------------------------------------------------------------------------------------------------------
//RX Max Packet

#define USBC_BP_RXMAXP_PACKET_COUNT        0x0800
#define USBC_BP_RXMAXP_MAXIMUM_PAYLOAD     0x0001

//----------------------------------------------------------------------------------------------------------------------------------
//RX EP Control and Status Register for Device only

#define  USBC_BP_RXCSR_D_AUTO_CLEAR           0x8000
#define  USBC_BP_RXCSR_D_ISO                  0x4000
#define  USBC_BP_RXCSR_D_DMA_REQ_EN           0x2000
#define  USBC_BP_RXCSR_D_DISABLE_NYET         0x1000
#define  USBC_BP_RXCSR_D_DMA_REQ_MODE         0x0800
#define  USBC_BP_RXCSR_D_INCOMPLETE           0x0100
#define  USBC_BP_RXCSR_D_CLEAR_DATA_TOGGLE    0x0080
#define  USBC_BP_RXCSR_D_SENT_STALL           0x0040
#define  USBC_BP_RXCSR_D_SEND_STALL           0x0020
#define  USBC_BP_RXCSR_D_FLUSH_FIFO           0x0010
#define  USBC_BP_RXCSR_D_DATA_ERROR           0x0008
#define  USBC_BP_RXCSR_D_OVERRUN              0x0004
#define  USBC_BP_RXCSR_D_FIFO_FULL            0x0002
#define  USBC_BP_RXCSR_D_RX_PKT_READY         0x0001

//----------------------------------------------------------------------------------------------------------------------------------
//TX EP FIFO size control

#define USBC_BP_RXFIFOSZ_DPB                  0x10

//----------------------------------------------------------------------------------------------------------------------------------
//TX EP FIFO size control

#define USBC_BP_TXFIFOSZ_DPB                  0x10

//----------------------------------------------------------------------------------------------------------------------------------

#define DEVICE_DESCRIPTOR                      1
#define CONFIGURATION_DESCRIPTOR               2
#define STRING_DESCRIPTOR                      3
#define INTERFACE_DESCRIPTOR                   4
#define ENDPOINT_DESCRIPTOR                    5
#define DEVICE_QUALIFIER_DESCRIPTOR            6
#define OTHER_SPEED_CONFIGURATION_DESCRIPTOR   7
#define INTERFACE_POWER1_DESCRIPTOR            8
#define INTERFACE_ASSOC_DESCRIPTOR            11
#define HID_DESCRIPTOR_TYPE                   33
#define REPORT_DESCRIPTOR                     34

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint8  bLength;
  uint8  bDescriptorType;
  uint16 bcdUSB;
  uint8  bDeviceClass;
  uint8  bDeviceSubClass;
  uint8  bDeviceProtocol;
  uint8  bMaxPacketSize0;
  uint16 idVendor;
  uint16 idProduct;
  uint16 bcdDevice;
  uint8  iManufacturer;
  uint8  iProduct;
  uint8  iSerialNumber;
  uint8  bNumConfigurations;
} __attribute__ ((packed)) USB_DeviceDescriptor;

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint8  bLength;
  uint8  bDescriptorType;
  uint16 wTotalLength;
  uint8  bNumInterfaces;
  uint8  bConfigurationValue;
  uint8  iConfiguration;
  uint8  bmAttributes;
  uint8  MaxPower;
} __attribute__ ((packed)) USB_ConfigDescriptor;

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint8 bLength;
  uint8 bDescriptorType;
  uint8 bInterfaceNumber;
  uint8 bAlternateSetting;
  uint8 bNumEndpoints;
  uint8 bInterfaceClass;
  uint8 bInterfaceSubClass;
  uint8 bInterfaceProtocol;
  uint8 iInterface;
} __attribute__ ((packed)) USB_InterfaceDescriptor;

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint8  bLegth;
  uint8  bDescriptorType;
  uint8  bEndpointAddress;
  uint8  bmAttributes;
  uint16 wMaxPacketSize;
  uint8  bInterval;
} __attribute__ ((packed)) USB_EndPointDescriptor;

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  USB_ConfigDescriptor    configuration_descriptor;
  USB_InterfaceDescriptor interface_descritor;
  USB_EndPointDescriptor  endpoint_descriptor[2];
} __attribute__ ((packed)) Mass_Storage_Descriptor;

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
  uint8  bRequestType;
  uint8  bRequest;
  uint16 wValue;
  uint16 wIndex;
  uint16 wLength;
} __attribute__ ((packed)) USB_Setup_Packet;

//----------------------------------------------------------------------------------------------------------------------------------

void usb_device_init(void);

void usb_device_disable(void);
void usb_device_enable(void);

void usb_write_to_fifo(void *FIFO, void *buffer, uint32 length);
void usb_read_from_fifo(void *FIFO, void *buffer, uint32 length);

void usb_device_stall_tx_ep(void);
void usb_device_stall_rx_ep(void);

void usb_write_ep1_data(void *buffer, uint32 length);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* USB_INTERFACE_H */

