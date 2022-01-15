//--------------------------------------------------------------------------------------

#ifndef SPI_CONTROL_H
#define SPI_CONTROL_H

//--------------------------------------------------------------------------------------

#define SPI0_GCR         ((volatile unsigned int *)(0x01C05004))
#define SPI0_TCR         ((volatile unsigned int *)(0x01C05008))
#define SPI0_IER         ((volatile unsigned int *)(0x01C05010))
#define SPI0_ISR         ((volatile unsigned int *)(0x01C05014))
#define SPI0_FCR         ((volatile unsigned int *)(0x01C05018))
#define SPI0_FSR         ((volatile unsigned int *)(0x01C0501C))
#define SPI0_WCR         ((volatile unsigned int *)(0x01C05020))
#define SPI0_CCR         ((volatile unsigned int *)(0x01C05024))
#define SPI0_MBC         ((volatile unsigned int *)(0x01C05030))
#define SPI0_MTC         ((volatile unsigned int *)(0x01C05034))
#define SPI0_BCC         ((volatile unsigned int *)(0x01C05038))

#define SPI0_TXD_INT     ((volatile unsigned int *)(0x01C05200))
#define SPI0_RXD_INT     ((volatile unsigned int *)(0x01C05300))

#define SPI0_TXD_SHORT   ((volatile unsigned short *)(0x01C05200))
#define SPI0_RXD_SHORT   ((volatile unsigned short *)(0x01C05300))

#define SPI0_TXD_BYTE    ((volatile unsigned char *)(0x01C05200))
#define SPI0_RXD_BYTE    ((volatile unsigned char *)(0x01C05300))

//--------------------------------------------------------------------------------------
//Global control settings
#define SPI_GCR_SRST                0x80000000       //Soft reset. Self clearing
#define SPI_GCR_TP_EN               0x00000080       //Transmit pause enable
#define SPI_GCR_MODE_MASTER         0x00000002       //Enable master mode
#define SPI_GCR_MODE_EN             0x00000001       //Enable SPI controller

//--------------------------------------------------------------------------------------
//Transfer control settings
#define SPI_TCR_XCH_START           0x80000000       //Exchange burst start
#define SPI_TCR_SDM_NORMAL          0x00002000       //Set master sample data mode to normal
#define SPI_TCR_FBS_LSB             0x00001000       //Set first bit transmit to LSB
#define SPI_TCR_SDC_DELAY           0x00000800       //Set master sample data control to delay
#define SPI_TCR_RPSM_RAPID          0x00000400       //Set rapids mode to rapids write
#define SPI_TCR_DDB_ONE             0x00000200       //Set dummy burst type to bit value one
#define SPI_TCR_DHB_DISCARD         0x00000100       //Set discard hash burst to discarddummy burst type to bit value one
#define SPI_TCR_SS_LEVEL_HIGH       0x00000080       //Set ss level to high
#define SPI_TCR_SS_OWNER_SOFT       0x00000040       //Set ss owner to software
#define SPI_TCR_SS_SEL_SS0          0x00000000       //Set ss chip select line 0
#define SPI_TCR_SS_SEL_SS1          0x00000010       //Set ss chip select line 1
#define SPI_TCR_SS_SEL_SS2          0x00000020       //Set ss chip select line 2
#define SPI_TCR_SS_SEL_SS3          0x00000030       //Set ss chip select line 3
#define SPI_TCR_SS_CTL_NEGATE       0x00000008       //Set ss control to negate
#define SPI_TCR_SPOL_ACTIVE_LOW     0x00000004       //Set spol to active low
#define SPI_TCR_CPOL_ACTIVE_LOW     0x00000002       //Set spol to active low
#define SPI_TCR_CPHA_PHASE_1        0x00000001       //Set clock/data phase to leading edge for setup data 

//--------------------------------------------------------------------------------------
//FIFO control settings
#define SPI_FCR_TX_FIFO_RST         0x80000000       //Transmit FIFO reset. Self clearing
#define SPI_FCR_TX_TRIG_LEV_64      0x00400000       //Trigger level for transmit FIFO

#define SPI_FCR_RX_FIFO_RST         0x00008000       //Receive FIFO reset. Self clearing
#define SPI_FCR_RX_TRIG_LEV_1       0x00000001       //Trigger level for receive FIFO

//--------------------------------------------------------------------------------------
//Clock control settings
#define SPI_CCR_DRS_DIV_2           0x00001000       //Divide rate select. Clock divide rate 2

//SPI frequency is based on AHB_CLK.
//When CDR1 is used it is AHB_CLK / 2^(N + 1)
//Divide factor CDR1 (0 -- 15)
#define SPI_CCR_CDR1(x)             ((x & 0xF) << 8)

//When CDR2 is used it is AHB_CLK / 2*(N + 1)
//Divide factor CDR1 (0 -- 15)
#define SPI_CCR_CDR2(x)            (x & 0xF)

//--------------------------------------------------------------------------------------

#define FLASH_SECTOR_ADDR_MASK    0x00FFF000

#define FLASH_SECTOR_MASK         0x00000FFF
#define FLASH_SECTOR_SIZE               4096

#define FLASH_PAGE_MASK           0x000000FF
#define FLASH_PAGE_SIZE                  256

//--------------------------------------------------------------------------------------
//Functions
void sys_spi_flash_init(void);
void sys_spi_flash_exit(void);
void sys_spi_flash_read(int addr, unsigned char *buffer, int length);
void sys_spi_flash_write(int addr, unsigned char *buffer, int length);

void sys_spi_flash_write_enable(void);
void sys_spi_flash_wait_while_busy(void);
void sys_spi_flash_erase_sector(int addr);
void sys_spi_flash_program_page(int addr, unsigned char *buffer, int length);
void sys_spi_flash_program_sector(int addr, unsigned char *buffer, int length);

//--------------------------------------------------------------------------------------
//Support functions
void sys_spi_write(unsigned char *buffer, int length);
void sys_spi_read(unsigned char *buffer, int length);

//--------------------------------------------------------------------------------------

#endif /* SPI_CONTROL_H */

