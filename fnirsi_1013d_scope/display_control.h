//--------------------------------------------------------------------------------------

#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H

//--------------------------------------------------------------------------------------
//LCD timing control registers
#define TCON_CTRL              ((volatile uint32 *)(0x01C0C000))
#define TCON_INT0              ((volatile uint32 *)(0x01C0C004))
#define TCON_INT1              ((volatile uint32 *)(0x01C0C008))


#define TCON0_CTRL             ((volatile uint32 *)(0x01C0C040))
#define TCON_CLK_CTRL          ((volatile uint32 *)(0x01C0C044))
#define TCON0_BASIC_TIMING0    ((volatile uint32 *)(0x01C0C048))
#define TCON0_BASIC_TIMING1    ((volatile uint32 *)(0x01C0C04C))
#define TCON0_BASIC_TIMING2    ((volatile uint32 *)(0x01C0C050))
#define TCON0_BASIC_TIMING3    ((volatile uint32 *)(0x01C0C054))
#define TCON0_HV_TIMING        ((volatile uint32 *)(0x01C0C058))
#define TCON0_CPU_IF           ((volatile uint32 *)(0x01C0C060))


#define TCON0_IO_CTRL0         ((volatile uint32 *)(0x01C0C088))
#define TCON0_IO_CTRL1         ((volatile uint32 *)(0x01C0C08C))


#define TCON1_IO_CTRL1         ((volatile uint32 *)(0x01C0C0F4))

//--------------------------------------------------------------------------------------
//Display engine back end registers
#define DEBE_MODE_CTRL         ((volatile uint32 *)(0x01E60800))

#define DEBE_LAY_SIZE          ((volatile uint32 *)(0x01E60808))    //Not present in the manual but used in the scope software and set to the same value as DEBE_LAY0_SIZE

#define DEBE_LAY0_SIZE         ((volatile uint32 *)(0x01E60810))
#define DEBE_LAY1_SIZE         ((volatile uint32 *)(0x01E60814))
#define DEBE_LAY2_SIZE         ((volatile uint32 *)(0x01E60818))
#define DEBE_LAY3_SIZE         ((volatile uint32 *)(0x01E6081C))

#define DEBE_LAY0_LINEWIDTH    ((volatile uint32 *)(0x01E60840))
#define DEBE_LAY1_LINEWIDTH    ((volatile uint32 *)(0x01E60844))
#define DEBE_LAY2_LINEWIDTH    ((volatile uint32 *)(0x01E60848))
#define DEBE_LAY3_LINEWIDTH    ((volatile uint32 *)(0x01E6084C))

#define DEBE_LAY0_FB_ADDR1L    ((volatile uint32 *)(0x01E60850))
#define DEBE_LAY0_FB_ADDR2L    ((volatile uint32 *)(0x01E60854))
#define DEBE_LAY0_FB_ADDR3L    ((volatile uint32 *)(0x01E60858))
#define DEBE_LAY0_FB_ADDR4L    ((volatile uint32 *)(0x01E6085C))

#define DEBE_LAY0_FB_ADDR1H    ((volatile uint32 *)(0x01E60860))    //Not present in the manual but used in the scope to set the top 3 bits of the frame buffer address
#define DEBE_LAY0_FB_ADDR2H    ((volatile uint32 *)(0x01E60864))
#define DEBE_LAY0_FB_ADDR3H    ((volatile uint32 *)(0x01E60868))
#define DEBE_LAY0_FB_ADDR4H    ((volatile uint32 *)(0x01E6086C))


#define DEBE_REGBUFF_CTRL      ((volatile uint32 *)(0x01E60870))

#define DEBE_LAY0_ATT_CTRL1    ((volatile uint32 *)(0x01E608A0))

#define DEBE_COEF23            ((volatile uint32 *)(0x01E6097C))

//--------------------------------------------------------------------------------------
//LCD timing control settings
#define TCON_CTRL_MODULE_EN                 0x80000000         //Enable the LCD timing control module
#define TCON_CTRL_IO_MAP_SEL_TCON1          0x00000001         //Use the TCON1 registers instead of the TCON0 registers



//--------------------------------------------------------------------------------------
//Display engine back end settings
#define DEBE_MODE_CTRL_EN                   0x00000001         //Enable the display engine back end
#define DEBE_MODE_CTRL_START                0x00000002         //Start the display engine back end

#define DEBE_MODE_CTRL_LAYER0_EN            0x00000100         //Enable layer0



#define DEBE_REGBUFF_CTRL_LOAD              0x00000001         //Load the module registers



#define DEBE_LAY0_ATT_CTRL1_RGB565          0x00000500         //16 bits per pixel RGB 565 mode


#define DEBE_NUMBER_OF_BYTES                384

//--------------------------------------------------------------------------------------
//Functions

void sys_disable_display(void);

void sys_init_display(uint16 xsize, uint16 ysize, uint16 *address);

void display_bitmap(uint16 xpos, uint16 ypos, uint16 xsize, uint16 ysize, uint16 *source, uint16 *dest);

//--------------------------------------------------------------------------------------

#endif /* DISPLAY_CONTROL_H */

