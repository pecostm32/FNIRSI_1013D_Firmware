#ifndef DRAM_CONTROL_H
#define DRAM_CONTROL_H

//Allwinner F1C100s DRAM controller registers
#define DRAM_SCONR        ((volatile unsigned int *)(0x01c01000))
#define DRAM_STMG0R       ((volatile unsigned int *)(0x01c01004))
#define DRAM_STMG1R       ((volatile unsigned int *)(0x01c01008))
#define DRAM_SCTLR        ((volatile unsigned int *)(0x01c0100C))
#define DRAM_SREFR        ((volatile unsigned int *)(0x01c01010))
#define DRAM_SEXTMR       ((volatile unsigned int *)(0x01c01014))
#define DRAM_DDLYR        ((volatile unsigned int *)(0x01c01024))
#define DRAM_DADRR        ((volatile unsigned int *)(0x01c01028))
#define DRAM_DVALR        ((volatile unsigned int *)(0x01c0102C))
#define DRAM_DRPTR0       ((volatile unsigned int *)(0x01c01030))
#define DRAM_DRPTR1       ((volatile unsigned int *)(0x01c01034))
#define DRAM_DRPTR2       ((volatile unsigned int *)(0x01c01038))
#define DRAM_DRPTR3       ((volatile unsigned int *)(0x01c0103C))
#define DRAM_SEFR         ((volatile unsigned int *)(0x01c01040))
#define DRAM_MAE          ((volatile unsigned int *)(0x01c01044))
#define DRAM_ASPR         ((volatile unsigned int *)(0x01c01048))
#define DRAM_SDLY0        ((volatile unsigned int *)(0x01c0104C))
#define DRAM_SDLY1        ((volatile unsigned int *)(0x01c01050))
#define DRAM_SDLY2        ((volatile unsigned int *)(0x01c01054))
#define DRAM_MCR0         ((volatile unsigned int *)(0x01c01100))
#define DRAM_MCR1         ((volatile unsigned int *)(0x01c01104))
#define DRAM_MCR2         ((volatile unsigned int *)(0x01c01108))
#define DRAM_MCR3         ((volatile unsigned int *)(0x01c0110C))
#define DRAM_MCR4         ((volatile unsigned int *)(0x01c01110))
#define DRAM_MCR5         ((volatile unsigned int *)(0x01c01114))
#define DRAM_MCR6         ((volatile unsigned int *)(0x01c01118))
#define DRAM_MCR7         ((volatile unsigned int *)(0x01c0111C))
#define DRAM_MCR8         ((volatile unsigned int *)(0x01c01120))
#define DRAM_MCR9         ((volatile unsigned int *)(0x01c01124))
#define DRAM_MCR10        ((volatile unsigned int *)(0x01c01128))
#define DRAM_MCR11        ((volatile unsigned int *)(0x01c0112C))
#define DRAM_BWCR         ((volatile unsigned int *)(0x01c01140))

//Settings for the registers
#define PLL_DDR_CLK      156000000

#define SDR_T_CAS             0x02
#define SDR_T_RAS             0x08
#define SDR_T_RCD             0x03
#define SDR_T_RP              0x03
#define SDR_T_WR              0x03
#define SDR_T_RFC             0x0D
#define SDR_T_XSR             0xF9
#define SDR_T_RC              0x0B
#define SDR_T_INIT            0x08
#define SDR_T_INIT_REF        0x07
#define SDR_T_WTR             0x02
#define SDR_T_RRD             0x02
#define SDR_T_XP              0x00

//SDRAM pad pull register settings
#define ENABLE_INTERNAL_REF          0x00800000
#define REF_CONF_FACTOR_32           0x00400000


//Types of memory
typedef enum
{
  DRAM_TYPE_SDR = 0,
  DRAM_TYPE_DDR = 1,
  DRAM_TYPE_MDDR = 2,
} dram_type;

typedef struct
{
  unsigned int base;              //dram base address
  unsigned int size;              //dram size (unit: MByte)
  unsigned int clk;               //dram work clock (unit: MHz)
  unsigned int access_mode;       //0: interleave mode 1: sequence mode
  unsigned int cs_num;            //dram chip count  1: one chip  2: two chip
  unsigned int ddr8_remap;        //for 8 bits data width DDR 0: normal  1: 8 bits
  dram_type    sdr_ddr;           //Type of DDR in use
  unsigned int bwidth;            //dram bus width
  unsigned int col_width;         //column address width
  unsigned int row_width;         //row address width
  unsigned int bank_size;         //dram bank count
  unsigned int cas;               //dram cas
} dram_parameters;


//Function to call from main program for initializing the memory
void sys_dram_init(void);

//Support functions for the init function
int dram_init(dram_parameters *params);
int dram_params_setup(dram_parameters *params);
int dram_initial(void);
int dram_delay_scan(void);
unsigned int dram_check_type(dram_parameters *params);
void dram_set_autofresh_cycle(unsigned int clk);
unsigned int dram_scan_readpipe(dram_parameters *params);
unsigned int dram_get_dram_size(dram_parameters *params);
unsigned int dram_check_delay(unsigned int bwidth);

int sdr_readpipe_scan(void);
unsigned int sdr_readpipe_select(void);

void dram_delay(unsigned int ms);

#endif //DRAM_CONTROL_H

