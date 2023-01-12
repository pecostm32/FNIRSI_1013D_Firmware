//----------------------------------------------------------------------------------------------------------------------------------

#ifndef VARIABLES_H
#define VARIABLES_H

//----------------------------------------------------------------------------------------------------------------------------------

#include <string.h>

#include "types.h"
#include "font_structs.h"
#include "fnirsi_1013d_scope.h"
#include "ff.h"

//----------------------------------------------------------------------------------------------------------------------------------
//Version info
//----------------------------------------------------------------------------------------------------------------------------------

#define VERSION_STRING             "V0.006"

#define VERSION_STRING_XPOS             690
#define VERSION_STRING_YPOS              24

//----------------------------------------------------------------------------------------------------------------------------------
//Defines
//----------------------------------------------------------------------------------------------------------------------------------

#define SETTINGS_SECTOR                 700    //Location of the settings on the SD card for now

#define VIEW_NOT_ACTIVE                   0
#define VIEW_ACTIVE                       1

#define VIEW_ITEM_XSTART                  2
#define VIEW_ITEM_YSTART                  0
#define VIEW_ITEM_WIDTH                 182
#define VIEW_ITEM_HEIGHT                120

#define VIEW_ITEM_TRACE_POINTS          182

#define VIEW_ITEM_XNEXT                 182
#define VIEW_ITEM_YNEXT                 120

#define VIEW_ITEM_XLAST                 720

#define VIEW_THUMBNAIL_DATA_SIZE     400000
#define VIEW_FILE_NUMBER_DATA_SIZE     2000

#define VIEW_MAX_ITEMS                 1000

#define VIEW_ITEMS_PER_PAGE              16

#define VIEW_TYPE_MASK                    1

#define VIEW_TYPE_PICTURE                 0
#define VIEW_TYPE_WAVEFORM                1

#define VIEW_MAX_TYPES                    2

#define VIEW_ITEM_SELECTED_XSTART        77
#define VIEW_ITEM_SELECTED_YSTART        45

//States for displaying the selected signs
#define VIEW_ITEM_NOT_SELECTED            0
#define VIEW_ITEM_SELECTED_NOT_DISPLAYED  1
#define VIEW_ITEM_SELECTED_DISPLAYED      2
#define VIEW_ITEM_NOT_SELECTED_DISPLAYED  3

//States for select mode
#define VIEW_SELECT_NONE                  0
#define VIEW_SELECT_ALL                   1
#define VIEW_SELECT_INDIVIDUAL            2

//States for bottom file menu displaying
#define VIEW_BOTTON_MENU_INIT             3     //This is a combination of init and show, so show needs to be 1
#define VIEW_BOTTON_MENU_SHOW             1     //Needs to be 1 / 0 for the show and hide since an exor is used to toggle the modes
#define VIEW_BOTTON_MENU_HIDE             0
#define VIEW_BOTTOM_MENU_ACTIVE           8     //A flag needed for the simulator to signal picture viewing is active, so it can give touch on quit

#define VIEW_TRACE_LOAD_OK                0
#define VIEW_TRACE_LOAD_ERROR             1

#define VIEW_BITMAP_LOAD_OK               0
#define VIEW_BITMAP_LOAD_ERROR            1

#define VIEW_CONFIRM_DELETE_NO            1
#define VIEW_CONFIRM_DELETE_YES           2

#define VIEW_NUMBER_OF_SETTINGS         200

#define CHANNEL1_SETTING_OFFSET          10
#define CHANNEL2_SETTING_OFFSET          40
#define TRIGGER_SETTING_OFFSET           70
#define OTHER_SETTING_OFFSET            100
#define CURSOR_SETTING_OFFSET           130
#define MEASUREMENT_SETTING_OFFSET      160
#define CALIBRATION_SETTING_OFFSET      200

#define SETTING_SECTOR_VERSION_HIGH  0x0100
#define SETTING_SECTOR_VERSION_LOW   0x0002

