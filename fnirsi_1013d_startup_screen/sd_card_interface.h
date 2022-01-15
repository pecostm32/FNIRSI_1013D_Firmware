//----------------------------------------------------------------------------------------------------------------------------------

#ifndef SD_CARD_INTERFACE_H
#define SD_CARD_INTERFACE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define SD0_GCTL         ((volatile uint32 *)(0x01C0F000))
#define SD0_CKCR         ((volatile uint32 *)(0x01C0F004))
#define SD0_TMOR         ((volatile uint32 *)(0x01C0F008))
#define SD0_BWDR         ((volatile uint32 *)(0x01C0F00C))
#define SD0_BKSR         ((volatile uint32 *)(0x01C0F010))
#define SD0_BYCR         ((volatile uint32 *)(0x01C0F014))
#define SD0_CMDR         ((volatile uint32 *)(0x01C0F018))
#define SD0_CAGR         ((volatile uint32 *)(0x01C0F01C))
#define SD0_RESP0        ((volatile uint32 *)(0x01C0F020))
#define SD0_RESP1        ((volatile uint32 *)(0x01C0F024))
#define SD0_RESP2        ((volatile uint32 *)(0x01C0F028))
#define SD0_RESP3        ((volatile uint32 *)(0x01C0F02C))
#define SD0_IMKR         ((volatile uint32 *)(0x01C0F030))
#define SD0_MISR         ((volatile uint32 *)(0x01C0F034))
#define SD0_RISR         ((volatile uint32 *)(0x01C0F038))
#define SD0_STAR         ((volatile uint32 *)(0x01C0F03C))
#define SD0_FWLR         ((volatile uint32 *)(0x01C0F040))
#define SD0_FUNS         ((volatile uint32 *)(0x01C0F044))
#define SD0_CBCR         ((volatile uint32 *)(0x01C0F048))
#define SD0_BBCR         ((volatile uint32 *)(0x01C0F04C))
#define SD0_DBGC         ((volatile uint32 *)(0x01C0F050))
#define SD0_A12A         ((volatile uint32 *)(0x01C0F058))
#define SD0_HWRST        ((volatile uint32 *)(0x01C0F078))
#define SD0_DMAC         ((volatile uint32 *)(0x01C0F080))
#define SD0_DLBA         ((volatile uint32 *)(0x01C0F084))
#define SD0_IDST         ((volatile uint32 *)(0x01C0F088))
#define SD0_IDIE         ((volatile uint32 *)(0x01C0F08C))
#define SD0_CHDA         ((volatile uint32 *)(0x01C0F090))
#define SD0_CBDA         ((volatile uint32 *)(0x01C0F094))
#define SD0_CARD_THLDC   ((volatile uint32 *)(0x01C0F100))
#define SD0_EMMC_DSBD    ((volatile uint32 *)(0x01C0F10C))
#define SD0_FIFO         ((volatile uint32 *)(0x01C0F200))

//----------------------------------------------------------------------------------------------------------------------------------

//Port F registers
#define PORT_F_CFG_REG   ((volatile uint32 *)(0x01C208B4))

//----------------------------------------------------------------------------------------------------------------------------------

//Manual states bit 0 while in the scope software bit 1 is used. In the sunxi_mmc implementation this register is not used
#define SD_HWRST_ACTIVE                  0x00000001


#define SD_GCTL_SOFT_RST                 0x00000001
#define SD_GCTL_FIFO_RST                 0x00000002
#define SD_GCTL_DMA_RST                  0x00000004

#define SD_GCTL_CD_DBC_ENB               0x00000100

#define SD_GCTL_FIFO_ACCESS_AHB          0x80000000



#define SD_CKCR_CCLK_ENB                 0x00010000
#define SD_CKCR_CCLK_CTRL                0x00020000
#define SD_CKCR_CCLK_CLR_DIV             0xFFFFFF00


#define SD_BWDR_1_BIT_WIDTH              0x00000000
#define SD_BWDR_4_BIT_WIDTH              0x00000001





#define SD_CMD_START                     0x80000000

#define SD_CMD_SEND_INIT_SEQ             0x00008000

#define SD_CMD_RESP_EXPIRE               0x00000040
#define SD_CMD_LONG_RESPONSE             0x00000080
#define SD_CMD_CHK_RESPONSE_CRC          0x00000100
#define SD_CMD_DATA_EXPIRE               0x00000200
#define SD_CMD_WRITE                     0x00000400

#define SD_CMD_AUTO_STOP                 0x00001000
#define SD_CMD_WAIT_PRE_OVER             0x00002000

