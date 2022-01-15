//----------------------------------------------------------------------------------------------------------------------------------

#ifndef DISPLAY_LIB_H
#define DISPLAY_LIB_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define DISPLAY_DRAW_CLOCK_WISE             0
#define DISPLAY_DRAW_COUNTER_CLOCK_WISE     1

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct tagDisplayData   DISPLAYDATA,  *PDISPLAYDATA;

//----------------------------------------------------------------------------------------------------------------------------------

struct tagDisplayData
{
  uint16     fg_color;
  uint16    *ygradient;            //Buffer for holding a y gradient. Needs full y dimension to work
  uint16    *screenbuffer;
  uint16    *linepointer;
  uint32     width;
  uint32     height;
  uint32     pixelsperline;
};

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_dimensions(uint32 width, uint32 height);
void display_set_fg_color(uint32 color);
void display_set_screen_buffer(uint16 *buffer);

void display_set_fg_y_gradient(uint16 *buffer, uint32 ystart, uint32 yend, uint32 startcolor, uint32 endcolor);

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_line(uint32 xstart, uint32 ystart, uint32 xend, uint32 yend);
void display_draw_horz_line(uint32 ypos, uint32 xstart, uint32 xend);
void display_draw_vert_line(uint32 xpos, uint32 ystart, uint32 yend);
void display_draw_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height);

//----------------------------------------------------------------------------------------------------------------------------------

void display_fill_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height);

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_icon_fg_color_y_gradient(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* DISPLAY_LIB_H */