#define WAVEFORM_FILE_VERSION    0x01000002    //Version 1.0.0.2

#define WAVEFORM_FILE_ERROR             200

#define THUMBNAIL_POINTER_RIGHT           0
#define THUMBNAIL_POINTER_LEFT            1
#define THUMBNAIL_POINTER_DOWN            2

#define PICTURE_HEADER_SIZE               70
#define PICTURE_DATA_SIZE                 (800 * 480 * 2)                              //trace data
#define PICTURE_FILE_SIZE                 (PICTURE_HEADER_SIZE + PICTURE_DATA_SIZE)    //Bitmap header + pixel data

#define PICTURE_HEADER_MISMATCH           100

#define MESSAGE_SAVE_SUCCESSFUL           0
#define MESSAGE_FILE_CREATE_FAILED        1
#define MESSAGE_FILE_OPEN_FAILED          2
#define MESSAGE_FILE_WRITE_FAILED         3
#define MESSAGE_FILE_READ_FAILED          4
#define MESSAGE_FILE_SEEK_FAILED          5
#define MESSAGE_FILE_DELETE_FAILED        6

#define MESSAGE_DIRECTORY_CREATE_FAILED   7

#define MESSAGE_FILE_SYSTEM_FAILED        8

#define MESSAGE_THUMBNAIL_FILE_CORRUPT    9
#define MESSAGE_THUMBNAIL_FILE_FULL      10

#define MESSAGE_BMP_HEADER_MISMATCH      11

#define MESSAGE_WAV_VERSION_MISMATCH     12
#define MESSAGE_WAV_CHECKSUM_ERROR       13

//----------------------------------------------------------------------------------------------------------------------------------
//Number of bits used for fixed point calculations on the voltages

#define VOLTAGE_SHIFTER                21

//----------------------------------------------------------------------------------------------------------------------------------
//Menu positions and dimensions
//----------------------------------------------------------------------------------------------------------------------------------
//Run and stop text

#define RUN_STOP_TEXT_XPOS                  97
#define RUN_STOP_TEXT_YPOS                  12
#define RUN_STOP_TEXT_WIDTH                 38
#define RUN_STOP_TEXT_HEIGHT                18

//----------------------------------------------------------------------------------------------------------------------------------
//Channel button and menu

#define CH1_BUTTON_XPOS                    150
#define CH2_BUTTON_XPOS                    260

#define CH_BUTTON_YPOS                       5
#define CH_BUTTON_WIDTH                     30
#define CH_BUTTON_HEIGHT                    35

#define CH_BUTTON_BG_WIDTH                 103
#define CH_BUTTON_BG_HEIGHT                 35

#define CH1_MENU_XPOS          CH1_BUTTON_XPOS
#define CH2_MENU_XPOS          CH2_BUTTON_XPOS

#define CH_MENU_YPOS                        46
#define CH_MENU_WIDTH                      183
#define CH_MENU_HEIGHT                     252

#define CH1_TOUCHED_COLOR            0x000000FF
#define CH2_TOUCHED_COLOR            0x00FF0000

//----------------------------------------------------------------------------------------------------------------------------------
//channel voltage per division buttons

#define CH1_VOLT_DIV_MENU_YPOS              99
#define CH2_VOLT_DIV_MENU_YPOS             264

//----------------------------------------------------------------------------------------------------------------------------------
// Acquisition button and menu

#define ACQ_BUTTON_XPOS                    380
#define ACQ_BUTTON_YPOS                      5
#define ACQ_BUTTON_WIDTH                    30
#define ACQ_BUTTON_HEIGHT                   35

#define ACQ_BUTTON_BG_WIDTH                103
#define ACQ_BUTTON_BG_HEIGHT                35

#define ACQ_MENU_XPOS          ACQ_BUTTON_XPOS
#define ACQ_MENU_YPOS                       46
#define ACQ_MENU_WIDTH                     304
#define ACQ_MENU_HEIGHT                    336

//----------------------------------------------------------------------------------------------------------------------------------
// Sampling system