#define SD_CMD_UPCLK_ONLY                0x00200000


#define SD_STATUS_CARD_DATA_BUSY         0x00000200

#define SD_STATUS_FIFO_EMPTY             0x00000004
#define SD_STATUS_FIFO_FULL              0x00000008



#define SD_RINT_RESP_ERROR               0x00000002
#define SD_RINT_COMMAND_DONE             0x00000004
#define SD_RINT_DATA_OVER                0x00000008
#define SD_RINT_TX_DATA_REQUEST          0x00000010
#define SD_RINT_RX_DATA_REQUEST          0x00000020
#define SD_RINT_RESP_CRC_ERROR           0x00000040
#define SD_RINT_DATA_CRC_ERROR           0x00000080
#define SD_RINT_RESP_TIMEOUT             0x00000100
#define SD_RINT_DATA_TIMEOUT             0x00000200
#define SD_RINT_VOLTAGE_CHANGE_DONE      0x00000400
#define SD_RINT_FIFO_RUN_ERROR           0x00000800
#define SD_RINT_HARDWARE_LOCKED          0x00001000
#define SD_RINT_START_BIT_ERROR          0x00002000
#define SD_RINT_AUTO_COMMAND_DONE        0x00004000
#define SD_RINT_END_BIT_ERROR            0x00008000
#define SD_RINT_SDIO_INTERRUPT           0x00010000
#define SD_RINT_CARD_INSERT              0x40000000
#define SD_RINT_CARD_REMOVE              0x80000000

#define SD_RINT_INTERRUPT_ERROR_BITS     (SD_RINT_END_BIT_ERROR | SD_RINT_START_BIT_ERROR | SD_RINT_HARDWARE_LOCKED | SD_RINT_FIFO_RUN_ERROR | SD_RINT_VOLTAGE_CHANGE_DONE | SD_RINT_DATA_TIMEOUT | SD_RINT_RESP_TIMEOUT | SD_RINT_DATA_CRC_ERROR | SD_RINT_RESP_CRC_ERROR | SD_RINT_RESP_ERROR)


#define SD_RESPONSE_NONE                 0x00000000
#define SD_RESPONSE_PRESENT              0x00000001
#define SD_RESPONSE_136                  0x00000002
#define SD_RESPONSE_CRC                  0x00000004
#define SD_RESPONSE_BUSY                 0x00000008



#define SD_DATA_READ                              1
#define SD_DATA_WRITE                             2

#define SD_CARD_TYPE_NONE                         0
#define SD_CARD_TYPE_SDHC                         1
#define SD_CARD_TYPE_SDSC                         2
#define SD_CARD_TYPE_MMC                          3


#define SD_NOT_CAPABLE                            1
#define SD_OK                                     0
#define SD_ERROR                                 -1
#define SD_ERROR_TIMEOUT                         -2
#define SD_ERROR_INVALID_BUFFER                  -3
#define SD_ERROR_SECTOR_OUT_OF_RANGE             -4

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct tagSD_CARD_COMMAND   SD_CARD_COMMAND, *PSD_CARD_COMMAND;
typedef struct tagSD_CARD_DATA      SD_CARD_DATA,    *PSD_CARD_DATA;

//----------------------------------------------------------------------------------------------------------------------------------

struct tagSD_CARD_COMMAND
{
  uint16 cmdidx;
  uint32 resp_type;
  uint32 cmdarg;
  uint32 response[4];
};

struct tagSD_CARD_DATA
{
  uint8  *data;
  uint32  flags;
  uint32  blocks;
  uint32  blocksize;
};

//----------------------------------------------------------------------------------------------------------------------------------

int32 sd_card_init(void);

int32 sd_card_read(uint32 sector, uint32 blocks, uint8 *buffer);

int32 sd_card_write(uint32 sector, uint32 blocks, uint8 *buffer);

int32 sd_card_get_specifications(void);

int32 sd_card_set_clock_and_bus(int32 usewidebus);

int32 sd_card_check_switchable_function(void);

int32 sd_card_clk_init(uint32 frequency);
int32 sd_card_change_clk(uint32 frequency);
int32 sd_card_update_clock(void);

int32 sd_card_send_command(PSD_CARD_COMMAND command, PSD_CARD_DATA data);

int32 sd_send_data(PSD_CARD_DATA data);

int32 sd_rint_wait(uint32 timeout, uint32 status_bit);

void sd_card_delay(uint32 delay);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* SD_CARD_INTERFACE_H */

