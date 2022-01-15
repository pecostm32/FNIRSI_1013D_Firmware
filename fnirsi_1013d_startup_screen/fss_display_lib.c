//----------------------------------------------------------------------------------------------------------------------------------

//Need some screen buffer system where it is possible to build up an image without disturbing the actual screen

//So function for copying a full screen
//A function for copying a partial screen
//A function for animating a menu

//Needed functions
//display_fill_arc            //Two possible modes: fill a pie section (lines from origin to the angles) or a slice section (straight line from start angle to end angle)

//Need to think about line widths or dot size


//----------------------------------------------------------------------------------------------------------------------------------

#include "fss_display_lib.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

DISPLAYDATA displaydata;

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_dimensions(uint32 width, uint32 height)
{
  //Adjust for zero being part of the display
  displaydata.width  = width - 1;
  displaydata.height = height - 1;
  displaydata.pixelsperline = width;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_fg_color(uint32 color)
{
  displaydata.fg_color = (color & 0x00F80000) >> 8 | (color & 0x0000FC00) >> 5 | (color & 0x000000F8) >> 3;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_screen_buffer(uint16 *buffer)
{
  displaydata.screenbuffer = buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_fg_y_gradient(uint16 *buffer, uint32 ystart, uint32 yend, uint32 startcolor, uint32 endcolor)
{
  uint32 y,ys,ye;
  int32  rs,re,gs,ge,bs,be;
  int32  rd,gd,bd,yd;
  
  //Set the buffer pointer for the gradient
  displaydata.ygradient = buffer;

  //Determine the lowest x for start point
  if(ystart < yend)
  {
    //Use the coordinates as is
    ys = ystart;
    ye = yend;
  }
  else
  {
    //Swap start and end
    ys = yend;
    ye = ystart;
  }
  
  //Make sure yend is in range of the screen
  if(ye > displaydata.height)
  {
    ye = displaydata.height;
  }
  
  //Calculate the y delta
  yd = ye - ys;
  
  //Get individual color bytes in the msb minus one bit
  rs = (startcolor <<  7) & 0x7F800000;
  re = (endcolor   <<  7) & 0x7F800000;
  gs = (startcolor << 15) & 0x7F800000;
  ge = (endcolor   << 15) & 0x7F800000;
  bs = (startcolor << 23) & 0x7F800000;
  be = (endcolor   << 23) & 0x7F800000;
  
  //Calculate the integer color steps. Can be negative.
  rd = (re - rs) / yd;
  gd = (ge - gs) / yd;
  bd = (be - bs) / yd;
  
  //Process the gradient in a loop and set a color for each entry in range of ystart and yend
  for(y=ys;y<=ye;y++)
  {
    //Set the current color
    buffer[y] = ((rs & 0x7C000000) >> 15) | ((gs & 0x7C000000) >> 20) | ((bs & 0x7C000000) >> 26);
    
    //Calculate the next color elements
    rs += rd;
    gs += gd;
    bs += bd;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_line(uint32 xstart, uint32 ystart, uint32 xend, uint32 yend)
{
  register uint16 *ptr;
  register int32  x, xs, xe, y, ys, ye, dx;
  register int32  yacc;
  register int32  ystep;
  
  //Determine the lowest x for start point
  if(xstart < xend)
  {
    //Use the coordinates as is
    xs = xstart;
    xe = xend;
    ys = ystart;
    ye = yend;
  }
  else
  {
    //Swap start and end
    xs = xend;
    xe = xstart;
    ys = yend;
    ye = ystart;
  }
  
  //Check if the line is vertical
  if(xstart == xend)
  {
    //If so use 1 for delta x to calculate the y segment length
    dx = 1;
  }
  else
  {
    //Not vertical then calculate delta x.
    dx = (xe - xs) + 1;
  }
  
  //Calculate the y segment length
  ystep = ((ye - ys) << 16) / dx;
  
  //Initialize the y accumulator for broken pixel accounting
  yacc = ys << 16;
  
  //Handle all the x positions
  for(x=xs;x<=xe;x++)
  {
    //Calculate the y end of this segment
    yacc += ystep;
    ye = yacc >> 16;

    //Check if line is going down or up    
    if(ystep >= 0)
    {
      //Going down so add to y
      for(y=ys;y<=ye;y++)
      {
        //Check on screen bounds
        if((x >= 0) && (x < displaydata.width) && (y >= 0) && (y < displaydata.height))
        {
          //Point to the pixel in the screen buffer
          ptr = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + x);

          //Fill the dot
          *ptr = displaydata.fg_color;
        }
      }
    }
    else
    {
      //Going up so subtract from y
      for(y=ys;y>=ye;y--)
      {
        //Check on screen bounds
        if((x >= 0) && (x < displaydata.width) && (y >= 0) && (y < displaydata.height))
        {
          //Point to the pixel in the screen buffer
          ptr = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + x);

          //Fill the dot
          *ptr = displaydata.fg_color;
        }
      }
    }
    
    //Set y start of next segment
    ys = ye;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_horz_line(uint32 ypos, uint32 xstart, uint32 xend)
{
  register uint16 *ptr;
  register uint32 x, xs, xe;
  
  //Check if the line is on the screen
  if(ypos > displaydata.height)
  {
    //Outside the y range so do nothing
    return;
  }
    
  //Determine the lowest x for start point
  if(xstart < xend)
  {
    //Use the coordinates as is
    xs = xstart;
    xe = xend;
  }
  else
  {
    //Swap start and end
    xs = xend;
    xe = xstart;
  }
  
  //Check if the end of the line is on the screen
  if(xe > displaydata.width)
  {
    //Outside so limit to the end of the screen
    xe = displaydata.width;
  }

  //Point to where the line needs to be drawn
  ptr = displaydata.screenbuffer + ((ypos * displaydata.pixelsperline) + xs);
  
  //Draw the dots
  for(x=xs;x<=xe;x++)
  {
    //Fill the dot
    *ptr++ = displaydata.fg_color;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_vert_line(uint32 xpos, uint32 ystart, uint32 yend)
{
  register uint16 *ptr;
  register uint32 y, ys, ye;
  register uint32 pixels = displaydata.pixelsperline;
  
  //Check if the line is on the screen
  if(xpos > displaydata.width)
  {
    //Outside the x range so do nothing
    return;
  }
    
  //Determine the lowest y for start point
  if(ystart < yend)
  {
    //Use the coordinates as is
    ys = ystart;
    ye = yend;
  }
  else
  {
    //Swap start and end
    ys = yend;
    ye = ystart;
  }
  
  //Check if the end of the line is on the screen
  if(ye > displaydata.height)
  {
    //Outside so limit to the end of the screen
    ye = displaydata.height;
  }

  //Point to where the line needs to be drawn
  ptr = displaydata.screenbuffer + ((ys * pixels) + xpos);
  
  //Draw the dots
  for(y=ys;y<=ye;y++)
  {
    //Fill the dot
    *ptr = displaydata.fg_color;
    
    //Point to the next dot
    ptr += pixels;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height)
{
  //Compensate for the last pixel
  width--;
  height--;
  
  uint32 xe = xpos + width;
  uint32 ye = ypos + height;
  
  //Just draw the needed lines
  display_draw_horz_line(ypos, xpos, xe);
  display_draw_horz_line(ye, xpos, xe);
  display_draw_vert_line(xpos, ypos, ye);
  display_draw_vert_line(xe, ypos, ye);
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_fill_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height)
{
  uint16 *ptr;
  uint32  y;
  uint32  x;

  //Calculate the last x and y position to compare against
  width += xpos;
  height += ypos;

  //Check on x bound
  if(width > displaydata.width)
  {
    width = displaydata.width;
  }
  
  //Check on y bound
  if(height > displaydata.height)
  {
    height = displaydata.height;
  }
  
  //Draw all the pixels
  for(y=ypos;y<height;y++)
  {
    //Point to the first pixel of this line in the screen buffer
    ptr = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + xpos);

    //Draw the pixels on the line
    for(x=xpos;x<width;x++)
    {
      //Set the current screen buffer pixel with the requested color
      *ptr++ = displaydata.fg_color;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_icon_fg_color_y_gradient(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height)
{
  register uint16 *ptr;
  register uint32  line;
  register uint32  pixel;
  register uint32  idx;
  register uint32  pixeldata;
  register uint32  bytesperrow = (width + 7) / 8;
  register uint32  pixels = displaydata.pixelsperline;
  
  //Setup destination pointer
  ptr = displaydata.screenbuffer + xpos + (ypos * pixels);
  
  //Copy the needed lines
  for(line=0;line<height;line++)
  {
    //Point the icon start byte for this line
    idx = line * bytesperrow;
    
    //Get the data for per bit handling
    pixeldata = icon[idx];
    
    //Copy a single line to the destination buffer
    for(pixel=0;pixel<width;)
    {
      //Select the pixel to check
      pixeldata <<= 1;
      
      //Copy one pixel at a time with a check on being on
      if(pixeldata & 0x0100)
      {
        //When on use the foreground gradient
        ptr[pixel] = displaydata.ygradient[ypos+line];
      }
      
      //Select next pixel
      pixel++;
      
      //Check if pixel on multiple of 8 for next byte select
      if((pixel & 0x07) == 0)
      {
        //Point to the next byte
        idx++;
    
        //And get the data for it
        pixeldata = icon[idx];
      }
    }

    //Point to the next line of pixels in the destination
    ptr += pixels;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