#define MAX_SAMPLE_BUFFER_SIZE            3000
#define UINT32_SAMPLE_BUFFER_SIZE         (MAX_SAMPLE_BUFFER_SIZE / 4)

#define SAMPLE_COUNT                      MAX_SAMPLE_BUFFER_SIZE
#define SAMPLES_PER_ADC                   (SAMPLE_COUNT / 2)

//----------------------------------------------------------------------------------------------------------------------------------
//Typedefs
//----------------------------------------------------------------------------------------------------------------------------------

typedef struct tagTouchCoords           TOUCHCOORDS,          *PTOUCHCOORDS;

typedef struct tagDisplayPoints         DISPLAYPOINTS,        *PDISPLAYPOINTS;

typedef struct tagChannelSettings       CHANNELSETTINGS,      *PCHANNELSETTINGS;
typedef struct tagScopeSettings         SCOPESETTINGS,        *PSCOPESETTINGS;

typedef struct tagThumbnailData         THUMBNAILDATA,        *PTHUMBNAILDATA;

typedef struct tagPathInfo              PATHINFO,             *PPATHINFO;

typedef struct tagScreenTimeCalcData    SCREENTIMECALCDATA,   *PSCREENTIMECALCDATA;
typedef struct tagVoltCalcData          VOLTCALCDATA,         *PVOLTCALCDATA;
typedef struct tagFreqCalcData          FREQCALCDATA,         *PFREQCALCDATA;
typedef struct tagTimeCalcData          TIMECALCDATA,         *PTIMECALCDATA;

//----------------------------------------------------------------------------------------------------------------------------------

typedef void (*MEASUREMENTFUNCTION)(PCHANNELSETTINGS settings);

//----------------------------------------------------------------------------------------------------------------------------------
//Structures
//----------------------------------------------------------------------------------------------------------------------------------

