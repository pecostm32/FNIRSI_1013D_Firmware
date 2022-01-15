//----------------------------------------------------------------------------------------------------------------------------------

#ifndef FONT_STRUCTS_H
#define FONT_STRUCTS_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"

//----------------------------------------------------------------------------------------------------------------------------------

#define VARIABLE_WIDTH_FONT             1
#define FIXED_WIDTH_FONT                2

//----------------------------------------------------------------------------------------------------------------------------------

typedef struct tagFontData               FONTDATA,             *PFONTDATA;
typedef struct tagFontInformation        FONTINFORMATION,      *PFONTINFORMATION;
typedef struct tagFontMetrics            FONTMETRICS,          *PFONTMETRICS;
typedef struct tagFontFixedWidthInfo     FONTFIXEDWIDTHINFO,   *PFONTFIXEDWIDTHINFO;
typedef struct tagFontExtendedInfo       FONTEXTENDEDINFO,     *PFONTEXTENDEDINFO;
typedef struct tagFontTanslationTable    FONTTANSLATIONTABLE,  *PFONTTANSLATIONTABLE;

//----------------------------------------------------------------------------------------------------------------------------------
//Font base structure for all types of fonts

struct tagFontData
{
  uint8     type;                 //Type of font, to indicate variable width or fixed width font
  uint8     height;               //Height of the font used for drawing the pixels
  uint8     baseline;             //Baseline is used as an offset for the y position
  uint8     nu;                   //Not used
  void     *fontinformation;      //Pointer to characters in the font information.
};

//----------------------------------------------------------------------------------------------------------------------------------
//Structures for variable width fonts

struct tagFontInformation
{
  uint16           first_char;
  uint16           last_char;
  PFONTMETRICS     fontmetrics;   //Points to metrics data
  PFONTINFORMATION next_info;     //Points to a possible next info field
};

struct tagFontMetrics
{
  uint8  pixels;                  //Number of pixels per font line
  uint8  width;                   //Number of pixels to displace for next character
  uint8  bytes;                   //Number of bytes per font line
  uint8  nu;                      //Not used
  const uint8 *data;                    //Pointer to the actual pixel data. It uses single bit per pixel bitmaps with 8 bit multiples per line. A character width of 11 uses two bytes per line
};

//----------------------------------------------------------------------------------------------------------------------------------
//Structures for fixed width fonts

struct tagFontFixedWidthInfo
{
  uint8             *data;              //Points to pixel data for the characters
  PFONTEXTENDEDINFO  extended_info;     //Points to a possible extended characters info field
  uint16             first_char;
  uint16             last_char;
  uint8              pixels;            //Number of pixels per font line
  uint8              width;             //Character width
  uint8              bytes;             //Number of bytes per font line
  uint8              nu;                //Not used
};

struct tagFontExtendedInfo
{
  uint16               first_char;
  uint16               last_char;
  PFONTTANSLATIONTABLE data;            //Pointer to a translation table. Two shorts per entry
};

struct tagFontTanslationTable
{
  uint16 char1;                         //If 0xFFFF character is not used. If valid printed first
  uint16 char2;                         //If valid printed second in mode 2. (Only foreground pixels are set)
};

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* FONT_STRUCTS_H */

