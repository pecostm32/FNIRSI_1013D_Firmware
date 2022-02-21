//----------------------------------------------------------------------------------------------------------------------------------

//Need some screen buffer system where it is possible to build up an image without disturbing the actual screen

//So function for copying a full screen
//A function for copying a partial screen
//A function for animating a menu

//Needed functions
//display_fill_arc            //Two possible modes: fill a pie section (lines from origin to the angles) or a slice section (straight line from start angle to end angle)

//Need to think about line widths or dot size


//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "font_structs.h"
#include "display_lib.h"
#include "sin_cos_math.h"

#include <string.h>


#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------------------------------------------------------------------

DISPLAYDATA displaydata;

//----------------------------------------------------------------------------------------------------------------------------------

extern const uint8 left_pointer_icon[];
extern const uint8 right_pointer_icon[];
extern const uint8 top_pointer_icon[];

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_position(uint32 xpos, uint32 ypos)
{
  displaydata.xpos = xpos;
  displaydata.ypos = ypos;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_dimensions(uint32 width, uint32 height)
{
  //Adjust for zero being part of the display
  displaydata.width  = width - 1;
  displaydata.height = height - 1;
  displaydata.pixelsperline = width;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_font(PFONTDATA font)
{
  displaydata.font = font;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_fg_color(uint32 color)
{
  displaydata.fg_color = (color & 0x00F80000) >> 8 | (color & 0x0000FC00) >> 5 | (color & 0x000000F8) >> 3;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_bg_color(uint32 color)
{
  displaydata.bg_color = (color & 0x00F80000) >> 8 | (color & 0x0000FC00) >> 5 | (color & 0x000000F8) >> 3;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_screen_buffer(uint16 *buffer)
{
  displaydata.screenbuffer = buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_source_buffer(uint16 *buffer)
{
  displaydata.sourcebuffer = buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_set_destination_buffer(uint16 *buffer)
{
  displaydata.destbuffer = buffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_save_screen_buffer(void)
{
  displaydata.savebuffer = displaydata.screenbuffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_restore_screen_buffer(void)
{
  displaydata.screenbuffer = displaydata.savebuffer;
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

void display_draw_horz_dots(uint32 ypos, uint32 xstart, uint32 xend, uint32 interval)
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

  //Point to where the dots need to be drawn
  ptr = displaydata.screenbuffer + ((ypos * displaydata.pixelsperline) + xs);
  
  //Draw the dots
  for(x=xs;x<=xe;x+=interval)
  {
    //Fill the dot
    *ptr = displaydata.fg_color;
    
    ptr += interval;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_vert_dots(uint32 xpos, uint32 ystart, uint32 yend, uint32 interval)
{
  register uint16 *ptr;
  register uint32 y, ys, ye;
  register uint32 pixels;
  
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

  //Calculate the dot interval
  pixels = displaydata.pixelsperline * interval;
  
  //Point to where the dots need to be drawn
  ptr = displaydata.screenbuffer + ((ys * displaydata.pixelsperline) + xpos);
  
  //Draw the dots
  for(y=ys;y<=ye;y+=interval)
  {
    //Fill the dot
    *ptr = displaydata.fg_color;
    
    //Point to the next dot
    ptr += pixels;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_horz_dashes(uint32 ypos, uint32 xstart, uint32 xend, uint32 length, uint32 interval)
{
  register uint16 *ptr;
  register uint32 x, xs, xe, cnt;
  
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

  //Point to where the dots need to be drawn
  ptr = displaydata.screenbuffer + ((ypos * displaydata.pixelsperline) + xs);
  
  //Draw the dashes
  for(cnt=0,x=xs;x<=xe;x++)
  {
    //Fill the dot
    *ptr++ = displaydata.fg_color;
    
    //Check if dash length number of pixels done
    if(cnt++ == length)
    {
      //If so reset and skip the interval
      cnt = 0;
      x += interval;
      ptr += interval;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_vert_dashes(uint32 xpos, uint32 ystart, uint32 yend, uint32 length, uint32 interval)
{
  register uint16 *ptr;
  register uint32 y, ys, ye, cnt;
  register uint32 pixels1;
  register uint32 pixels2;
  
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

  //Calculate the dash interval
  pixels1 = displaydata.pixelsperline;
  pixels2 = displaydata.pixelsperline * interval;
  
  //Point to where the dots need to be drawn
  ptr = displaydata.screenbuffer + ((ys * displaydata.pixelsperline) + xpos);
  
  //Draw the dots
  for(cnt=0,y=ys;y<=ye;y++)
  {
    //Fill the dot
    *ptr = displaydata.fg_color;
    
    //Point to the next dot
    ptr += pixels1;
    
    //Check if dash length number of pixels done
    if(cnt++ == length)
    {
      //If so reset and skip the interval
      cnt = 0;
      y += interval;
      ptr += pixels2;
    }
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

void display_draw_rounded_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 radius)
{
  //Compensate for the last pixel
  width--;
  height--;
  
  uint32 xs = xpos + radius;
  uint32 ys = ypos + radius;
  uint32 xe = xpos + width - radius;
  uint32 ye = ypos + height - radius;
  
  //Just draw the needed lines
  display_draw_horz_line(ypos, xs, xe);
  display_draw_horz_line(ypos + height, xs, xe);
  display_draw_vert_line(xpos, ys, ye);
  display_draw_vert_line(xpos + width, ys, ye);
  
  //And the needed arcs
  display_draw_arc(xs, ys, radius, 1800, 2700, 0);
  display_draw_arc(xe, ys, radius, 2700,    0, 0);
  display_draw_arc(xe, ye, radius,    0,  900, 0);
  display_draw_arc(xs, ye, radius,  900, 1800, 0);
}

//----------------------------------------------------------------------------------------------------------------------------------

const uint32 angles[4][2] = { { 0, 900 }, { 900, 1800 }, { 1800, 2700 }, { 2700, 3600 } };

//----------------------------------------------------------------------------------------------------------------------------------

void display_draw_arc(uint32 xpos, uint32 ypos, uint32 radius, uint32 startangle, uint32 endangle, uint32 direction)
{
  uint32  startquadrant = (startangle / 900) % 4;
  uint32  endquadrant   = (endangle / 900) % 4;
  uint32  quadrants[4]  = { 0, 0, 0, 0 };
  uint32  quadrant = startquadrant;
  uint16 *ptr;
  uint32   x, y;
  uint32  sa;
  uint32  ea;
  uint32  a, step;

  //Determine the angles step fit for the given radius  
  if(radius > 450)
    step = 1;
  else
    step = 450 / radius;
  
  //Flag the quadrants that need to be drawn
  while(quadrant != endquadrant)
  {
    quadrants[quadrant] = 1;
    
    if(direction == DISPLAY_DRAW_CLOCK_WISE)
      quadrant = (quadrant + 1) % 4;
    else
      quadrant = (quadrant - 1) % 4;
  }
  
  //Flag the end quadrant as well
  quadrants[endquadrant] = 1;
  
  //Draw the pixels for each quadrant
  for(quadrant=0;quadrant<4;quadrant++)
  {
    //Check if anything needs to be drawn in this quadrant
    if(quadrants[quadrant])
    {
      //Determine the angles to use based on the quadrant
      if(quadrant == startquadrant)
      {
        //Check on drawing direction
        if(direction == DISPLAY_DRAW_CLOCK_WISE)
        {
          //For clock wise start is from the given angle to the end of the quadrant
          sa = startangle % 3600;
          ea = angles[quadrant][1];
        }
        else
        {
          //For counter clock wise start is from the start of the quadrant to the given start angle
          sa = angles[quadrant][0];
          ea = startangle % 3600;
        }
      }
      else if(quadrant == endquadrant)
      {
        //Check on drawing direction
        if(direction == DISPLAY_DRAW_CLOCK_WISE)
        {
          //For clock wise start is from the starting angle of the quadrant to the given end angle
          sa = angles[quadrant][0];
          ea = endangle % 3600;
        }
        else
        {
          //For counter clock wise start is from the given end angle to the end of the quadrant
          sa = endangle % 3600;
          ea = angles[quadrant][1];
        }
      }
      else
      {
        //For fully drawn quadrant it is always from start to end of the quadrant
        sa = angles[quadrant][0];      
        ea = angles[quadrant][1];
      }

      //Draw the pixels
      for(a=sa;a<ea;)
      {
        //Get the coordinates for the current angle
        x = getxpos(a, xpos, radius);
        y = getypos(a, ypos, radius);

        //Check on screen bounds
        if((x >= 0) && (x <= displaydata.width) && (y >= 0) && (y <= displaydata.height))
        {
          //Point to the pixel in the screen buffer
          ptr = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + x);

          //Fill the dot
          *ptr = displaydata.fg_color;
        }

        //Step to the next angle
        a += step;
      }
    }
  }
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
  for(y=ypos;y<=height;y++)
  {
    //Point to the first pixel of this line in the screen buffer
    ptr = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + xpos);

    //Draw the pixels on the line
    for(x=xpos;x<=width;x++)
    {
      //Set the current screen buffer pixel with the requested color
      *ptr++ = displaydata.fg_color;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_fill_rounded_rect(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 radius)
{
  uint16 *ptr1, *ptr2;
  uint32  x, xc, xs, xe, xt, y, yc, ys, ye;
  uint32  a, step, r;

  //Compensate for the last pixel
  width--;
  height--;
  
  //Calculate the max radius for the given width and height
  if(width < height)
  {
    r = width / 2;
  }
  else
  {
    r = height / 2;
  }
  
  //Check if the given radius is not to big
  if(radius > r)
  {
    //If it is use the max
    radius = r;
  }

  //Determine the angles step fit for the given radius  
  if(radius > 450)
    step = 1;
  else if(radius > 0)
  {
    step = 450 / radius;
  }

  //Only draw the arced corners when there is a radius
  if(radius)
  {
    //Calculate the position of the arc center
    xc = xpos + radius;
    yc = ypos + radius;
    
    //Calculate the biggest x and y end
    xt = xpos + width;
    ye = ypos + height;
    
    //Fill the horizontal lines for the corner sections by working through the angles
    for(a=2700;a>=1800;a-=step)
    {
      //Get the coordinates for the current angle
      xs = getxpos(a, xc, radius);
      y  = getypos(a, yc, radius);
      
      //Calculate the bottom line to draw
      ys = ye - (y - ypos);
      
      //Calculate the x end for this line
      xe = xt - (xs - xpos);
      
      //Check the x bound
      if(xe > displaydata.width)
      {
        xe = displaydata.width;
      }
      
      //Point to the start of the top and the bottom section line
      ptr1 = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + xs);
      ptr2 = displaydata.screenbuffer + ((ys * displaydata.pixelsperline) + xs);
      
      //Draw the pixels on the lines
      //20-02-2022 Fix for wrong shape at the right side. x needs to become equal to xe
      for(x=xs;x<=xe;x++)
      {
        //Set the top line current screen buffer pixel with the requested color
        *ptr1++ = displaydata.fg_color;
        
        //Check on y bound
        if(ys <= displaydata.height)
        {
          //Set the bottom line current screen buffer pixel with the requested color
          *ptr2++ = displaydata.fg_color;
        }
      }
    }
  }
  
  //Make width the xend
  width += xpos;
  
  //Calculate the y positions for the middle section
  ys = ypos + radius;
  ye = ypos + height - radius;
  
  //Check on x bound
  if(width > displaydata.width)
  {
    width = displaydata.width;
  }
  
  //Check on y bound
  if(ye > displaydata.height)
  {
    ye = displaydata.height;
  }
 
  //Draw all the pixels for the middle section
  for(y=ys;y<=ye;y++)
  {
    //Point to the first pixel of this line in the screen buffer
    ptr1 = displaydata.screenbuffer + ((y * displaydata.pixelsperline) + xpos);

    //Draw the pixels on the line
    for(x=xpos;x<=width;x++)
    {
      //Set the current screen buffer pixel with the requested color
      *ptr1++ = displaydata.fg_color;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_slide_top_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed)
{
  register uint16 *ptr1, *ptr2;
  register int32   startline;     //Needs to be an int because it has to become negative to stop
  register uint32  line;
  register uint32  startxy;
  register uint32  pixels = displaydata.pixelsperline;

  //Starting line of the rectangle to display first
  startline = height - ((height * speed) >> 20) - 1;
  
  //Start x,y offset for source and destination calculation
  startxy = xpos + (ypos * displaydata.pixelsperline);
  
  //For copying bytes instead of shorts the width doubles
  width <<=1;
  
  //Draw lines as long as is needed to get the whole rectangle on screen
  while(startline >= 0)
  {
    //Source pointer is based on the current line
    ptr2 = displaydata.sourcebuffer + startxy + (startline * pixels);
    
    //Destination pointer is always the first line
    ptr1 = displaydata.screenbuffer + startxy;
    
    //Handle the needed number of lines for this loop
    for(line=startline;line<height;line++)
    {
      //Copy a single line to the screen buffer
      memcpy(ptr1, ptr2, width);
      
      //Point to the next line of pixels in both destination and source
      ptr1 += pixels;
      ptr2 += pixels;
    }
    
    //Calculate the new starting line
    startline = startline - 1 - ((startline * speed) >> 20);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_slide_left_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed)
{
  register uint16 *ptr1, *ptr2;
  register uint32  line;
  register int32   startpixel;     //Needs to be an int because it has to become negative to stop
  register uint32  bytecount;
  register uint32  startxy;
  register uint32  pixels = displaydata.pixelsperline;

  //Starting pixel of the rectangle to display first
  startpixel = width - ((width * speed) >> 20) - 1;
  
  //Start x,y offset for source and destination calculation
  startxy = xpos + (ypos * pixels);
  
  //Draw sections as long as is needed to get the whole rectangle on screen
  while(startpixel >= 0)
  {
    //Source pointer is based on the current start pixel
    ptr2 = displaydata.sourcebuffer + startxy + startpixel;
    
    //Destination pointer is always the first x,y offset
    ptr1 = displaydata.screenbuffer + startxy;
    
    //Determine the number of pixels to do per loop. Increasing number as start pixel shifts to the left of the bitmap.
    //Need the number in bytes so times two
    bytecount = (width - startpixel) << 1;
    
    //Handle the lines
    for(line=0;line<height;line++)
    {
      //Copy the needed pixels for this loop to the screen buffer
      memcpy(ptr1, ptr2, bytecount);
      
      //Point to the next line of pixels in both destination and source
      ptr1 += pixels;
      ptr2 += pixels;
    }
    
    //Calculate the new starting pixel
    startpixel = startpixel - 1 - ((startpixel * speed) >> 20);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_slide_right_rect_onto_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height, uint32 speed)
{
  register uint16 *ptr1, *ptr2;
  register uint32  line;
  register int32   startpixel;     //Needs to be an int because it has to become negative to stop
  register uint32  bytecount;
  register uint32  startxy;
  register uint32  pixels = displaydata.pixelsperline;

  //Starting pixel of the rectangle where to display first
  startpixel = width - ((width * speed) >> 20) - 1;
  
  //Start x,y offset for source and destination calculation
  startxy = xpos + (ypos * pixels);
  
  //Draw sections as long as is needed to get the whole rectangle on screen
  while(startpixel >= 0)
  {
    //Source pointer is always the first x,y offset
    ptr2 = displaydata.sourcebuffer + startxy;
    
    //Destination pointer is based on the current start pixel
    ptr1 = displaydata.screenbuffer + startxy + startpixel;
    
    //Determine the number of pixels to do per loop. Increasing number as start pixel shifts to the right of the destination bitmap.
    //Need the number in bytes so times two
    bytecount = (width - startpixel) << 1;
    
    //Handle the lines
    for(line=0;line<height;line++)
    {
      //Copy the needed pixels for this loop to the screen buffer
      memcpy(ptr1, ptr2, bytecount);
      
      //Point to the next line of pixels in both destination and source
      ptr1 += pixels;
      ptr2 += pixels;
    }
    
    //Calculate the new starting pixel
    startpixel = startpixel - 1 - ((startpixel * speed) >> 20);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_rect_from_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height)
{
  register uint16 *ptr1, *ptr2;
  register uint32  line;
  register uint32  startpixel;
  register uint32  pixels = displaydata.pixelsperline;

  //Start pixel for source and destination calculation
  startpixel = xpos + (ypos * pixels);

  //Setup destination and source pointers
  ptr1 = displaydata.destbuffer + startpixel;
  ptr2 = displaydata.screenbuffer + startpixel;
  
  //For copying bytes instead of shorts the width doubles
  width <<=1;
  
  //Copy the needed lines
  for(line=0;line<height;line++)
  {
    //Copy a single line to the destination buffer
    memcpy(ptr1, ptr2, width);

    //Point to the next line of pixels in both destination and source
    ptr1 += pixels;
    ptr2 += pixels;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_rect_to_screen(uint32 xpos, uint32 ypos, uint32 width, uint32 height)
{
  register uint16 *ptr1, *ptr2;
  register uint32  line;
  register uint32  startpixel;
  register uint32  pixels = displaydata.pixelsperline;

  //Start pixel for source and destination calculation
  startpixel = xpos + (ypos * pixels);

  //Setup destination and source pointers
  ptr1 = displaydata.screenbuffer + startpixel;
  ptr2 = displaydata.sourcebuffer + startpixel;
  
  //For copying bytes instead of shorts the width doubles
  width <<= 1;
  
  //Copy the needed lines
  for(line=0;line<height;line++)
  {
    //Copy a single line to the destination buffer
    memcpy(ptr1, ptr2, width);

    //Point to the next line of pixels in both destination and source
    ptr1 += pixels;
    ptr2 += pixels;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_copy_icon_use_colors(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height)
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
        //When on use the foreground color
        ptr[pixel] = displaydata.fg_color;
      }
      else
      {
        //When off use the background color
        ptr[pixel] = displaydata.bg_color;
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

void display_copy_icon_fg_color(const uint8 *icon, uint32 xpos, uint32 ypos, uint32 width, uint32 height)
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
      
      //Copy one pixel at a time with a check on being on and only fill in the ones that are on
      if(pixeldata & 0x0100)
      {
        //When on use the foreground color
        ptr[pixel] = displaydata.fg_color;
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

void display_left_pointer(uint32 xpos, uint32 ypos, int8 id)
{
  //Draw the pointer
  display_copy_icon_fg_color(left_pointer_icon, xpos, ypos, 21, 14);
 
  //Set the color for drawing the id
  displaydata.fg_color = displaydata.bg_color;
  
  //Draw the id
  display_character(xpos + 5, ypos, id);
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_right_pointer(uint32 xpos, uint32 ypos, int8 id)
{
  //Draw the pointer
  display_copy_icon_fg_color(right_pointer_icon, xpos, ypos, 21, 14);
 
  //Set the color for drawing the id
  displaydata.fg_color = displaydata.bg_color;
  
  //Draw the id
  display_character(xpos + 9, ypos, id);
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_top_pointer(uint32 xpos, uint32 ypos, int8 id)
{
  //Draw the pointer
  display_copy_icon_fg_color(top_pointer_icon, xpos, ypos, 14, 21);
 
  //Set the color for drawing the id
  displaydata.fg_color = displaydata.bg_color;
  
  //Draw the id
  display_character(xpos + 3, ypos + 1, id);
}

//----------------------------------------------------------------------------------------------------------------------------------

uint8 printhexnibble(uint8 nibble)
{
  //Check if needs to be converted to A-F character
  if(nibble > 9)
  {
    //To make alpha add 55. (55 = 'A' - 10)
    nibble += 55;
  }
  else
  {
    //To make digit add '0'
    nibble += '0';
  }

  return(nibble);
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_hex(uint32 xpos, uint32 ypos, uint32 digits, int32 value)
{
  int8  b[13];
  int32 i;
  int32 shifter;
    
  //Limit to 8 digits
  if(digits > 8)
  {
    digits = 8;
  }
  
  //Set the starting shifter
  shifter = (digits * 4) - 4;
  
  //Put in the hexadecimal leader
  memcpy(b, "0x", 2);
  
  //Compensate for the leader
  digits += 2;
  
  //Put in the digits after the leader
  for(i=2;i<digits;i++)
  {
    //Add the current digit to the string
    b[i] = printhexnibble((value >> shifter) & 0x0F);
    
    //Adjust the shifter
    shifter -= 4;
  }
  
  //Terminate the string
  b[i] = 0;
  
  //Display the result
  display_text(xpos, ypos, b);
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_decimal(uint32 xpos, uint32 ypos, int32 value)
{
  char   b[13];
  uint32 u = value;
  uint32 i = 12;

  if(value == 0)
  {
    //Value is zero so just display a 0 character
    display_text(xpos, ypos, "0");
  }
  else
  {
    //Terminate the string for displaying
    b[12] = 0;

    //Check if negative value
    if(value < 0)
    {
      //Negate if so
      u = -value;
    }

    //Process the digits
    while(u)
    {
      //Set current digit to decreased index
      b[--i] = (u % 10) + '0';

      //Take of the current digit
      u /= 10;
    }

    //Check if negative value for adding the sign
    if(value < 0)
    {
      //If so put minus character in the buffer
      b[--i] = '-';
    }
    
    display_text(xpos, ypos, &b[i]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_character(uint32 xpos, uint32 ypos, int8 text)
{
  //Set the positions for drawing the text
  displaydata.xpos = xpos;
  displaydata.ypos = ypos;
  
  //Check on the type of font for the correct handling function
  if(displaydata.font->type == VARIABLE_WIDTH_FONT)
  {
    //Draw this character in the screen buffer
    draw_vw_character((uint16)text);
  }
  else
  {
    //Draw this character in the screen buffer
    draw_fw_character((uint16)text);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void display_text(uint32 xpos, uint32 ypos, int8 *text)
{
  //Set the positions for drawing
  displaydata.xpos = xpos;
  displaydata.ypos = ypos;
  
  //Check on the type of font for the correct handling function
  if(displaydata.font->type == VARIABLE_WIDTH_FONT)
  {
    //Process all the characters
    while(*text)
    {
      //Draw this character in the screen buffer
      draw_vw_character((uint16)*text);
      
      //Skip to the next character
      text++;
    }
  }
  else if(displaydata.font->type == FIXED_WIDTH_FONT)
  {
    //Process all the characters
    while(*text)
    {
      //Draw this character in the screen buffer
      draw_fw_character((uint16)*text);
      
      //Skip to the next character
      text++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void draw_vw_character(uint16 character)
{
  //Get the font information for this character
  PFONTDATA        font = displaydata.font;
  PFONTINFORMATION info = check_char_in_vw_font(font->fontinformation, character);
  PFONTMETRICS     metrics;
  
  uint16 *ptr;
  int32   idx;
  uint32  height;
  uint32  y;
  uint32  pixel;
  uint32  pixeldata;
  
  //Check if character is valid
  if(info)
  {
    //Get the metrics data for this character
    metrics = &info->fontmetrics[character - info->first_char];

    //Get the actual metrics for drawing the character
    height = font->height;
    
    //Add bounds limiting here for both height and pixels

    //Draw all the pixels
    for(y=0;y<height;y++)
    {
      idx = y * metrics->bytes;
      pixeldata = metrics->data[idx];

      //Point to the first pixel on this line in the screen buffer
      ptr = displaydata.screenbuffer + (((displaydata.ypos + y) * displaydata.pixelsperline) + displaydata.xpos);

      for(pixel=0;pixel<metrics->pixels;)
      {
        //Select the pixel to check
        pixeldata <<= 1;

        //Check if the pixel is set and print it if so
        if(pixeldata & 0x0100)
        {
          //Set the current screen buffer pixel with the requested color
          *ptr = displaydata.fg_color;
        }

        //Select next pixel count and point to next screen buffer pixel
        pixel++;
        ptr++;

        //Check if pixel on multiple of 8 for next byte select
        if((pixel & 0x07) == 0)
        {
          //Point to the next byte
          idx++;

          //Get the actual data for it
          pixeldata = metrics->data[idx];
        }
      }
    }

    displaydata.xpos += metrics->width;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

PFONTINFORMATION check_char_in_vw_font(PFONTINFORMATION info, uint16 character)
{
  //Check if the character is in this information range 
  if((character >= info->first_char) && (character <= info->last_char))
  {
    //If so signal character is found
    return(info);
  }
  
  //Else check if there are no more font information ranges
  if(info->next_info == 0)
  {
    //Signal character invalid
    return(0);
  }
  
  //Else go and check the next information set
  return(check_char_in_vw_font(info->next_info, character));
}

//----------------------------------------------------------------------------------------------------------------------------------

void draw_fw_character(uint16 character)
{
  //Get the font information for this character
  PFONTDATA           font = displaydata.font;
  PFONTFIXEDWIDTHINFO info = font->fontinformation;
  
  uint16 char1 = 0xFFFF;
  uint16 char2 = 0xFFFF;
  uint32 idx;
  
  //Check if the character is in the main information set
  if((character >= info->first_char) && (character <= info->last_char))
  {
    //If so signal character is found
    char1 = character - info->first_char;
  }
  else
  {
    //Get the extended information set
    PFONTEXTENDEDINFO extended = info->extended_info;
    
    //Check if the character is in the extended information set
    if((character >= extended->first_char) && (character <= extended->last_char))
    {
      //Get the translation table if needed
      PFONTTANSLATIONTABLE trans = extended->data;
      
      //Get the index into the translation table
      idx = character - extended->first_char;
      
      //Get the two characters from the translation table
      char1 = trans[idx].char1;
      char2 = trans[idx].char2;
    }
  }
  
  //Check if first character is valid
  if(char1 != 0xFFFF)
  {
    //Draw it if so
    render_fw_character(char1);
  }

  //Check if second character is valid
  if(char2 != 0xFFFF)
  {
    //Draw the second on top of the first
    render_fw_character(char2);
  }
  
  //Check if either character is valid for x position update
  if((char1 != 0xFFFF) || (char2 != 0xFFFF))
  {
    displaydata.xpos += info->width;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void render_fw_character(uint16 character)
{
  //Get the font information for this character
  PFONTDATA           font = displaydata.font;
  PFONTFIXEDWIDTHINFO info = font->fontinformation;
  
  uint8  *data;
  uint16 *ptr;
  int32   idx;
  uint32  height;
  uint32  y;
  uint32  pixel;
  uint32  pixeldata;
  
  //Get the actual metrics for drawing the character
  height = font->height;
  
  //Point to the first byte of the pixel data
  data = info->data + (character * font->height * info->bytes);

  //Add bounds limiting here for both height and pixels
  
  
  //Draw all the pixels
  for(y=0;y<height;y++)
  {
    idx = y * info->bytes;
    pixeldata = data[idx];

    //Point to the first pixel on this line in the screen buffer
    ptr = displaydata.screenbuffer + (((displaydata.ypos + y) * displaydata.pixelsperline) + displaydata.xpos);

    for(pixel=0;pixel<info->pixels;)
    {
      //Select the pixel to check
      pixeldata <<= 1;

      //Check if the pixel is set and print it if so
      if(pixeldata & 0x0100)
      {
        //Set the current screen buffer pixel with the requested color
        *ptr = displaydata.fg_color;
      }

      //Select next pixel count and point to next screen buffer pixel
      pixel++;
      ptr++;

      //Check if pixel on multiple of 8 for next byte select
      if((pixel & 0x07) == 0)
      {
        //Point to the next byte
        idx++;

        //Get the actual data for it
        pixeldata = data[idx];
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