struct tagTouchCoords
{
  uint16 x1;
  uint16 x2;
  uint16 y1;
  uint16 y2;
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagDisplayPoints
{
  uint16 x;
  uint16 y;  
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagChannelSettings
{
  //Settings
  uint8  enable;
  uint8  coupling;
  uint8  magnification;
  uint8  displayvoltperdiv;
  uint8  samplevoltperdiv;
  uint8  fftenable;
  
  //Trace on screen position
  uint16 traceposition;

  //New setting for controlling the ground level of the ADC differential input
  uint16 dcoffset;
  
  //Inter ADC difference compensation
  int16  compensation;
  int16  adc1compensation;
  int16  adc2compensation;
  
  //DC offset calibration for center level of the ADC's
  uint16 dc_calibration_offset[7];
  
  //Measurements
  int32  min;
  int32  max;
  int32  average;
  int32  center;
  int32  peakpeak;
  uint32 rms;
  uint32 frequencyvalid;
  uint32 frequency;
  uint32 lowtime;
  uint32 hightime;
  uint32 periodtime;
  
  //Frequency determination work variables
  uint32 highlevel;
  uint32 lowlevel;
  uint32 state;
  uint32 zerocrossings;
  uint32 lowsamplecount;
  uint32 lowdivider;
  uint32 highsamplecount;
  uint32 highdivider;
  uint32 previousindex;
  
  //Auto ranging space
  uint32 maxscreenspace;
  
  //Calibration measurement data
  uint32 rawaverage;
  uint32 adc1rawaverage;
  uint32 adc2rawaverage;
  
  //Sample data
  uint8 *tracebuffer;
  uint8 *buffer;
  
  //Screen data
  PDISPLAYPOINTS tracepoints;
  uint32         noftracepoints;
  
  //Sample gathering options
  uint8 checkfirstadc;
  uint8 enabletrigger;
  
  //FPGA commands
  uint8 enablecommand;            //Needs to be set to 0x02 for channel 1 and 0x03 for channel 2
  uint8 couplingcommand;          //Needs to be set to 0x34 for channel 1 and 0x37 for channel 2
  uint8 voltperdivcommand;        //Needs to be set to 0x33 for channel 1 and 0x36 for channel 2
  uint8 offsetcommand;            //Needs to be set to 0x32 for channel 1 and 0x35 for channel 2
  uint8 adc1command;              //Needs to be set to 0x20 for channel 1 and 0x22 for channel 2
  uint8 adc2command;              //Needs to be set to 0x21 for channel 1 and 0x23 for channel 2
  
  //Channel color
  uint32 color;
  
  //Channel button and menu
  uint32 buttonxpos;
  uint32 menuxpos;
  uint32 voltdivypos;
  uint32 touchedcolor;
  
  
  int8 *buttontext;
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagScopeSettings
{
  CHANNELSETTINGS channel1;
  CHANNELSETTINGS channel2;

  uint16 samplecount;       //Number of samples in trace buffer
  uint16 nofsamples;        //Number of samples to read from the FPGA
  
  uint8 samplerate;
  uint8 timeperdiv;
  uint8 triggermode;
  uint8 triggeredge;
  uint8 triggerchannel;
  
  uint16 triggerhorizontalposition;    //Position on screen of the trigger point in the signal displaying
  uint16 triggerverticalposition;      //Screen position of the trigger level indicator
  uint16 triggerlevel;                 //Actual trigger level set to the FPGA
  
  uint8 samplemode;                    //New for mode select in the fpga_do_conversion function
  
  uint8 movespeed;
  
  uint8 rightmenustate;
  uint8 waveviewmode;
  
  uint8 batterychargelevel;
  uint8 batterycharging;
  uint8 runstate;
  
  uint8 screenbrightness;
  uint8 gridbrightness;
  uint8 gridenable;
  uint8 alwaystrigger50;
  uint8 xymodedisplay;
  uint8 confirmationmode;
  
  uint8 timecursorsenable;
  uint8 voltcursorsenable;
  
  uint16 timecursor1position;
  uint16 timecursor2position;
  
  uint16 voltcursor1position;
  uint16 voltcursor2position;
  
  uint8 measuresstate[2][12];
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagThumbnailData
{
  int8  filename[33];
  uint8 channel1enable;
  uint8 channel2enable;
  uint8 channel1traceposition;
  uint8 channel2traceposition;
  uint8 triggerverticalposition;
  uint8 triggerhorizontalposition;
  uint8 xydisplaymode;
  uint8 disp_xstart;
  uint8 disp_xend;
  uint8 channel1data[VIEW_ITEM_TRACE_POINTS];
  uint8 channel2data[VIEW_ITEM_TRACE_POINTS];
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagPathInfo
{
  char   *name;
  uint32  length;
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagScreenTimeCalcData
{
  uint32 mul_factor;
  uint8  time_scale;
  uint8  freq_scale;
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagVoltCalcData
{
  uint32 mul_factor;
  uint8  volt_scale;
};

//----------------------------------------------------------------------------------------------------------------------------------

struct tagFreqCalcData
{
  uint32 sample_rate;
  uint8  freq_scale;
};
        
//----------------------------------------------------------------------------------------------------------------------------------

struct tagTimeCalcData
{
  uint32 mul_factor;
  uint8  time_scale;
};
        
//----------------------------------------------------------------------------------------------------------------------------------
//Linker variables
//----------------------------------------------------------------------------------------------------------------------------------

extern uint8 BSS_START;
extern uint8 BSS_END;

//----------------------------------------------------------------------------------------------------------------------------------
//Timer data
//----------------------------------------------------------------------------------------------------------------------------------

extern volatile uint32 timer0ticks;

//----------------------------------------------------------------------------------------------------------------------------------
//Touch data
//----------------------------------------------------------------------------------------------------------------------------------

#ifndef USE_TP_CONFIG
extern uint8 tp_config_data[186];

extern uint32 xscaler;
extern uint32 yscaler;
#endif

extern uint8  havetouch;
extern uint16 xtouch;
extern uint16 ytouch;

extern uint8 xswap;
extern uint8 yswap;

extern uint8 config_valid;

//----------------------------------------------------------------------------------------------------------------------------------
//State machine data
//----------------------------------------------------------------------------------------------------------------------------------

extern uint16 previousxtouch;
extern uint16 previousytouch;

extern uint16 xtouchdisplacement;
extern uint16 ytouchdisplacement;

extern uint16 maxdisplacement;

extern uint8 touchstate;

extern uint32 previoustimerticks;

extern uint8 systemsettingsmenuopen;
extern uint8 screenbrightnessopen;
extern uint8 gridbrightnessopen;
extern uint8 calibrationopen;

//----------------------------------------------------------------------------------------------------------------------------------
//Scope data
//----------------------------------------------------------------------------------------------------------------------------------

extern FATFS fs;

extern SCOPESETTINGS scopesettings;

extern CHANNELSETTINGS calibrationsettings;

extern SCOPESETTINGS savedscopesettings1;
extern SCOPESETTINGS savedscopesettings2;

extern uint32 channel1tracebuffer[UINT32_SAMPLE_BUFFER_SIZE];

extern DISPLAYPOINTS channel1pointsbuffer[730];

extern uint32 channel2tracebuffer[UINT32_SAMPLE_BUFFER_SIZE];

extern DISPLAYPOINTS channel2pointsbuffer[730];

extern uint16 thumbnailtracedata[730];

extern uint16 settingsworkbuffer[256];

//New variables for trace displaying
extern double disp_xpos_per_sample;
extern double disp_sample_step;

extern int32 disp_first_sample;

extern uint32 disp_have_trigger;
extern uint32 disp_trigger_index;

extern int32 disp_xstart;
extern int32 disp_xend;

//----------------------------------------------------------------------------------------------------------------------------------
//Distances of touch point to traces and cursors
//----------------------------------------------------------------------------------------------------------------------------------

extern uint16 distance_channel_1;
extern uint16 distance_channel_2;

extern uint16 distance_trigger_level;

extern uint16 distance_time_cursor_left;
extern uint16 distance_time_cursor_right;

extern uint16 distance_volt_cursor_top;
extern uint16 distance_volt_cursor_bottom;

//----------------------------------------------------------------------------------------------------------------------------------
//Previous trace and cursor settings
//----------------------------------------------------------------------------------------------------------------------------------

extern uint16 previous_channel_1_offset;
extern uint16 previous_channel_2_offset;

extern uint16 previous_trigger_level_offset;

extern uint16 previous_trigger_point_position;

extern uint16 previous_left_time_cursor_position;
extern uint16 previous_right_time_cursor_position;

extern uint16 previous_top_volt_cursor_position;
extern uint16 previous_bottom_volt_cursor_position;

//----------------------------------------------------------------------------------------------------------------------------------
//Calibration data
//----------------------------------------------------------------------------------------------------------------------------------

extern uint32 samplerateindex;

//Average data for calibration calculations
extern uint32 samplerateaverage[2][6];

//Single ADC bit dc offset step per input sensitivity setting
extern uint32 sampleratedcoffsetstep[2][6];

//----------------------------------------------------------------------------------------------------------------------------------
//Predefined data
//----------------------------------------------------------------------------------------------------------------------------------

extern const int8 *volt_div_texts[3][7];

extern const int32 signal_adjusters[7];

extern const uint32 timebase_settings[24];

extern const uint32 sample_rate_settings[18];

extern const float sample_time_converters[18];

extern const uint32 time_per_div_matching[24];

extern const uint32 samplerate_for_autosetup[4];

extern const SCREENTIMECALCDATA screen_time_calc_data[24];

extern const VOLTCALCDATA volt_calc_data[3][7];

extern const FREQCALCDATA freq_calc_data[18];

extern const TIMECALCDATA time_calc_data[18];

extern const char *magnitude_scaler[8];

extern const int32 vertical_scaling_factors[7][7];

extern const PATHINFO view_file_path[2];
extern const char     view_file_extension[2][5];
extern const char    *thumbnail_file_names[2];

extern const uint8 bmpheader[PICTURE_HEADER_SIZE];

extern const uint32 frequency_per_div[24];
extern const uint32 sample_rate[18];

extern const uint8 time_per_div_sample_rate[24];
extern const uint8 sample_rate_time_per_div[18];
extern const uint8 viable_time_per_div[18][24];

extern const int8 *time_div_texts[24];
extern const int8 time_div_text_x_offsets[24];

extern const int8 *acquisition_speed_texts[18];
extern const int8 acquisition_speed_text_x_offsets[18];

extern const char *measurement_names[12];

//----------------------------------------------------------------------------------------------------------------------------------
//For touch filtering on slider movement
//----------------------------------------------------------------------------------------------------------------------------------

extern uint16 prevxtouch;

//----------------------------------------------------------------------------------------------------------------------------------
//Data for picture and waveform view mode
//----------------------------------------------------------------------------------------------------------------------------------

extern FIL     viewfp;
extern DIR     viewdir;
extern FILINFO viewfileinfo;

extern char viewfilename[32];

extern uint8 viewactive;

extern uint8 viewtype;

extern uint8 viewselectmode;
extern uint8 viewpage;
extern uint8 viewpages;
extern uint8 viewitemsonpage;

extern uint8 viewbottommenustate;

extern uint16 viewcurrentindex;

extern uint16 viewavailableitems;

extern uint8 viewitemselected[VIEW_ITEMS_PER_PAGE];

extern THUMBNAILDATA viewthumbnaildata[VIEW_MAX_ITEMS];

extern uint16 viewfilenumberdata[VIEW_MAX_ITEMS];

extern uint8 viewbitmapheader[PICTURE_HEADER_SIZE];

extern uint32 viewfilesetupdata[VIEW_NUMBER_OF_SETTINGS];

//----------------------------------------------------------------------------------------------------------------------------------
//Display data
//----------------------------------------------------------------------------------------------------------------------------------

//This first buffer is defined as 32 bits to be able to write it to file
extern uint32 maindisplaybuffer[SCREEN_SIZE / 2];

extern uint16 displaybuffer1[SCREEN_SIZE];
extern uint16 displaybuffer2[SCREEN_SIZE];

extern uint16 gradientbuffer[SCREEN_HEIGHT];

extern char measurementtext[20];

//----------------------------------------------------------------------------------------------------------------------------------
//Fonts
//----------------------------------------------------------------------------------------------------------------------------------

extern FONTDATA font_0;
extern FONTDATA font_2;
extern FONTDATA font_3;
extern FONTDATA font_4;
extern FONTDATA font_5;

//----------------------------------------------------------------------------------------------------------------------------------
//Icons
//----------------------------------------------------------------------------------------------------------------------------------

extern const uint8 system_settings_icon[];
extern const uint8 picture_view_icon[];
extern const uint8 waveform_view_icon[];
extern const uint8 usb_icon[];
extern const uint8 screen_brightness_icon[];
extern const uint8 grid_brightness_icon[];
extern const uint8 trigger_50_percent_icon[];
extern const uint8 baseline_calibration_icon[];
extern const uint8 x_y_mode_display_icon[];
extern const uint8 confirmation_icon[];
extern const uint8 return_arrow_icon[];
extern const uint8 left_pointer_icon[];
extern const uint8 right_pointer_icon[];
extern const uint8 top_pointer_icon[];
extern const uint8 select_sign_icon[];
extern const uint8 waste_bin_icon[];
extern const uint8 previous_picture_icon[];
extern const uint8 next_picture_icon[];

//----------------------------------------------------------------------------------------------------------------------------------

extern const uint8 letter_c_icon[];
extern const uint8 letter_e_icon[];
extern const uint8 letter_o_icon[];
extern const uint8 letter_p_icon[];
extern const uint8 letter_s_icon[];

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* VARIABLES_H */

