//----------------------------------------------------------------------------------------------------------------------------------

#ifndef DISPLAY_LIB_H
#define DISPLAY_LIB_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "font_structs.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define DISPLAY_DRAW_CLOCK_WISE             0
#define DISPLAY_DRAW_COUNTER_CLOCK_WISE     1

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct tagDisplayData   DISPLAYDATA,  *PDISPLAYDATA;

//----------------------------------------------------------------------------------------------------------------------------------

struct tagDisplayData
{
  PFONTDATA  font;
  uint16     fg_color;
  uint16     bg_color;
  uint16    *ygradient;            //Buffer for holding a y gradient. Needs full y dimension to work
  uint16    *screenbuffer;
  uint16    *sourcebuffer;         //For copy to screen or slide function the source from where to get the data from
  uint16    *destbuffer;           //For copy from screen the destination where to put the data
  uint16    *savebuffer;
  uint16    *linepointer;
  uint32     xpos;
  uint32     ypos;
  uint32     width;
  uint32     height;
  uint32     pixelsperline;
};

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_position(uint32 xpos, uint32 ypos);
void display_set_dimensions(uint32 width, uint32 height);
void display_set_font(PFONTDATA font);
void display_set_fg_color(uint32 color);
void display_set_bg_color(uint32 color);
void display_set_screen_buffer(uint16 *buffer);
void display_set_source_buffer(uint16 *buffer);
void display_set_destination_buffer(uint16 *buffer);

void display_save_screen_buffer(void);
void display_restore_screen_buffer(void);

void display_set_fg_y_gradient(uint16 *buffer, uint32 ystart, uint32 yend, uint32 startcolor, uint32 endcolor);

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_line(uint32 xstart, uint32 ystart, uint32 xend, uint32 yend);
void display_draw_horz_line(uint32 ypos, uint32 xstart, uint32 xend);
void display_draw_vert_line(uint32 xpos, uint32 ystart, uint32 yend);
void display_draw_horz_dots(uint32 ypos, uint32 xstart, uint32 xend, uint32 interval);
void display_draw_vert_dots(uint32 xpos, uint32 ystart, uint32 yend, uint32 interval);
void display_draw_horz_dashes(uint32 ypos, uint32 xstart, uint32 xend, uint32 length, uint32 interval);
void display_draw_vert_dashes(uint32 xpos, uint32 ystart, uint32 yend, uint32 length, uint32 interval);
void display_draw_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height);
void display_draw_rounded_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 radius);
void display_draw_arc(uint32 xpos, uint32 ypos, uint32 radius, uint32 startangle, uint32 endangle, uint32 direction);

//----------------------------------------------------------------------------------------------------------------------------------

void display_fill_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height);
void display_fill_rounded_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 radius);

//----------------------------------------------------------------------------------------------------------------------------------

void display_slide_top_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed);
void display_slide_left_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed);
void display_slide_right_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed);

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_rect_from_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height);
void display_copy_rect_to_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height);

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_icon_use_colors(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height);
void display_copy_icon_fg_color(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height);
void display_copy_icon_fg_color_y_gradient(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height);

//----------------------------------------------------------------------------------------------------------------------------------

void display_left_pointer(uint32 xpos, uint32 ypos, int8 id);
void display_right_pointer(uint32 xpos, uint32 ypos, int8 id);
void display_top_pointer(uint32 xpos, uint32 ypos, int8 id);

//----------------------------------------------------------------------------------------------------------------------------------

void display_hex(uint32 xpos, uint32 ypos, uint32 digits, int32 value);
void display_decimal(uint32 xpos, uint32 ypos, int32 value);
void display_character(uint32 xpos, uint32 ypos, int8 text);
void display_text(uint32 xpos, uint32 ypos, int8 *text);

//----------------------------------------------------------------------------------------------------------------------------------
//Variable width font handling functions

void draw_vw_character(uint16 character);

PFONTINFORMATION check_char_in_vw_font(PFONTINFORMATION info, uint16 character);

//----------------------------------------------------------------------------------------------------------------------------------
//Fixed width font handling functions

void draw_fw_character(uint16 character);
void render_fw_character(uint16 character);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* DISPLAY_LIB_H */
