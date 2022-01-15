//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "scope_functions.h"
#include "statemachine.h"
#include "touchpanel.h"
#include "timer.h"
#include "fpga_control.h"
#include "spi_control.h"
#include "sd_card_interface.h"
#include "display_lib.h"
#include "ff.h"

#include "usb_interface.h"
#include "variables.h"

#include "sin_cos_math.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_display_lib(void)
{
  display_set_bg_color(0x00000000);

  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  display_set_dimensions(SCREEN_WIDTH, SCREEN_HEIGHT);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_main_screen(void)
{
  //Prepare the screen in a working buffer
//  display_set_screen_buffer(displaybuffer1);

  //Setup for clearing top and right menu bars
  display_set_fg_color(0x00000000);

  //Top bar for the main, channel and trigger menu and information
  display_fill_rect(0, 0, 730, 46);

  //Right bar for control buttons
  display_fill_rect(730, 0, 70, 480);

  //Setup the menu bar on the right side
  scope_setup_right_control_menu();

  //Check if normal or wave view mode
  if(scopesettings.waveviewmode == 0)
  {
    //Normal mode so show menu button
    scope_menu_button(0);
  }
  else
  {
    //Wave view mode so show return button
    scope_main_return_button(0);
  }

  //Show the user if the acquisition is running or stopped
  scope_run_stop_text();

  //Display the channel menu select buttons and their settings
  scope_channel_settings(&scopesettings.channel1, 0);
  scope_channel_settings(&scopesettings.channel2, 0);

  //Display the current time per div setting
  scope_acqusition_settings(0);

  //Show the user selected move speed
  scope_move_speed(0);

  //Display the trigger menu select button and the settings
  scope_trigger_settings(0);

  //Show the battery charge level and charging indicator
  scope_battery_status();
  
  //Show version information
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_2);
  display_text(VERSION_STRING_XPOS, VERSION_STRING_YPOS, VERSION_STRING);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_view_screen(void)
{
  //Switch to view mode so disallow saving of settings on power down
  viewactive = VIEW_ACTIVE;

  //Set scope run state to running to have it sample fresh data on exit
  scopesettings.runstate = 0;

  //Only needed for waveform view. Picture viewing does not change the scope settings
  if(viewtype == VIEW_TYPE_WAVEFORM)
  {
    //Save the current settings
    scope_save_setup(&savedscopesettings1);
  }
  
  //Initialize the view mode variables
  //Used for indicating if select all or select button is active
  viewselectmode = 0;

  //Start at first page
  viewpage = 0;

  //Clear the item selected flags
  memset(viewitemselected, VIEW_ITEM_NOT_SELECTED, VIEW_ITEMS_PER_PAGE);

  //Set storage buffer for screen capture under selected signs and messages
  display_set_destination_buffer(displaybuffer2);
  display_set_source_buffer(displaybuffer2);

  //Display the file actions menu on the right side of the screen
  scope_setup_right_file_menu();

  //Load the thumbnail file for the current view type
  if(scope_load_thumbnail_file() != 0)
  {
    //Restore the main screen
    scope_setup_main_screen();
    
    //Loading the thumbnail file failed so no sense in going on
    return;
  }

  //Display the available thumbnails for the current view type
  scope_initialize_and_display_thumbnails();

  //Handle touch for this part of the user interface
  handle_view_mode_touch();

  //This is only needed when an actual waveform has been viewed, but that needs an extra flag
  //Only needed for waveform view. Picture viewing does not change the scope settings
  if(viewtype == VIEW_TYPE_WAVEFORM)
  {
    //Restore the current settings
    scope_restore_setup(&savedscopesettings1);

    //Make sure view mode is normal
    scopesettings.waveviewmode = 0;

    //And resume with auto trigger mode
    scopesettings.triggermode = 0;

    //Need to restore the original scope data and fpga settings

    //Is also part of startup, so could be done with a function
    //Set the volts per div for each channel based on the loaded scope settings
    fpga_set_channel_voltperdiv(&scopesettings.channel1);
    fpga_set_channel_voltperdiv(&scopesettings.channel2);

    //These are not done in the original code
    //Set the channels AC or DC coupling based on the loaded scope settings
    fpga_set_channel_coupling(&scopesettings.channel1);
    fpga_set_channel_coupling(&scopesettings.channel2);

    //Setup the trigger system in the FPGA based on the loaded scope settings
    fpga_set_sample_rate(scopesettings.samplerate);
    fpga_set_trigger_channel();
    fpga_set_trigger_edge();
    fpga_set_trigger_level();
    fpga_set_trigger_mode();

    //Set channel screen offsets
    fpga_set_channel_offset(&scopesettings.channel1);
    fpga_set_channel_offset(&scopesettings.channel2);
  }

  //Reset the screen to the normal scope screen
  scope_setup_main_screen();

  //Back to normal mode so allow saving of settings on power down
  viewactive = VIEW_NOT_ACTIVE;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_usb_screen(void)
{
  //Clear the whole screen
  display_set_fg_color(0x00000000);
  display_fill_rect(0, 0, 800, 480);

  //Set the light color for the equipment borders
  display_set_fg_color(0x00AAAAAA);

  //Draw the computer screen
  display_fill_rounded_rect(470, 115, 250, 190, 2);
  display_fill_rect(580, 305, 30, 20);
  display_fill_rounded_rect(550, 325, 90, 10, 2);
  display_fill_rect(550, 331, 89, 4);

  //Draw the scope
  display_fill_rounded_rect(80, 200, 180, 135, 2);

  //Draw the cable
  display_fill_rect(210, 188, 10, 12);
  display_fill_rect(213, 154, 4, 36);
  display_fill_rect(213, 150, 152, 4);
  display_fill_rect(361, 154, 4, 106);
  display_fill_rect(361, 260, 98, 4);
  display_fill_rect(458, 257, 12, 10);

  //Fill in the screens with a blue color
  display_set_fg_color(0x00000055);
  display_fill_rect(477, 125, 235, 163);
  display_fill_rect(88, 210, 163, 112);

  //Draw a dark border around the blue screens
  display_set_fg_color(0x00111111);
  display_draw_rect(477, 125, 235, 163);
  display_draw_rect(88, 210, 163, 112);

  //Display the text with font 5 and the light color
  display_set_font(&font_5);
  display_set_fg_color(0x00AAAAAA);
  display_text(125, 254, "ON / OFF");

  //Start the USB interface
  usb_device_enable();

  //Wait for the user to touch the scope ON / OFF section to quit
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the text field
      if((xtouch >= 90) && (xtouch <= 250) && (ytouch >= 210) && (ytouch <= 320))
      {
        //Touched the text field so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        break;
      }
    }
  }

  //Stop the USB interface
  usb_device_disable();

  //Re-sync the system files
  scope_sync_thumbnail_files();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_right_control_menu(void)
{
  //Setup for clearing right menu bar
  display_set_fg_color(0x00000000);

  //Clear the right bar for the control buttons
  display_fill_rect(730, 0, 70, 480);

  //Display the control button
  scope_control_button(0);

  //Check in which state the right menu is in
  if(scopesettings.rightmenustate == 0)
  {
    //Main control state so draw the always used buttons
    scope_t_cursor_button(0);
    scope_v_cursor_button(0);
    scope_measures_button(0);
    scope_save_picture_button(0);

    //Check if in wave view mode
    if(scopesettings.waveviewmode == 0)
    {
      //Main control mode buttons
      scope_run_stop_button(0);
      scope_auto_set_button(0);
      scope_save_wave_button(0);
    }
    else
    {
      //Wave view mode buttons
      scope_previous_wave_button(0);
      scope_next_wave_button(0);
      scope_delete_wave_button(0);
    }
  }
  else
  {
    //Channel sensitivity state
    scope_channel_sensitivity_control(&scopesettings.channel1, 0, 0);
    scope_channel_sensitivity_control(&scopesettings.channel2, 0, 0);

    //Check if in wave view mode
    if(scopesettings.waveviewmode == 0)
    {
      //Main control mode
      scope_50_percent_trigger_button(0);
    }
    else
    {
      //Wave view mode
      scope_show_grid_button(0);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_right_file_menu(void)
{
  //Set black color for background
  display_set_fg_color(0x00000000);

  //Clear the right menu bar
  display_fill_rect(730, 0, 70, 480);

  //Add the buttons
  scope_return_button(0);
  scope_select_all_button(0);
  scope_select_button(0);
  scope_delete_button(0);
  scope_page_up_button(0);
  scope_page_down_button(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_bottom_file_menu(int mode)
{
  //Check if background needs to be saved
  if(mode == VIEW_BOTTON_MENU_INIT)
  {
    //Save the screen rectangle where the menu will be displayed
    display_copy_rect_from_screen(0, 390, 800, 90);
  }

  //Check if it needs to be drawn
  if(mode & VIEW_BOTTON_MENU_SHOW)
  {
    //Draw the background in grey
    display_set_fg_color(0x00202020);
    display_fill_rect(0, 390, 800, 90);

    //Draw the filename in white
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_5);
    display_text(20, 392, viewfilename);
    
    //Setup the buttons
    scope_bmp_return_button(0);    
    scope_bmp_delete_button(0);
    scope_bmp_previous_button(0);
    scope_bmp_next_button(0);
    
    //Signal menu is visible
    viewbottommenustate = VIEW_BOTTON_MENU_SHOW | VIEW_BOTTOM_MENU_ACTIVE;
  }
  else
  {
    //Hide the menu bar
    display_copy_rect_to_screen(0, 390, 800, 90);

    //Signal menu is not visible
    viewbottommenustate = VIEW_BOTTON_MENU_HIDE | VIEW_BOTTOM_MENU_ACTIVE;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Right side bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_control_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Grey color for inactive button
    display_set_fg_color(0x00404040);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 5, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(739, 5, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(748, 22, "CTRL");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_run_stop_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 65, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 65, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 75, "RUN/");
  display_text(746, 90, "STOP");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_auto_set_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 125, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 125, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 135, "AUTO");
  display_text(753, 150, "SET");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_previous_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 65, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 65, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(749, 75, "prev");
  display_text(748, 90, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_next_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 125, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 125, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(749, 135, "next");
  display_text(748, 150, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_t_cursor_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 185, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 185, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 195, "T");
  display_text(765, 195, "CU");
  display_text(746, 210, "RSOR");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_v_cursor_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 245, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 245, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 255, "V");
  display_text(765, 255, "CU");
  display_text(746, 270, "RSOR");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_measures_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 305, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 305, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 315, "MEAS");
  display_text(746, 330, "URES");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_picture_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 365, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 365, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 375, "SAVE");
  display_text(753, 390, "PIC");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 435, "SAVE");
  display_text(746, 450, "WAVE");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_delete_wave_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(745, 436, "delete");
  display_text(748, 449, "wave");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_50_percent_trigger_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(752, 436, "50%");
  display_text(749, 449, "TRIG");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_grid_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }
  else
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(739, 425, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, 425, 51, 50, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(748, 436, "show");
  display_text(752, 449, "grid");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_sensitivity_control(PCHANNELSETTINGS settings, int type, int mode)
{
  uint32 y;

  //Check if V+ is active or inactive
  if((type == 0) && (mode != 0))
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }

  //Top button
  display_fill_rounded_rect(739, settings->voltdivypos, 51, 50, 2);
  
  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, settings->voltdivypos, 51, 50, 2);

  //Check if V- is active or inactive
  if((type != 0) && (mode != 0))
  {
    //Orange color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //Black color for inactive button
    display_set_fg_color(0x00202020);
  }

  //Y position o f bottom button
  y = settings->voltdivypos + 86;
  
  //Bottom button
  display_fill_rounded_rect(739, y, 51, 50, 2);

  //Draw the edge
  display_set_fg_color(0x00505050);
  display_draw_rounded_rect(739, y, 51, 50, 2);
  
  //Display V+ and V- the text in larger font
  display_set_font(&font_0);
  
  //Check if V+ is active or inactive
  if((type == 0) && (mode != 0))
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }
  else
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Top button text
  display_text(757, settings->voltdivypos + 18, "V+");

  //Check if V- is active or inactive
  if((type != 0) && (mode != 0))
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }
  else
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  
  //Bottom button text
  display_text(757, settings->voltdivypos + 104, "V-");

  //Display the channel identifier bar with the channel color
  display_set_fg_color(settings->color);
  display_fill_rect(739, settings->voltdivypos + 56, 51, 24);

  //Display the channel identifier in black
  display_set_fg_color(0x00000000);
  display_set_font(&font_2);
  display_text(754, settings->voltdivypos + 61, settings->buttontext);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_return_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 14, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 14, 63, 58, 2);
  
  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(747, 34, "return");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_select_all_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else if(mode == 1)
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //White for enabled button
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 93, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 93, 63, 58, 2);
  
  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 106, "select");
  display_text(758, 120, "all");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_select_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else if(mode == 1)
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }
  else
  {
    //White for enabled button
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 173, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 173, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 193, "select");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_delete_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 253, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 253, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(746, 273, "delete");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_page_up_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 333, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 333, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(750, 345, "page");
  display_text(758, 360, "up");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_page_down_button(int mode)
{
  //Check if inactive or active mode
  if(mode == 0)
  {
    //Black color for inactive button
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active color for activated button
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Draw the body of the button
  display_fill_rounded_rect(734, 413, 63, 58, 2);

  //Outline the button
  display_set_fg_color(0x00606060);
  display_draw_rounded_rect(734, 413, 63, 58, 2);

  //Check if inactive or active mode
  if(mode == 0)
  {
    //White text color for inactive button
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Black text color for activated button
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(750, 425, "page");
  display_text(748, 442, "down");
}

//----------------------------------------------------------------------------------------------------------------------------------
// Bitmap control bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_return_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(40, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(40, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(return_arrow_icon, 79, 436, 41, 27);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_delete_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(240, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(240, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(waste_bin_icon, 284, 433, 31, 33);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_previous_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(440, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(440, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(previous_picture_icon, 483, 438, 33, 24);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_next_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00303030);
  }
  else
  {
    //Active so active color background
    display_set_fg_color(ITEM_ACTIVE_COLOR);
  }

  //Fill in the body of the button
  display_fill_rounded_rect(640, 425, 120, 50, 3);

  //Draw rounded rectangle as button border in black
  display_set_fg_color(0x00000000);
  display_draw_rounded_rect(640, 425, 120, 50, 3);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00303030);
  }
  else
  {
    //Active so black foreground and active color background
    display_set_fg_color(0x00303030);
    display_set_bg_color(ITEM_ACTIVE_COLOR);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(next_picture_icon, 683, 438, 33, 24);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Top bar function
//----------------------------------------------------------------------------------------------------------------------------------

void scope_menu_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark blue background
    display_set_fg_color(0x00000078);
  }
  else
  {
    //Active so pale yellow background
    display_set_fg_color(0x00FFFF80);
  }

  //Draw the background
  display_fill_rect(0, 0, 80, 38);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black foreground
    display_set_fg_color(0x00000000);
  }

  //Draw the menu symbol
  display_fill_rect(6,  11, 7, 7);
  display_fill_rect(15, 11, 7, 7);
  display_fill_rect(6,  20, 7, 7);
  display_fill_rect(15, 20, 7, 7);

  //Display the text
  display_set_font(&font_3);
  display_text(32, 11, "MENU");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_return_button(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so black background
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00FFFFFF);
  }

  //Draw the background
  display_fill_rect(0, 0, 80, 38);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00000000);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFFFF);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(return_arrow_icon, 20, 5, 41, 27);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_run_stop_text(void)
{
  //Check if run or stop mode
  if(scopesettings.runstate == 0)
  {
    //Run mode. Black box
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Stop mode. Red box
    display_set_fg_color(0x00FF0000);
  }

  //Fill the box
  display_fill_rect(RUN_STOP_TEXT_XPOS, RUN_STOP_TEXT_YPOS, RUN_STOP_TEXT_WIDTH, RUN_STOP_TEXT_HEIGHT);

  //Select the font for the text
  display_set_font(&font_3);

  //Check if run or stop mode
  if(scopesettings.runstate == 0)
  {
    //Run mode. White text
    display_set_fg_color(0x00FFFFFF);
    display_text(RUN_STOP_TEXT_XPOS + 4, RUN_STOP_TEXT_YPOS + 1, "RUN");
  }
  else
  {
    //Stop mode. Black text
    display_set_fg_color(0x00000000);
    display_text(RUN_STOP_TEXT_XPOS + 1, RUN_STOP_TEXT_YPOS + 1, "STOP");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_settings(PCHANNELSETTINGS settings, int mode)
{
  int8 **vdtext;

  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(settings->buttonxpos, CH_BUTTON_YPOS, CH_BUTTON_BG_WIDTH, CH_BUTTON_BG_HEIGHT);

  //Check if channel is enabled or disabled
  if(settings->enable == 0)
  {
    //Disabled so off colors
    //Check if inactive or active
    if(mode == 0)
    {
      //Inactive, grey menu button
      display_set_fg_color(0x00444444);
    }
    else
    {
      //Active, light grey menu button
      display_set_fg_color(0x00BBBBBB);
    }
  }
  else
  {
    //Enabled so on colors
    //Check if inactive or active
    if(mode == 0)
    {
      //Inactive, channel 1 color menu button
      display_set_fg_color(settings->color);
    }
    else
    {
      //Active, blue menu button
      display_set_fg_color(settings->touchedcolor);
    }
  }

  //Fill the button
  display_fill_rect(settings->buttonxpos, CH_BUTTON_YPOS, CH_BUTTON_WIDTH, CH_BUTTON_HEIGHT);

  //Select the font for the text
  display_set_font(&font_2);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(settings->buttonxpos + 30, CH_BUTTON_YPOS, CH_BUTTON_BG_WIDTH - 30, CH_BUTTON_BG_HEIGHT);
  }

  //Display the channel identifier text
  display_text(settings->buttonxpos + 5, CH_BUTTON_YPOS + 11, settings->buttontext);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Check on which coupling is set
  if(settings->coupling == 0)
  {
    //DC coupling
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 3, "DC");
  }
  else
  {
    //AC coupling
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 3, "AC");
  }

  //Print the probe magnification factor
  switch(settings->magnification)
  {
    case 0:
      //Times 1 magnification
      display_text(settings->buttonxpos + 63, CH_BUTTON_YPOS + 3, "1X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[0];
      break;

    case 1:
      //Times 10 magnification
      display_text(settings->buttonxpos + 61, CH_BUTTON_YPOS + 3, "10X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[1];
      break;

    default:
      //Times 100 magnification
      display_text(settings->buttonxpos + 59, CH_BUTTON_YPOS + 3, "100X");

      //Set the volts per div text range to be used for this magnification
      vdtext = (int8 **)volt_div_texts[2];
      break;
  }

  //Display the sensitivity when in range
  if(settings->displayvoltperdiv < 7)
  {
    display_text(settings->buttonxpos + 38, CH_BUTTON_YPOS + 19, vdtext[settings->displayvoltperdiv]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acqusition_settings(int mode)
{
  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(ACQ_BUTTON_XPOS, ACQ_BUTTON_YPOS, ACQ_BUTTON_BG_WIDTH, ACQ_BUTTON_BG_HEIGHT);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, green menu button
    display_set_fg_color(TRIGGER_COLOR);
  }
  else
  {
    //Active, magenta menu button
    display_set_fg_color(0x00FF00FF);
  }

  //Fill the button
  display_fill_rect(ACQ_BUTTON_XPOS, ACQ_BUTTON_YPOS, ACQ_BUTTON_WIDTH, ACQ_BUTTON_HEIGHT);

  //Select the font for the text
  display_set_font(&font_2);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(ACQ_BUTTON_XPOS + 30, ACQ_BUTTON_YPOS, ACQ_BUTTON_BG_WIDTH - 30, ACQ_BUTTON_BG_HEIGHT);
  }

  //Display the acquisition identifier text
  display_text(ACQ_BUTTON_XPOS + 4, ACQ_BUTTON_YPOS + 11, "ACQ");

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Only display the text when in range of the text array
  if(scopesettings.samplerate < (sizeof(acquisition_speed_texts) / sizeof(int8 *)))
  {
    //Display the text from the table
    display_text(ACQ_BUTTON_XPOS + 38, ACQ_BUTTON_YPOS + 3, (int8 *)acquisition_speed_texts[scopesettings.samplerate]);
  }

  //Only display the text when in range of the text array
  if(scopesettings.timeperdiv < (sizeof(time_div_texts) / sizeof(int8 *)))
  {
    //Display the text from the table
    display_text(ACQ_BUTTON_XPOS + 38, ACQ_BUTTON_YPOS + 19, (int8 *)time_div_texts[scopesettings.timeperdiv]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_move_speed(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark blue background
    display_set_fg_color(0x00000078);
  }
  else
  {
    //Active so pale yellow background
    display_set_fg_color(0x00FFFF80);
  }

  //Draw the background
  display_fill_rect(493, 5, 40, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black text
    display_set_fg_color(0x00000000);
  }

  //Select the font for the text
  display_set_font(&font_3);

  //Display the common text
  display_text(496, 6, "move");

  //Check on which speed is set
  if(scopesettings.movespeed == 0)
  {
    display_text(501, 21, "fast");
  }
  else
  {
    display_text(499, 21, "slow");
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_settings(int mode)
{
  int8 *modetext = 0;

  //Clear the area first
  display_set_fg_color(0x00000000);
  display_fill_rect(570, 5, 110, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, green menu button
    display_set_fg_color(TRIGGER_COLOR);
  }
  else
  {
    //Active, magenta menu button
    display_set_fg_color(0x00FF00FF);
  }

  //Fill the button
  display_fill_rect(570, 5, 31, 35);

  //Select the font for the text
  display_set_font(&font_4);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, black text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Active, white text
    display_set_fg_color(0x00FFFFFF);

    //Fill the settings background
    display_fill_rect(601, 5, 68, 35);
  }

  //Display the channel identifier text
  display_text(582, 15, "T");

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive, white text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active, black text
    display_set_fg_color(0x00000000);
  }

  //Check on which trigger mode is set
  switch(scopesettings.triggermode)
  {
    case 0:
      modetext = "Auto";
      break;

    case 1:
      modetext = "Single";
      break;

    case 2:
      modetext = "Normal";
      break;
  }

  //Select the font for the texts
  display_set_font(&font_2);

  //Check if valid setting
  if(modetext)
  {
    //Display the selected text if so
    display_text(606, 7, modetext);
  }

  //Draw the trigger edge symbol
  display_draw_vert_line(642, 27, 38);

  //Draw the arrow based on the selected edge
  if(scopesettings.triggeredge == 0)
  {
    //rising edge
    display_draw_horz_line(27, 642, 645);
    display_draw_horz_line(38, 639, 642);
    display_draw_horz_line(32, 641, 643);
    display_draw_horz_line(33, 640, 644);
  }
  else
  {
    //falling edge
    display_draw_horz_line(27, 639, 642);
    display_draw_horz_line(38, 642, 645);
    display_draw_horz_line(32, 640, 644);
    display_draw_horz_line(33, 641, 643);
  }

  //Check on which channel is used for triggering
  switch(scopesettings.triggerchannel)
  {
    //Channel 1
    case 0:
      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, dark channel 1 trigger color box
        display_set_fg_color(CHANNEL1_TRIG_COLOR);
      }
      else
      {
        //Active, some blue box
        display_set_fg_color(0x003333FF);
      }

      //Fill the channel background
      display_fill_rect(605, 25, 28, 15);

      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, black text
        display_set_fg_color(0x00000000);
      }
      else
      {
        //Active, white text
        display_set_fg_color(0x00FFFFFF);
      }

      //Display the text
      display_text(608, 26, "CH1");
      break;

    //Channel 2
    case 1:
      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, dark cyan box
        display_set_fg_color(CHANNEL2_TRIG_COLOR);
      }
      else
      {
        //Active, some red box
        display_set_fg_color(0x00FF3333);
      }

      //Fill the channel background
      display_fill_rect(605, 25, 28, 15);

      //Check if inactive or active
      if(mode == 0)
      {
        //Inactive, black text
        display_set_fg_color(0x00000000);
      }
      else
      {
        //Active, white text
        display_set_fg_color(0x00FFFFFF);
      }

      //Display the text
      display_text(608, 26, "CH2");
      break;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_battery_status(void)
{
  //Prepare the battery symbol in a working buffer to avoid flicker
  display_set_screen_buffer(displaybuffer1);

  //Clear the background
  display_set_fg_color(0x00000000);
  display_fill_rect(701, 5, 25, 13);

  //Draw an empty battery symbol in white
  display_set_fg_color(0x00FFFFFF);
  display_fill_rect(701, 9, 2, 4);
  display_fill_rect(703, 5, 22, 12);

  //Check if there is any charge
  if(scopesettings.batterychargelevel)
  {
    //Keep charge level on max if above
    if(scopesettings.batterychargelevel > 20)
    {
      //Max for displaying the level
      scopesettings.batterychargelevel = 20;
    }

    //Check if charge level is low
    if(scopesettings.batterychargelevel < 4)
    {
      //Draw the level indicator in red
      display_set_fg_color(0x00FF0000);
    }
    else
    {
      //Draw the level indicator in dark green
      display_set_fg_color(0x0000BB00);
    }

    //Draw the indicator based on the level
    display_fill_rect(724 - scopesettings.batterychargelevel, 6, scopesettings.batterychargelevel, 10);
  }

  //Draw the battery charging indicator when plugged in
  if(scopesettings.batterycharging)
  {
#if 0
    //Some light blue color
    display_set_fg_color(0x002222FF);

    //Draw an arrow when charging
    display_draw_horz_line(10, 708, 718);
    display_draw_horz_line(11, 708, 718);
    display_draw_horz_line(12, 708, 718);
    display_draw_vert_line(719, 8, 14);
    display_draw_vert_line(720, 9, 13);
    display_draw_vert_line(721, 10, 12);
    display_draw_vert_line(722, 11, 11);
#else
    //Some orange color
    display_set_fg_color(0x00FF6A00);

    //Draw a lightning bolt when charging
    display_draw_horz_line( 7, 715, 716);
    display_draw_horz_line( 8, 713, 716);
    display_draw_horz_line( 9, 711, 715);
    display_draw_horz_line(10, 709, 715);
    display_draw_horz_line(11, 707, 711);
    display_draw_horz_line(12, 705, 709);
    display_draw_horz_line(11, 713, 715);
    display_draw_horz_line(10, 719, 723);
    display_draw_horz_line(11, 717, 721);
    display_draw_horz_line(12, 713, 719);
    display_draw_horz_line(13, 713, 717);
    display_draw_horz_line(14, 712, 715);
    display_draw_horz_line(15, 712, 713);
#endif
  }

  //Copy it to the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(701, 5, 25, 13);
}

//----------------------------------------------------------------------------------------------------------------------------------
// Menu functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_main_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(0, 46, 149, 233);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(0, 46, 149, 233);

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(104, 9, 140);
  display_draw_horz_line(163, 9, 140);
  display_draw_horz_line(222, 9, 140);

  //Display the menu items
  scope_main_menu_system_settings(0);
  scope_main_menu_picture_view(0);
  scope_main_menu_waveform_view(0);
  scope_main_menu_usb_connection(0);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(0, 46, 149, 233, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_system_settings(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(9, 59, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(system_settings_icon, 21, 63, 15, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(69, 60, "System");
  display_text(68, 76, "settings");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_picture_view(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 116, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(picture_view_icon, 17, 122, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(73, 119, "Picture");
  display_text(79, 135, "view");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_waveform_view(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 175, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(waveform_view_icon, 17, 181, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(62, 178, "Waveform");
  display_text(79, 194, "view");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_main_menu_usb_connection(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so white background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(9, 235, 131, 35);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and white background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00CCCCCC);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(usb_icon, 20, 239, 18, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(80, 237, "USB");
  display_text(60, 253, "connection");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_channel_menu(PCHANNELSETTINGS settings)
{
  uint32 xstart;
  uint32 xend;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

  //Line start and end x positions
  xstart = settings->menuxpos + 14;
  xend   = settings->menuxpos + CH_MENU_WIDTH - 14;

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(CH_MENU_YPOS +  62, xstart, xend);
  display_draw_horz_line(CH_MENU_YPOS + 124, xstart, xend);
  display_draw_horz_line(CH_MENU_YPOS + 188, xstart, xend);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(settings->menuxpos + 15, CH_MENU_YPOS +  10, "open");
  display_text(settings->menuxpos + 22, CH_MENU_YPOS +  29, "CH");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS +  72, "open");
  display_text(settings->menuxpos + 19, CH_MENU_YPOS +  91, "FFT");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 136, "coup");
  display_text(settings->menuxpos + 18, CH_MENU_YPOS + 154, "ling");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 201, "probe");
  display_text(settings->menuxpos + 15, CH_MENU_YPOS + 219, "mode");

  //Display the actual settings
  scope_channel_enable_select(settings);
  scope_channel_fft_show(settings);
  scope_channel_coupling_select(settings);
  scope_channel_probe_magnification_select(settings);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(settings->menuxpos, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT, 69906);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_enable_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so dark grey box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 16, 32, 22);
  }
  else
  {
    //Enabled so dark grey box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 16, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so channel color box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 16, 32, 22);
  }
  else
  {
    //Enabled so channel color box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 16, 32, 22);
  }

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so white on text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Enabled so black on text
    display_set_fg_color(0x00000000);
  }

  //Display the on text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 19, "ON");

  //Check if channel is disabled or enabled
  if(settings->enable == 0)
  {
    //Disabled so black off text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Enabled so white off text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the off text
  display_text(settings->menuxpos + 133, CH_MENU_YPOS + 19, "OFF");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_fft_show(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so dark grey box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 78, 32, 22);
  }
  else
  {
    //Enabled so dark grey box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 78, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so channel color box behind off
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 78, 32, 22);
  }
  else
  {
    //Enabled so channel color box behind on
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 78, 32, 22);
  }

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so white on text
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Enabled so black on text
    display_set_fg_color(0x00000000);
  }

  //Display the on text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 81, "ON");

  //Check if fft is disabled or enabled
  if(settings->fftenable == 0)
  {
    //Disabled so black off text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //Enabled so white off text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the off text
  display_text(settings->menuxpos + 133, CH_MENU_YPOS + 81, "OFF");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_coupling_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so dark grey box behind ac text
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 142, 32, 22);
  }
  else
  {
    //AC so dark grey box behind dc text
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 142, 32, 22);
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so channel color box behind dc text
    display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 142, 32, 22);
  }
  else
  {
    //AC so channel color box behind ac text
    display_fill_rect(settings->menuxpos + 130, CH_MENU_YPOS + 142, 32, 22);
  }

  //Check if coupling is dc or ac
  if(settings->coupling == 0)
  {
    //DC so black DC text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //AC so white or grey DC text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the DC text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 145, "DC");

  //Check if coupling is DC or AC
  if(settings->coupling == 0)
  {
    //DC so white or grey AC text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //AC so black AC text
    display_set_fg_color(0x00000000);
  }

  //Display the off text
  display_text(settings->menuxpos + 135, CH_MENU_YPOS + 145, "AC");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_channel_probe_magnification_select(PCHANNELSETTINGS settings)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00181818);

  //Check if coupling is dc or ac
  switch(settings->magnification)
  {
    case 0:
      //dark grey times 10 and 100 magnification
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;

    case 1:
      //dark grey times 1 and 100 magnification
      display_fill_rect(settings->menuxpos +  78, CH_MENU_YPOS + 199, 20, 38);
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;

    default:
      //dark grey times 1 and 10 magnification
      display_fill_rect(settings->menuxpos +  78, CH_MENU_YPOS + 199, 20, 38);
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      break;
  }

  //Set channel color for the box behind the selected text
  display_set_fg_color(settings->color);

  //Check if which magnification to highlight
  switch(settings->magnification)
  {
    case 0:
      //Highlight times 1 magnification
      display_fill_rect(settings->menuxpos + 78, CH_MENU_YPOS + 199, 20, 38);
      break;

    case 1:
      //Highlight times 10 magnification
      display_fill_rect(settings->menuxpos + 109, CH_MENU_YPOS + 199, 23, 38);
      break;

    default:
      //Highlight times 100 magnification
      display_fill_rect(settings->menuxpos + 138, CH_MENU_YPOS + 199, 30, 38);
      break;
  }

  //Check if magnification is 1x
  if(settings->magnification == 0)
  {
    //Yes so black 1X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 1X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 1X text
  display_text(settings->menuxpos + 84, CH_MENU_YPOS + 201, "1");
  display_text(settings->menuxpos + 83, CH_MENU_YPOS + 219, "X");

  //Check if magnification is 10x
  if(settings->magnification == 1)
  {
    //Yes so black 10X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 10X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 10X text
  display_text(settings->menuxpos + 113, CH_MENU_YPOS + 201, "10");
  display_text(settings->menuxpos + 115, CH_MENU_YPOS + 219, "X");

  //Check if magnification is 100x
  if(settings->magnification > 1)
  {
    //Yes so black 100X text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //No so white or grey 100X text
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the 100X text
  display_text(settings->menuxpos + 142, CH_MENU_YPOS + 201, "100");
  display_text(settings->menuxpos + 149, CH_MENU_YPOS + 219, "X");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_acquisition_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);

  //A black line between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(ACQ_MENU_YPOS +  158, ACQ_MENU_XPOS + 8, ACQ_MENU_XPOS + ACQ_MENU_WIDTH - 8);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(ACQ_MENU_XPOS + 111, ACQ_MENU_YPOS +   8, "Sample Rate");
  display_text(ACQ_MENU_XPOS +  97, ACQ_MENU_YPOS + 166, "Time per Division");

  //Display the actual settings
  scope_acquisition_speed_select();
  scope_acquisition_timeperdiv_select();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT, 69906);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquisition_speed_select(void)
{
  uint32 i,x,y;

  //Select the font for the texts
  display_set_font(&font_2);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00383838);

  //Clear the boxes for the not selected items
  for(i=0;i<(sizeof(acquisition_speed_texts) / sizeof(int8 *));i++)
  {
    if(i != scopesettings.samplerate)
    {
      x = ((i & 3) * 72) + 10;
      y = ((i >> 2) * 23) + 33;

      display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);
    }
  }

  //Set channel 2 color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Get the position of the selected item
  x = ((scopesettings.samplerate & 3) * 72) + 10;
  y = ((scopesettings.samplerate >> 2) * 23) + 33;

  //Highlight the selected item
  display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);

  for(i=0;i<(sizeof(acquisition_speed_texts) / sizeof(int8 *));i++)
  {
    if(i != scopesettings.samplerate)
    {
      //Check if in stop mode
      if(scopesettings.runstate)
      {
        //When stopped select option is disabled so texts shown in light grey
        display_set_fg_color(0x00606060);
      }
      else
      {
        //When running available not selected texts shown in white
        display_set_fg_color(0x00FFFFFF);
      }
    }
    else
    {
      //Selected texts in black
      display_set_fg_color(0x00000000);
    }

    //Calculate the position of this text
    x = ((i & 3) * 72) + acquisition_speed_text_x_offsets[i];
    y = ((i >> 2) * 23) + 36;

    //Display the text from the table
    display_text(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, (int8 *)acquisition_speed_texts[i]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquisition_timeperdiv_select(void)
{
  uint32 c,i,x,y;

  //Select the font for the texts
  display_set_font(&font_2);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00383838);

  //Clear the boxes for the not selected items
  for(i=0;i<(sizeof(time_div_texts) / sizeof(int8 *));i++)
  {
    //Settings displayed from smallest to highest value
    c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - i;

    if(c != scopesettings.timeperdiv)
    {
      x = ((i & 3) * 72) + 10;
      y = ((i >> 2) * 23) + 191;

      display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);
    }
  }

  //Set channel 2 color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Get the position of the selected item
  c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - scopesettings.timeperdiv;
  x = ((c & 3) * 72) + 10;
  y = ((c >> 2) * 23) + 191;

  //Highlight the selected item
  display_fill_rect(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, 68, 20);

  for(i=0;i<(sizeof(time_div_texts) / sizeof(int8 *));i++)
  {
    //Settings displayed from smallest to highest value
    c = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - i;

    //Check if the current text is the selected on
    if(c != scopesettings.timeperdiv)
    {
      //When not check if the current on is a viable candidate for full screen trace display
      if(viable_time_per_div[scopesettings.samplerate][c])
      {
        //Available but viable not selected texts in white
        display_set_fg_color(0x00FFFFFF);
      }
      else
      {
        //Not viable but available not selected texts in grey
        display_set_fg_color(0x00686868);
      }
    }
    else
    {
      //Selected texts in black
      display_set_fg_color(0x00000000);
    }

    //Calculate the position of this text
    x = ((i & 3) * 72) + time_div_text_x_offsets[c];
    y = ((i >> 2) * 23) + 194;

    //Display the text from the table
    display_text(ACQ_MENU_XPOS + x, ACQ_MENU_YPOS + y, (int8 *)time_div_texts[c]);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_trigger_menu(void)
{
  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(560, 46, 172, 186);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(560, 46, 172, 186);

  //Three black lines between the settings
  display_set_fg_color(0x00000000);
  display_draw_horz_line(107, 570, 722);
  display_draw_horz_line(168, 570, 722);

  //Main texts in white
  display_set_fg_color(0x00FFFFFF);

  //Select the font for the texts
  display_set_font(&font_3);

  //Display the texts
  display_text(570,  56, "trigger");
  display_text(570,  75, "mode");
  display_text(570, 118, "trigger");
  display_text(570, 137, "edge");
  display_text(570, 182, "trigger");
  display_text(570, 200, "channel");

  //Display the actual settings
  scope_trigger_mode_select();
  scope_trigger_edge_select();
  scope_trigger_channel_select();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_top_rect_onto_screen(560, 46, 172, 186, 56415);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_mode_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the boxes behind the not selected texts
  display_set_fg_color(0x00181818);

  //Check which trigger mode is selected
  switch(scopesettings.triggermode)
  {
    case 0:
      //dark grey single and normal
      display_fill_rect(661, 57, 20, 38);
      display_fill_rect(692, 57, 21, 38);
      break;

    case 1:
      //dark grey auto and normal
      display_fill_rect(629, 57, 20, 38);
      display_fill_rect(692, 57, 21, 38);
      break;

    default:
      //dark grey auto and single
      display_fill_rect(629, 57, 20, 38);
      display_fill_rect(661, 57, 20, 38);
      break;
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check if which trigger mode to highlight
  switch(scopesettings.triggermode)
  {
    case 0:
      //Highlight auto mode
      display_fill_rect(629, 57, 20, 38);
      break;

    case 1:
      //Highlight single mode
      display_fill_rect(661, 57, 20, 38);
      break;

    default:
      //Highlight normal mode
      display_fill_rect(692, 57, 21, 38);
      break;
  }

  //Check if trigger mode is auto
  if(scopesettings.triggermode == 0)
  {
    //Yes so black auto text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White auto text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey auto text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the auto text
  display_text(631, 58, "au");
  display_text(633, 75, "to");

  //Check if trigger mode is single
  if(scopesettings.triggermode == 1)
  {
    //Yes so black single text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White single text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey single text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the single text
  display_text(666, 56, "si");
  display_text(663, 66, "ng");
  display_text(665, 79, "le");

  //Check if trigger mode is normal
  if(scopesettings.triggermode > 1)
  {
    //Yes so black normal text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //When not selected check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White normal text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey normal text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the normal text
  display_text(695, 56, "no");
  display_text(694, 66, "rm");
  display_text(696, 79, "al");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_edge_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so dark grey box behind falling
    display_fill_rect(671, 125, 45, 22);
  }
  else
  {
    //Falling so dark grey box behind rising
    display_fill_rect(626, 125, 40, 22);
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so trigger color box behind rising
    display_fill_rect(626, 125, 40, 22);
  }
  else
  {
    //Falling so trigger color box behind falling
    display_fill_rect(671, 125, 45, 22);
  }

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //Rising so black rising text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //For falling edge trigger check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the rising text
  display_text(629, 127, "rising");

  //Check which trigger edge is selected
  if(scopesettings.triggeredge == 0)
  {
    //For rising edge trigger check if in normal view mode
    if(scopesettings.waveviewmode == 0)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //Falling so black falling text
    display_set_fg_color(0x00000000);
  }

  //Display the falling text
  display_text(674, 127, "falling");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_trigger_channel_select(void)
{
  //Select the font for the texts
  display_set_font(&font_3);

  //Set dark grey color for the box behind the not selected text
  display_set_fg_color(0x00181818);

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so dark grey box behind CH2 text
    display_fill_rect(680, 188, 32, 22);
  }
  else
  {
    //2 so dark grey box behind CH1 text
    display_fill_rect(632, 188, 32, 22);
  }

  //Set trigger color for the box behind the selected text
  display_set_fg_color(TRIGGER_COLOR);

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so trigger color box behind CH1 text
    display_fill_rect(632, 188, 32, 22);
  }
  else
  {
    //2 so trigger color box behind CH2 text
    display_fill_rect(680, 188, 32, 22);
  }

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so black CH1 text
    display_set_fg_color(0x00000000);
  }
  else
  {
    //2 so white or grey CH1 text
    if((scopesettings.waveviewmode == 0) && scopesettings.channel1.enable)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }

  //Display the CH1 text
  display_text(635, 191, "CH1");

  //Check if channel is 1 or 2
  if(scopesettings.triggerchannel == 0)
  {
    //1 so white or grey CH2 text
    if((scopesettings.waveviewmode == 0) && scopesettings.channel2.enable)
    {
      //White text if in normal mode
      display_set_fg_color(0x00FFFFFF);
    }
    else
    {
      //Grey text when in waveform view mode
      display_set_fg_color(0x00606060);
    }
  }
  else
  {
    //2 so black CH2 text
    display_set_fg_color(0x00000000);
  }

  //Display the CH2 text
  display_text(683, 191, "CH2");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_system_settings_menu(void)
{
  int y;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(150, 46, 244, 353);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(150, 46, 244, 353);

  //Five black lines between the settings
  display_set_fg_color(0x00000000);

  for(y=104;y<350;y+=59)
  {
    display_draw_horz_line(y, 159, 385);
  }

  //Display the menu items
  scope_system_settings_screen_brightness_item(0);
  scope_system_settings_grid_brightness_item(0);
  scope_system_settings_trigger_50_item();
  scope_system_settings_calibration_item(0);
  scope_system_settings_x_y_mode_item();
  scope_system_settings_confirmation_item();

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(150, 46, 244, 353, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_screen_brightness_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 59, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(screen_brightness_icon, 171, 63, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(231, 60, "Screen");
  display_text(220, 76, "brightness");

  //Show the actual setting
  scope_system_settings_screen_brightness_value();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_screen_brightness_value(void)
{
  //Draw the yellow background
  display_set_fg_color(0x00FFFF00);
  display_fill_rect(332, 67, 32, 15);

  //Display the number with fixed width font and black color
  display_set_font(&font_0);
  display_set_fg_color(0x00000000);
  display_decimal(337, 68, scopesettings.screenbrightness);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_grid_brightness_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 116, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(grid_brightness_icon, 171, 122, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(240, 119, "Grid");
  display_text(220, 135, "brightness");

  //Show the actual setting
  scope_system_settings_grid_brightness_value();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_grid_brightness_value(void)
{
  //Draw the yellow background
  display_set_fg_color(0x00FFFF00);
  display_fill_rect(332, 124, 32, 15);

  //Display the number with fixed width font and black color
  display_set_font(&font_0);
  display_set_fg_color(0x00000000);
  display_decimal(337, 125, scopesettings.gridbrightness);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_trigger_50_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(trigger_50_percent_icon, 171, 181, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(229, 178, "Always");
  display_text(217, 194, "trigger 50%");

  //Show the state
  scope_display_slide_button(326, 183, scopesettings.alwaystrigger50);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_calibration_item(int mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00181818);
  }
  else
  {
    //Active so yellow background
    display_set_fg_color(0x00FFFF00);
  }

  //Draw the background
  display_fill_rect(159, 235, 226, 36);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground and grey background
    display_set_fg_color(0x00FFFFFF);
    display_set_bg_color(0x00181818);
  }
  else
  {
    //Active so black foreground and yellow background
    display_set_fg_color(0x00000000);
    display_set_bg_color(0x00FFFF00);
  }

  //Display the icon with the set colors
  display_copy_icon_use_colors(baseline_calibration_icon, 171, 239, 24, 25);

  //Display the text
  display_set_font(&font_3);
  display_text(225, 237, "Baseline");
  display_text(219, 253, "calibration");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_x_y_mode_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(x_y_mode_display_icon, 171, 297, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(223, 295, "X-Y mode");
  display_text(231, 311, "display");

  //Show the state
  scope_display_slide_button(326, 299, scopesettings.xymodedisplay);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_system_settings_confirmation_item(void)
{
  //Set the colors for white foreground and grey background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x00181818);

  //Display the icon with the set colors
  display_copy_icon_use_colors(confirmation_icon, 171, 356, 24, 24);

  //Display the text
  display_set_font(&font_3);
  display_text(217, 354, "Notification");
  display_text(213, 370, "confirmation");

  //Show the state
  scope_display_slide_button(326, 358, scopesettings.confirmationmode);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_calibration_start_text(void)
{
  //Save the screen under the baseline calibration start text
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(395, 222, 199, 59);

  //Setup the text in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 199, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 199, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(409, 227, "Please unplug");
  display_text(409, 243, "the probe and");
  display_text(409, 259, "USB first !");

  //Add the ok button
  scope_display_ok_button(517, 230, 0);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(395, 222, 199, 59, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_calibrating_text(void)
{
  //Restore the screen from under the calibration start text to get rid of it
  display_set_source_buffer(displaybuffer2);
  display_copy_rect_to_screen(395, 222, 199, 59);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 110, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 110, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(409, 243, "Calibrating...");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_show_calibration_done_text(void)
{
  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(395, 222, 110, 59);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);
  display_draw_rect(395, 222, 110, 59);

  //Display the text in white
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(414, 235, "Calibration");
  display_text(416, 251, "successful");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_measures_menu(void)
{
  int item;
  int channel;

  //Setup the menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);
  display_fill_rect(231, 263, 499, 214);

  //Draw the edge in black
  display_set_fg_color(0x00000000);
  display_draw_rect(231, 263, 499, 214);

  //Three horizontal black lines between the settings
  display_draw_horz_line(288, 232, 729);
  display_draw_horz_line(350, 232, 729);
  display_draw_horz_line(412, 232, 729);

  //Vertical separator between the channel sections
  display_draw_vert_line(481, 264, 476);

  //Vertical separators between the items
  display_draw_vert_line(294, 289, 476);
  display_draw_vert_line(356, 289, 476);
  display_draw_vert_line(418, 289, 476);
  display_draw_vert_line(544, 289, 476);
  display_draw_vert_line(606, 289, 476);
  display_draw_vert_line(668, 289, 476);

  //Channel 1 top bar
  display_set_fg_color(CHANNEL1_COLOR);
  display_fill_rect(482, 264, 247, 23);

  //Channel 2 top bar
  display_set_fg_color(CHANNEL2_COLOR);
  display_fill_rect(232, 264, 248, 23);

  //Display the channel identifier text in black
  display_set_fg_color(0x00000000);
  display_set_font(&font_2);
  display_text(490, 269, "CH1");
  display_text(240, 269, "CH2");

  //Display the menu items
  for(channel=0;channel<2;channel++)
  {
    //For each channel 12 items
    for(item=0;item<12;item++)
    {
      //Draw the separate items
      scope_measures_menu_item(channel, item);
    }
  }

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_right_rect_onto_screen(231, 263, 499, 214, 75646);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_measures_menu_item(int channel, int item)
{
  uint16  xpos;
  uint16  ypos;
  char   *text;

  //Set the x position offset for the given channel
  if(channel == 0)
  {
    //Channel 1 is on the right side
    xpos = 482;
  }
  else
  {
    //Channel 2 is on the left side
    xpos = 232;
  }

  //Set the text and the position for the given item
  switch(item)
  {
    case 0:
      text  = "Vmax";
      xpos += 15;
      ypos  = 312;
      break;

    case 1:
      text  = "Vmin";
      xpos += 79;
      ypos  = 312;
      break;

    case 2:
      text  = "Vavg";
      xpos += 141;
      ypos  = 312;
      break;

    case 3:
      text  = "Vrms";
      xpos += 203;
      ypos  = 312;
      break;

    case 4:
      text  = "VPP";
      xpos += 19;
      ypos  = 376;
      break;

    case 5:
      text  = "VP";
      xpos += 86;
      ypos  = 376;
      break;

    case 6:
      text  = "Freq";
      xpos += 143;
      ypos  = 376;
      break;

    case 7:
      text  = "Cycle";
      xpos += 201;
      ypos  = 376;
      break;

    case 8:
      text  = "Tim+";
      xpos += 17;
      ypos  = 437;
      break;

    case 9:
      text  = "Tim-";
      xpos += 80;
      ypos  = 437;
      break;

    case 10:
      text  = "Duty+";
      xpos += 138;
      ypos  = 437;
      break;

    case 11:
      text  = "Duty-";
      xpos += 202;
      ypos  = 437;
      break;

    default:
      return;
  }

  //Check if item is on or off
  if(scopesettings.measuresstate[channel][item] == 0)
  {
    //Off so some dark grey text
    display_set_fg_color(0x00444444);
  }
  else
  {
    //On so white text
    display_set_fg_color(0x00FFFFFF);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(xpos, ypos, text);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_slider(uint16 xpos, uint16 ypos, uint8 position)
{
  //Save the screen under the screen brightness slider
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(xpos, ypos, 331, 58);

  //Setup the slider menu in a separate buffer to be able to slide it onto the screen
  display_set_screen_buffer(displaybuffer1);

  //Draw the background in dark grey
  display_set_fg_color(0x00181818);

  //Fill the background
  display_fill_rect(xpos, ypos, 331, 58);

  //Draw the edge in a lighter grey
  display_set_fg_color(0x00333333);

  //Draw the edge
  display_draw_rect(xpos, ypos, 331, 58);

  //Display the actual slider
  scope_display_slider(xpos, ypos, position);

  //Set source and target for getting it on the actual screen
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Slide the image onto the actual screen. The speed factor makes it start fast and end slow, Smaller value makes it slower.
  display_slide_left_rect_onto_screen(xpos, ypos, 331, 58, 63039);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_slider(uint16 xpos, uint16 ypos, uint8 position)
{
  uint16 x = xpos + 20;
  uint16 y = ypos + 24;
  uint16 w = (291 * position) / 100;
  uint16 ys = ypos + 23;
  uint16 ye = ypos + 35;

  //Clear the background first
  display_set_fg_color(0x00181818);
  display_fill_rect(xpos + 8, ypos + 17, 315, 24);

  //Draw the first part of the slider bar in a yellow color
  display_set_fg_color(0x00FFFF00);
  display_fill_rounded_rect(x, y, w, 10, 2);

  //Adjust positions for the grey part
  x += w;
  w  = 291 - w;

  //Draw the last part of the slider bar in a light grey color
  display_set_fg_color(0x00666666);
  display_fill_rounded_rect(x, y, w, 10, 2);

  //Adjust positions for drawing the knob
  x -= 11;
  y -= 6;

  //Draw the knob
  display_set_fg_color(0x00AAAAAA);
  display_fill_rounded_rect(x, y, 22, 22, 2);

  //Draw the black lines on the knob
  display_set_fg_color(0x00000000);
  display_draw_vert_line(x +  6, ys, ye);
  display_draw_vert_line(x + 11, ys, ye);
  display_draw_vert_line(x + 16, ys, ye);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_move_slider(uint16 xpos, uint16 ypos, uint8 *position)
{
  uint16 xs = xpos + 20;
  uint16 value;
  int16 filter = xtouch - prevxtouch;

  //Check if update needed
  if((filter > -3) && (filter < 3))
  {
    //When change in movement less the absolute 3 don't process
    return(0);
  }

  //Save for next filter check
  prevxtouch = xtouch;

  //Make sure it stays in allowed range
  if(xtouch <= xs)
  {
    //Below slider keep it on 0
    value = 0;
  }
  else if(xtouch >= (xpos + 311))
  {
    //Above slider keep it on max
    value = 100;
  }
  else
  {
    //Based on xtouch position calculate a new position from 0 to 100
    value = ((xtouch - xs) * 100) / 291;
  }

  //Update the position variable
  *position = value;

  //Show the new position on screen
  scope_display_slider(xpos, ypos, value);

  //Signal there is change
  return(1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_slide_button(uint16 xpos, uint16 ypos, uint8 state)
{
  uint16 linex      = xpos + 8;
  uint16 lineystart = ypos + 6;
  uint16 lineyend   = ypos + 15;
  uint16 buttonx    = xpos + 4;
  uint32 edgecolor  = 0x00444444;
  uint32 fillcolor  = 0x00888888;

  if(state == 1)
  {
    //Displace the lines and button by 19 pixels
    linex   += 19;
    buttonx += 19;

    //Set the enabled colors
    edgecolor  = 0x00008800;
    fillcolor  = 0x0000FF00;
  }

  //Draw the background
  display_set_fg_color(fillcolor);
  display_fill_rounded_rect(xpos, ypos, 45, 21, 2);

  //Draw the edge
  display_set_fg_color(edgecolor);
  display_draw_rounded_rect(xpos, ypos, 45, 21, 2);

  //Draw button in dark grey
  display_set_fg_color(0x00444444);
  display_fill_rect(buttonx, ypos + 4, 19, 13);

  //Draw lines in black
  display_set_fg_color(0x00000000);
  display_draw_vert_line(linex,     lineystart, lineyend);
  display_draw_vert_line(linex + 3, lineystart, lineyend);
  display_draw_vert_line(linex + 6, lineystart, lineyend);
  display_draw_vert_line(linex + 9, lineystart, lineyend);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_ok_button(uint16 xpos, uint16 ypos, uint8 mode)
{
  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so dark grey background
    display_set_fg_color(0x00333333);
  }
  else
  {
    //Active so light grey background
    display_set_fg_color(0x00CCCCCC);
  }

  //Draw the background
  display_fill_rect(xpos, ypos, 66, 44);

  //Check if inactive or active
  if(mode == 0)
  {
    //Inactive so white foreground
    display_set_fg_color(0x00FFFFFF);
  }
  else
  {
    //Active so black foreground
    display_set_fg_color(0x00000000);
  }

  //Display the text
  display_set_font(&font_3);
  display_text(xpos + 24, ypos + 14, "OK");
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_adjust_timebase(void)
{
  //Check if touch within the trace display region
  if((previousxtouch > 2) && (previousxtouch < 720) && (previousytouch > 50) && (previousytouch < 470))
  {
    //Check if touch on the left of the center line
    if(previousxtouch < 358)
    {
      //Check if not already on the highest setting (50S/div)
      if(scopesettings.timeperdiv > 0)
      {
        //Go up in time by taking one of the setting
        scopesettings.timeperdiv--;
      }
    }
    //Check if touch on the right of the center line
    else if(previousxtouch > 362)
    {
      //Check if not already on the lowest setting (10nS/div)
      if(scopesettings.timeperdiv < ((sizeof(time_div_texts) / sizeof(int8 *)) - 1))
      {
        //Go down in time by adding one to the setting
        scopesettings.timeperdiv++;
      }
    }

    //For time per div set with tapping on the screen the direct relation between the time per div and the sample rate is set
    //but only when the scope is running. Otherwise the sample rate of the acquired buffer still is valid.
    if(scopesettings.runstate == 0)
    {
      //Set the sample rate that belongs to the selected time per div setting
      scopesettings.samplerate = time_per_div_sample_rate[scopesettings.timeperdiv];
    }

    //Set the new setting in the FPGA
    fpga_set_sample_rate(scopesettings.samplerate);

    //Show he new setting on the display
    scope_acqusition_settings(0);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_grid(void)
{
  uint32 color;
  register uint32 i;

  //Only draw the grid when something will show (not in the original code)
  if(scopesettings.gridbrightness > 3)
  {
    //Calculate a grey shade based on the grid brightness setting
    color = (scopesettings.gridbrightness * 255) / 100;
    color = (color << 16) | (color << 8) | color;

    //Set the color for drawing
    display_set_fg_color(color);

    //Draw the edge
    display_draw_rect(2, 46, 726, 404);

    //Draw the center lines
    display_draw_horz_line(249,  2, 726);
    display_draw_vert_line(364, 46, 448);

    //Draw the ticks on the x line
    for(i=4;i<726;i+=5)
    {
      display_draw_vert_line(i, 247, 251);
    }

    //Draw the ticks on the y line
    for(i=49;i<448;i+=5)
    {
      display_draw_horz_line(i, 362, 366);
    }

    //Draw the horizontal dots
    for(i=99;i<448;i+=50)
    {
      display_draw_horz_dots(i, 4, 726, 5);
    }

    //Draw the vertical dots
    for(i=14;i<726;i+=50)
    {
      display_draw_vert_dots(i, 49, 448, 5);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_pointers(void)
{
  uint32 position;

  //Draw channel 1 pointer when it is enabled
  if(scopesettings.channel1.enable)
  {
    //Set the colors for drawing
    display_set_fg_color(CHANNEL1_COLOR);
    display_set_bg_color(0x00000000);

    //Select the font for this pointer id
    display_set_font(&font_0);

    //Check if in normal or x-y display mode
    if(scopesettings.xymodedisplay == 0)
    {
      //y position for the channel 1 trace center pointer.
      position = 441 - scopesettings.channel1.traceposition;

      //Limit on the top of the displayable region
      if(position < 46)
      {
        position = 46;
      }
      //Limit on the bottom of the displayable region
      else if(position > 441)
      {
        position = 441;
      }

      //Draw the pointer
      display_left_pointer(2, position, '1');
    }
    else
    {
      //y position for the channel 1 trace center pointer.
      position = 157 + scopesettings.channel1.traceposition;

      //Limit on the left of the active range
      if(position < 166)
      {
        position = 166;
      }
      //Limit on the right of the active range
      else if(position > 548)
      {
        position = 548;
      }

      //Draw the pointer
      display_top_pointer(position, 47, '1');
    }
  }

  //Draw channel 2 pointer when it is enabled
  if(scopesettings.channel2.enable)
  {
    //y position for the channel 2 trace center pointer
    position = 441 - scopesettings.channel2.traceposition;

    //Limit on the top of the displayable region
    if(position < 46)
    {
      position = 46;
    }
    //Limit on the bottom of the displayable region
    else if(position > 441)
    {
      position = 441;
    }

    //Set the colors for drawing
    display_set_fg_color(CHANNEL2_COLOR);
    display_set_bg_color(0x00000000);

    //Select the font for this pointer id
    display_set_font(&font_0);

    //Draw the pointer
    display_left_pointer(2, position, '2');
  }

  //Need to think about trigger position in 200mS - 20mS/div settings. Not sure if they work or need to be done in software
  //The original scope does not show them for 50mS and 20mS/div

  //Draw trigger position and level pointer when in normal display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //x position for the trigger position pointer
    position = scopesettings.triggerhorizontalposition + 2;

    //Limit on the left of the displayable region
    if(position < 2)
    {
      position = 2;
    }
    //Limit on the right of the displayable region
    else if(position > 712)
    {
      position = 712;
    }

    //Set the colors for drawing
    display_set_fg_color(TRIGGER_COLOR);
    display_set_bg_color(0x00000000);

    //Select the font for this pointer id
    display_set_font(&font_3);

    //Draw the pointer
    display_top_pointer(position, 47, 'H');

    //y position for the trigger level pointer
    position = 441 - scopesettings.triggerverticalposition;

    //Limit on the top of the displayable region
    if(position < 46)
    {
      position = 46;
    }
    //Limit on the bottom of the displayable region
    else if(position > 441)
    {
      position = 441;
    }

    //Need to reset the fore ground color
    display_set_fg_color(TRIGGER_COLOR);

    //Draw the pointer
    display_right_pointer(707, position, 'T');
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_time_cursors(void)
{
  //Only draw the lines when enabled
  if(scopesettings.timecursorsenable)
  {
    //Set the color for the dashed lines
    display_set_fg_color(CURSORS_COLOR);

    //Draw the lines
    display_draw_vert_dashes(scopesettings.timecursor1position, 48, 448, 3, 3);
    display_draw_vert_dashes(scopesettings.timecursor2position, 48, 448, 3, 3);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_volt_cursors(void)
{
  //Only draw the lines when enabled
  if(scopesettings.voltcursorsenable)
  {
    //Set the color for the dashed lines
    display_set_fg_color(CURSORS_COLOR);

    //Draw the lines
    display_draw_horz_dashes(scopesettings.voltcursor1position, 5, 726, 3, 3);
    display_draw_horz_dashes(scopesettings.voltcursor2position, 5, 726, 3, 3);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_calculate_trigger_vertical_position()
{
  PCHANNELSETTINGS settings;

  //Select the channel based on the current trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  int32 position;

  //Center the trigger level around 0 point
  position = scopesettings.triggerlevel - 128;

  //Adjust it for the current volt per div setting
  position = (position * signal_adjusters[settings->displayvoltperdiv]) >> 22;

  //Add the trace center to it
  position = settings->traceposition + position;

  //Limit to extremes
  if(position < 0)
  {
    position = 0;
  }
  else if(position > 401)
  {
    position = 401;
  }

  //Set as new position
  scopesettings.triggerverticalposition = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquire_trace_data(void)
{
  uint32 data;

  //Check if running and not in a trace or cursor displacement state
  if((scopesettings.runstate == 0) && (touchstate == 0))
  {
    //Set the trigger level
    fpga_set_trigger_level();

    //Write the time base setting to the FPGA
    fpga_set_time_base(scopesettings.timeperdiv);

    //Sampling with trigger circuit enabled
    scopesettings.samplemode = 1;

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

    //Check if cut short with touch
    if(havetouch)
    {
      //If so skip the rest
      return;
    }

    //Check if in single mode
    if(scopesettings.triggermode == 1)
    {
      //Switch to stopped
      scopesettings.runstate = 1;

      //Show this on the screen
      scope_run_stop_text();
    }

    //Get trigger point information
    //Later on used to send to the FPGA with command 0x1F
    data = fpga_prepare_for_transfer();

    //Just using the same calculation for every setting solves the frequency calculation error
    //The signal representation still is correct and the trigger point seems to be more valid also
    //The original uses time base dependent processing here, but this seems to do the trick on all ranges
    //The software needs to verify the trigger to make it more stable
    if(data < 750)
    {
      //Less then 750 make it bigger
      data = data + 3345;
    }
    else
    {
      //More then 750 make it smaller
      data = data - 750;
    }

    //Only need a single count variable for both channels, since they run on the same sample rate
    //This can be changed to a global define
    scopesettings.nofsamples  = 1500;
    scopesettings.samplecount = 3000;

    //Check if channel 1 is enabled
    if(scopesettings.channel1.enable)
    {
      //Get the samples for channel 1
      fpga_read_sample_data(&scopesettings.channel1, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && (scopesettings.triggerchannel == 0))
      {
        //Use the channel 1 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel1.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }

    //Check if channel 2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Get the samples for channel 2
      fpga_read_sample_data(&scopesettings.channel2, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && scopesettings.triggerchannel)
      {
        //Use the channel 2 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel2.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }


    //Need to improve on this for a more stable displaying. On the low sample rate settings it seems to flip between two positions.
    //Determine the trigger position based on the selected trigger channel
    scope_process_trigger(scopesettings.nofsamples);

  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_process_trigger(uint32 count)
{
  uint8  *buffer;
  uint32  index;
  uint32  level = scopesettings.triggerlevel;
  uint32  sample1;
  uint32  sample2;

  //Select the trace buffer to process based on the trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    //Channel 1 buffer
    buffer = (uint8 *)channel1tracebuffer;
  }
  else
  {
    //Channel 2 buffer
    buffer = (uint8 *)channel2tracebuffer;
  }

  disp_have_trigger = 0;

  //Set a starting point for checking on trigger
  //Count is half a sample buffer!!
  index = count - 10;
  count = 20;

  //Need a better check here, maybe over a wider range of samples

  while(count--)
  {
    sample1 = buffer[index];
    sample2 = buffer[index + 1];

    if(((scopesettings.triggeredge == 0) && (sample1 < level) && (sample2 >= level)) ||
       ((scopesettings.triggeredge == 1) && (sample1 >= level) && (sample2 < level)))
    {
      //Set the current index as trigger point
      disp_trigger_index = index;

      //Signal trigger has been found
      disp_have_trigger = 1;

      //Done with checking
      break;
    }

    //Select next sample to check
    index++;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_do_baseline_calibration(void)
{
  uint32 flag = 1;
  uint32 voltperdiv;

  //Disable the trigger circuit
  scopesettings.samplemode = 0;

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = 3000;
  scopesettings.nofsamples  = 1500;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Clear the compensation values before doing the calibration
  calibrationsettings.adc1compensation = 0;
  calibrationsettings.adc2compensation = 0;

  //Setup for channel 1 calibration
  calibrationsettings.voltperdivcommand = 0x33;
  calibrationsettings.offsetcommand     = 0x32;
  calibrationsettings.adc1command       = 0x20;
  calibrationsettings.adc2command       = 0x21;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel1tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel1.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel1.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel1.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Setup for channel 2 calibration
  calibrationsettings.voltperdivcommand = 0x36;
  calibrationsettings.offsetcommand     = 0x35;
  calibrationsettings.adc1command       = 0x22;
  calibrationsettings.adc2command       = 0x23;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel2tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel2.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel2.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel2.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Load the normal operation settings back into the FPGA
  fpga_set_channel_voltperdiv(&scopesettings.channel1);
  fpga_set_channel_offset(&scopesettings.channel1);
  fpga_set_channel_voltperdiv(&scopesettings.channel2);
  fpga_set_channel_offset(&scopesettings.channel2);
  fpga_set_sample_rate(scopesettings.samplerate);

  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

#define HIGH_DC_OFFSET   500
#define LOW_DC_OFFSET   1200

uint32 scope_do_channel_calibration(void)
{
  uint32 flag = 1;
  int32  samplerate;
  int32  voltperdiv;
  int32  highaverage;
  int32  lowaverage;
  int32  dcoffsetstep;
  int32  compensationsum = 0;

  //Calibrate for the hardware sensitivity settings
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start with the first set of averages
    samplerateindex = 0;

    //Do the measurements on two sample rates. 200MSa/s and 50KSa/s
    for(samplerate=0;samplerate<18;samplerate+=11)
    {
      //Set the selected sample rate
      fpga_set_sample_rate(samplerate);

      //Set the matching time base
      fpga_set_time_base(sample_rate_time_per_div[samplerate]);

      //Set the high DC offset in the FPGA (Lower value returns higher ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = HIGH_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done or touch panel active
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as one reading here. Only use ADC1 data
      highaverage = calibrationsettings.adc1rawaverage;

      //Set the low DC offset in the FPGA (Higher value returns lower ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = LOW_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done or touch panel active
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as another reading here. Only use ADC1 data
      lowaverage = calibrationsettings.adc1rawaverage;

      //Calculate the DC offset step for a single ADC bit change for this volt/div setting
      //Low DC offset minus high DC offset (1200 - 500) = 700. Scaled up for fixed point calculation ==> 700 << 20 = 734003200
      dcoffsetstep = 734003200 / (highaverage - lowaverage);

      //Calculate the average DC offset settings for both the low to center as the high to center readings
      highaverage = HIGH_DC_OFFSET + (((highaverage - 128) * dcoffsetstep) >> 20);
      lowaverage  = LOW_DC_OFFSET  - (((128 - lowaverage) * dcoffsetstep) >> 20);

      //Set the result for this sample rate and volt per division setting
      samplerateaverage[samplerateindex][voltperdiv] = (highaverage + lowaverage) / 2;

      //Save the dc offset step for final calibration after compensation data has been determined
      sampleratedcoffsetstep[samplerateindex][voltperdiv] = dcoffsetstep;

      //Select the next set of sample indexes
      samplerateindex++;
    }
  }

  //Set the sample rate to 2MSa/s
  fpga_set_sample_rate(6);

  //Set the matching time base
  fpga_set_time_base(sample_rate_time_per_div[6]);

  //Average and test the results on a third sample rate
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the average of the two measurements as the DC offset to work with
    calibrationsettings.dc_calibration_offset[voltperdiv] = (samplerateaverage[0][voltperdiv] + samplerateaverage[1][voltperdiv]) / 2;

    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Set the new DC channel offset in the FPGA
    fpga_set_channel_offset(&calibrationsettings);

    //Wait 50ms before sampling
    timer0_delay(50);

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(&calibrationsettings, 100);

    //Check if the average reading is outside allowed range
    if((calibrationsettings.adc1rawaverage < 125) || (calibrationsettings.adc1rawaverage > 131))
    {
      //When deviation is more then 3, signal it as a failure
      flag = 0;
    }

    //Sum the ADC differences
    compensationsum += (calibrationsettings.adc2rawaverage - calibrationsettings.adc1rawaverage);
  }

  //Calculate the average of the ADC difference
  compensationsum /= 6;

  //Split the difference on the two ADC's
  calibrationsettings.adc1compensation = compensationsum / 2;
  calibrationsettings.adc2compensation = -1 * (compensationsum - calibrationsettings.adc1compensation);

  //Check if the found result is within limits
  if((compensationsum < -20) || (compensationsum > 20))
  {
    //Not so clear the flag
    flag = 0;
  }

  //Adjust the center point DC offsets with the found compensation values
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //The DC offset is based on the pre compensated ADC1 reading, so need to adjust with the average DC offset step times the ADC1 compensation value
    calibrationsettings.dc_calibration_offset[voltperdiv] = (int16)calibrationsettings.dc_calibration_offset[voltperdiv] + (((int32)calibrationsettings.adc1compensation * (((int32)sampleratedcoffsetstep[0][voltperdiv] + (int32)sampleratedcoffsetstep[1][voltperdiv]) / 2)) >> 20);
  }

  //Return the result of the tests. True if all tests passed
  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_do_50_percent_trigger_setup(void)
{
  //Check which channel is the active trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    //Use the channel 1 center value
    scopesettings.triggerlevel = scopesettings.channel1.center;
  }
  else
  {
    //Use the channel 2 center value
    scopesettings.triggerlevel = scopesettings.channel2.center;
  }

  //Set the trigger vertical position position to match the new trigger level
  scope_calculate_trigger_vertical_position();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_do_auto_setup(void)
{
  PCHANNELSETTINGS settings;

  int32  screentime;
  int32  timeperdiv;
  int32  samplerate;

  uint32 dochannel1 = scopesettings.channel1.enable;
  uint32 dochannel2 = scopesettings.channel2.enable;

  //No need to do auto setup if no channel is enabled
  if((dochannel1 == 0) && (dochannel2 == 0))
  {
    return;
  }

  //Disable the trigger circuit
  scopesettings.samplemode = 0;

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = 3000;
  scopesettings.nofsamples  = 1500;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Setup channel 1 if enabled
  if(dochannel1)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel1.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel1);
  }

  //Setup channel 2 if enabled
  if(dochannel2)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel2.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel2);
  }

  //Select the channel to work with
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  //Wait 50ms to allow the relays to settle
  timer0_delay(50);

  //Do the measurements on four sample rates. 200MSa/s, 2MSa/s, 20KSa/s and 1000Sa/s
  for(samplerate=0;samplerate<4;samplerate++)
  {
    //Set the selected sample rate
    fpga_set_sample_rate(samplerate_for_autosetup[samplerate]);

    //Set the matching time base
    fpga_set_time_base(sample_rate_time_per_div[samplerate_for_autosetup[samplerate]]);

    //Start the conversion and wait until done or touch panel active
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(settings, 100);

    //Check if there is a frequency reading and break the loop if so
    if(settings->frequencyvalid)
    {
      break;
    }
  }

  //When there is a frequency reading determine the needed time per division setting
  if(settings->frequencyvalid)
  {
    //Can't use the frequency here since it is based on the scopesettings.samplerate variable, which is not used here
    //Calculate the time in nanoseconds for getting three periods on the screen
    screentime = (float)settings->periodtime * sample_time_converters[samplerate_for_autosetup[samplerate]];

    //Match the found time to the nearest time per division setting
    for(timeperdiv=0;timeperdiv<24;timeperdiv++)
    {
      //When the found time is higher than the selected time per division quit the search
      if(screentime > time_per_div_matching[timeperdiv])
      {
        break;
      }
    }

    //Check if not on the first setting
    if(timeperdiv)
    {
      //If so take one of to get to the right one to use. Also ensures not selecting a non existing setting if none found
      timeperdiv--;
    }

    //Select the found time per division
    scopesettings.timeperdiv = timeperdiv;
    scopesettings.samplerate = time_per_div_sample_rate[timeperdiv];
  }
  else
  {
    //Set a default sample rate and time per division setting
    scopesettings.timeperdiv = 12;
    scopesettings.samplerate = time_per_div_sample_rate[12];
  }

  //Range the input sensitivity on the enabled channels
  //Check on which bottom check level needs to be used
  //When both channels are enabled and in normal display mode use separate sections of the screen for each channel.
  if(dochannel1 && dochannel2 && (scopesettings.xymodedisplay == 0))
  {
    //Both channels enabled then use a lower level. Smaller section of the display available per channel so lower value
    scopesettings.channel1.maxscreenspace = 1900;
    scopesettings.channel2.maxscreenspace = 1900;

    //Give both traces it's own location on screen
    scopesettings.channel1.traceposition = 300;
    scopesettings.channel2.traceposition = 100;

    //Get the sample data for the not trigger source channel when a valid frequency has been determined
    if(settings->frequencyvalid)
    {
      //Check which channel is the trigger source
      if(scopesettings.triggerchannel == 0)
      {
        //Channel 1 is trigger source so get channel 2 data
        fpga_read_sample_data(&scopesettings.channel2, 100);
      }
      else
      {
        //Channel 2 is trigger source so get channel 1 data
        fpga_read_sample_data(&scopesettings.channel1, 100);
      }
    }
  }
  else
  {
    //Only one channel enabled then more screen space available for it so higher value
    scopesettings.channel1.maxscreenspace = 3900;
    scopesettings.channel2.maxscreenspace = 3900;

    //Used channel will be set on the middle of the display
    scopesettings.channel1.traceposition = 200;
    scopesettings.channel2.traceposition = 200;
  }

  //Set the new sample rate in the FPGA
  fpga_set_sample_rate(scopesettings.samplerate);

  //Show the new settings
  scope_acqusition_settings(0);

  //Check the channels on valid voltage readings with the already done conversion, but only when valid frequency
  if(settings->frequencyvalid)
  {
    //Check if channel 1 is enabled and check its range if so
    if(dochannel1)
    {
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 is enabled and check its range if so
    if(dochannel2)
    {
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }
  }

  //If one or either channel not in range another conversion is needed
  if(dochannel1 || dochannel2)
  {
    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel1.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel2.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel2);
    }

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start the conversion and wait until done
    fpga_do_conversion();

    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel1, 100);

      //Check the range again
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel2, 100);

      //Check the range again
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }

    //If one or either channel not in range another conversion is needed
    if(dochannel1 || dochannel2)
    {
      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel1.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel2.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel2);
      }

      //Wait 50ms to allow the relays to settle
      timer0_delay(50);

      //Start the conversion and wait until done
      fpga_do_conversion();

      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel1, 100);

        //Check the range again
        dochannel1 = scope_check_channel_range(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel2, 100);

        //Check the range again
        dochannel2 = scope_check_channel_range(&scopesettings.channel2);
      }

      //If one or either channel not in range the least sensitive setting needs to be used
      if(dochannel1 || dochannel2)
      {
        //Check if channel 1 still not done
        if(dochannel1)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel1.samplevoltperdiv = 0;
        }

        //Check if channel 2 still not done
        if(dochannel2)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel2.samplevoltperdiv = 0;
        }
      }
    }
  }

  //Check if channel 1 is enabled and set the new settings if so
  if(scopesettings.channel1.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel1);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel1.displayvoltperdiv = scopesettings.channel1.samplevoltperdiv;
    
    //Update the display
    scope_channel_settings(&scopesettings.channel1, 0);
  }

  //Check if channel 2 is enabled and set the new settings if so
  if(scopesettings.channel2.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel2);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel2.displayvoltperdiv = scopesettings.channel2.samplevoltperdiv;
    
    //Update the display
    scope_channel_settings(&scopesettings.channel2, 0);
  }

  //Adjust the trigger level to 50% setting
  scope_do_50_percent_trigger_setup();
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_check_channel_range(PCHANNELSETTINGS settings)
{
  uint32 screenpixels;
  uint32 range;
  uint32 notdone = 0;

  //Convert the peak peak reading to screen pixels
  screenpixels = (settings->peakpeak * signal_adjusters[settings->samplevoltperdiv]) >> 22;

  //Check if there is any signal at all to avoid divide by zero
  if(screenpixels)
  {
    //Calculate a fixed point range to determine the volts per division setting, based on the available screen space
    range = settings->maxscreenspace / screenpixels;

    //Check if signal is at least 10 times smaller then the max range
    if(range >= 100)
    {
      //If so go three sensitivity settings up. (e.g. 500mV/div ==> 50mV/div)
      settings->samplevoltperdiv = 3;
    }
    //Else check if signal is at least 5 times smaller then the max range
    else if(range >= 50)
    {
      //If so go two sensitivity settings up. (e.g. 500mV/div ==> 100mV/div)
      settings->samplevoltperdiv += 2;
    }
    //Else check if signal is at least 2.5 times smaller then the max range
    else if(range >= 25)
    {
      //If so go one sensitivity setting up. (e.g. 500mV/div ==> 200mV/div)
      settings->samplevoltperdiv += 1;
    }
    //Else check if signal is bigger then the max range
    else if(range < 10)
    {
      //Out of range so not done
      notdone = 1;
    }

    //Check if volt per division not out of range
    if(settings->samplevoltperdiv > 6)
    {
      //Keep it on max if so
      settings->samplevoltperdiv = 6;
    }
  }
  else
  {
    //When there is no signal use the most sensitive setting
    settings->samplevoltperdiv = 6;
  }

  return(notdone);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_trace_data(void)
{
  //See if it is possible to rework this to fixed point. A 32 bit mantissa is not accurate enough though

  //On the scope the last pixel interleaving is not working properly. Don't know why.

  //Check out the sin x/x interpolation downloaded from keysight (Sine wave reproduction using Sinc interpolation - v.1.0.py)

  //This needs more code to skip when stopped and no changes are made. It seems to be ok without this, but what is not needed is not needed
  //In the display trace routine the display only needs to be redrawn when certain conditions apply
  //  The user changes the sample rate or the time per div setting. Might need to block the setting of the sample rate when stopped because that makes no change to the current sample buffer.
  //  The user moves pointers around. Trace up and down should work, and trigger left and right. Changing the trigger level should not do anything, or even be disabled



  //Need to compensate for the position being on the left side of the pointer
  uint32 triggerposition = scopesettings.triggerhorizontalposition + 7;

  //Check if a trigger position has been found
  if(disp_have_trigger == 0)
  {
    //When not use the center of the sample buffer
    disp_trigger_index = scopesettings.samplecount / 2;
  }

  //Make sure the two settings are in range of the tables!!!!
  //SKipp displaying if not????


  //The amount of x positions needed per sample is based on the number of pixels per division, the set time per division and the sample rate.
  disp_xpos_per_sample = (50.0 * frequency_per_div[scopesettings.timeperdiv]) / sample_rate[scopesettings.samplerate];

  //This gives the step size for going through the samples. Is also the linear interleaving step for the y direction
  disp_sample_step = 1.0 / disp_xpos_per_sample;

  //The displayable x range is based on the number of samples and the number of x positions needed per sample
  //Halved to allow trigger position to be in the center
  double xrange = (scopesettings.samplecount * disp_xpos_per_sample) / 2.0;

  //x range needs to be at least 1 pixel
  if(xrange < 1.0)
  {
    xrange = 1.0;
  }
  else if(xrange > 725.0)
  {
    //Limit on max screen pixels to avoid disp_xend becoming 0x80000000 due to overflow
    xrange = 725.0;
  }

  //Calculate the start and end x coordinates
  disp_xstart = triggerposition - xrange;
  disp_xend = triggerposition + xrange;

  //Limit on actual start of trace display
  if(disp_xstart < 3)
  {
    disp_xstart = 3;
  }

  //And limit on the actual end of the trace display
  if(disp_xend > 725)
  {
    disp_xend = 725;
  }

  //Determine first sample to use based on a full screen worth of samples and the trigger position in relation to the number of pixels on the screen
  disp_first_sample = disp_trigger_index - (((725.0 / disp_xpos_per_sample) * triggerposition) / 725.0) - 1;

  //If below the first sample limit it on the first sample
  if(disp_first_sample < 0)
  {
    disp_first_sample = 0;
  }

  //This makes sure no reading outside the buffer can occur
  if(disp_sample_step > ((scopesettings.samplecount) / 2))
  {
    disp_sample_step = (scopesettings.samplecount) / 2;
  }




  //If samplestep > 1 might be an idea to draw the in between samples on the same x position to avoid aliasing
  //If sample step < 1 then skip drawing on x positions. The draw line function does the linear interpolation



  //Use a separate buffer to clear the screen
  display_set_screen_buffer(displaybuffer1);

  //Clear the trace portion of the screen
  display_set_fg_color(0x00000000);
  display_fill_rect(2, 46, 728, 434);

  //Check if not in waveform view mode with grid disabled
  if((scopesettings.waveviewmode == 0) || scopesettings.gridenable == 0)
  {
    //Draw the grid lines and dots based on the grid brightness setting
    scope_draw_grid();
  }

  //Check if scope is in normal display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //The calculations done above need to go here??


    //Check if channel1 is enabled
    if(scopesettings.channel1.enable)
    {
      //This can be reduced in parameters by using the channel structure as input and add the color as item in the structure

      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel1);
    }

    //Check if channel2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel2);
    }

    //Displaying of FFT needs to be added here.

  }
  else
  {
    //Scope set to x y display mode
    //Set x-y mode display trace color
    display_set_fg_color(XYMODE_COLOR);

    uint32 index = disp_trigger_index - 315;
    uint32 last = index + 730;

    //Need two samples per channel
    uint32 x1,x2;
    uint32 y1,y2;

    //Get the samples for the first point
    x1 = scope_get_x_sample(&scopesettings.channel1, index);
    y1 = scope_get_y_sample(&scopesettings.channel2, index);
    
    //Handle all the needed samples
    for(;index<last;index++)
    {
      //Get the samples for the next point
      x2 = scope_get_x_sample(&scopesettings.channel1, index);
      y2 = scope_get_y_sample(&scopesettings.channel2, index);

      //Draw the line between these two points
      display_draw_line(x1, y1, x2, y2);

      //Swap the points for drawing the next part
      x1 = x2;
      y1 = y2;
    }
  }

  //Draw the cursors with their measurement displays
  scope_draw_time_cursors();
  scope_draw_volt_cursors();
  scope_display_cursor_measurements();

  //Draw the signal center, trigger level and trigger position pointers
  scope_draw_pointers();

  //Show the enabled measurements on the screen
  scope_display_measurements();    //Still needs implementing

  //Check if in waveform view
  if(scopesettings.waveviewmode)
  {
    //Display the file name
    //Use white text and font_0
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_0);
    display_text(550, 48, viewfilename);
  }
  
  //Copy it to the actual screen buffer
  display_set_source_buffer(displaybuffer1);
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(2, 46, 728, 434);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_x_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> 22;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }

  //Limit the sample on max displayable
  if(sample > 401)
  {
    sample = 401;
  }

  //The x center position has an extra offset compared to the y trace position
  return(sample + 165);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_y_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> 22;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }

  //Limit the sample on max displayable
  if(sample > 401)
  {
    sample = 401;
  }

  //Display y coordinates are inverted to signal orientation
  return(448 - sample);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_channel_trace(PCHANNELSETTINGS settings)
{
  double inputindex;

  register int32 previousindex;
  register int32 currentindex;
  register int32 sample1 = 0;
  register int32 sample2 = 0;
  register uint32 lastx = disp_xstart;
  register uint32 xpos = disp_xstart + 1;

  register PDISPLAYPOINTS tracepoints = settings->tracepoints;

  //Set the trace color for the current channel
  display_set_fg_color(settings->color);

  //Get the processed sample
  sample1 = scope_get_y_sample(settings, disp_first_sample);

  //Store the first sample in the trace points buffer
  tracepoints->x = lastx;
  tracepoints->y = sample1;
  tracepoints++;

  //Start with one tracepoint
  settings->noftracepoints = 1;

  //Step to the next input index
  inputindex = disp_first_sample + disp_sample_step;

  //The previous index is the index of the first sample
  previousindex = disp_first_sample;

  //Process the sample data to screen data
  for(; xpos < disp_xend; inputindex += disp_sample_step, xpos++)
  {
    //Get the current integer index into the sample buffer
    currentindex = inputindex;

    //Check if linear approximation needs to be done. (Only when step < 1) pixels are skipped if so.
    if(currentindex != previousindex)
    {
      //Set new previous index
      previousindex = currentindex;

      //Get the processed sample
      sample2 = scope_get_y_sample(settings, currentindex);

      //Store the second sample in the screen buffer
      tracepoints->x = xpos;
      tracepoints->y = sample2;
      tracepoints++;

      //One more tracepoint
      settings->noftracepoints++;

      //Need to draw a line here
      display_draw_line(lastx, sample1, xpos, sample2);

      sample1 = sample2;

      lastx = xpos;
    }
  }

  //When step less then 1 the last pixel needs to be interpolated between current sample and next sample.
  if(disp_sample_step < 1.0)
  {
    //Calculate the scaler for the last y value based on the x distance from the last drawn position to the end of the screen
    //divided by the x distance it takes to where the next position should be drawn (Number of x steps per sample)
    double scaler =  (725.0 - lastx) / disp_xpos_per_sample;    // (1 / samplestep);

    //Get the processed sample
    sample2 = scope_get_y_sample(settings, inputindex);

    sample2 = sample1 + ((double)((double)sample2 - (double)sample1) / scaler);

    //Store the last sample in the screen buffer
    tracepoints->x = xpos;
    tracepoints->y = sample2;
    tracepoints++;

    //One more tracepoint
    settings->noftracepoints++;

    //Draw the last line
    display_draw_line(lastx, sample1, xpos, sample2);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_cursor_measurements(void)
{
  uint32 height = 5;
  uint32 ch1ypos = 52;
  uint32 ch2ypos = 52;
  uint32 delta;
  char   displaytext[10];

  //Check if need to do anything here
  if(scopesettings.timecursorsenable || (scopesettings.voltcursorsenable && (scopesettings.channel1.enable || scopesettings.channel2.enable)))
  {
    //Check if time cursor is enabled
    if(scopesettings.timecursorsenable)
    {
      //Add height for two text lines
      height += 32;

      //Shift the voltage text positions down
      ch1ypos += 32;
      ch2ypos += 32;
    }

    //Check if volt cursor is enabled
    if(scopesettings.voltcursorsenable)
    {
      //Check if channel 1 is enabled
      if(scopesettings.channel1.enable)
      {
        //Add height for one text line
        height += 16;

        //Shift the channel 2 voltage text down
        ch2ypos += 16;
      }

      //Check if channel 2 is enabled
      if(scopesettings.channel2.enable)
      {
        //Add height for one text line
        height += 16;
      }
    }

    //Set gray background for the cursor measurements
    display_set_fg_color(0x00404040);

    //Draw rounded rectangle depending on what is enabled.
    display_fill_rounded_rect(5, 49, 102, height, 2);

    //Use white text and font_0
    display_set_fg_color(0x00FFFFFF);
    display_set_font(&font_0);

    //Check if time cursor is enabled
    if(scopesettings.timecursorsenable)
    {
      //Time texts are always on the top two lines

      //Get the time delta based on the cursor positions
      delta = scopesettings.timecursor2position - scopesettings.timecursor1position;

      //Get the time calculation data for this time base setting.
      PTIMECALCDATA tcd = (PTIMECALCDATA)&time_calc_data[scopesettings.timeperdiv];

      //For the time multiply with the scaling factor and display based on the time scale
      delta *= tcd->mul_factor;

      //Format the time for displaying
      scope_print_value(displaytext, delta, tcd->time_scale, "T ", "S");
      display_text(10, 52, displaytext);

      //Calculate the frequency for this time. Need to adjust for it to stay within 32 bits
      delta /= 10;
      delta = 1000000000 / delta;

      //Format the frequency for displaying
      scope_print_value(displaytext, delta, tcd->freq_scale, "F ", "Hz");
      display_text(10, 68, displaytext);
    }

    //Check if volt cursor is enabled
    if(scopesettings.voltcursorsenable)
    {
      PVOLTCALCDATA vcd;
      uint32        volts;

      //Get the volts delta based on the cursor positions
      delta = scopesettings.voltcursor2position - scopesettings.voltcursor1position;

      //Check if channel 1 is enabled
      if(scopesettings.channel1.enable)
      {
        //Calculate the voltage based on the channel 1 settings
        vcd = (PVOLTCALCDATA)&volt_calc_data[scopesettings.channel1.magnification][scopesettings.channel1.displayvoltperdiv];

        //Multiply with the scaling factor for the channel 1 settings
        volts = delta * vcd->mul_factor;

        //Channel 1 text has a variable position
        //Format the voltage for displaying
        scope_print_value(displaytext, volts, vcd->volt_scale, "V1 ", "V");
        display_text(10, ch1ypos, displaytext);
      }

      //Check if channel 2 is enabled
      if(scopesettings.channel2.enable)
      {
        //Calculate the voltage based on the channel 2 settings
        vcd = (PVOLTCALCDATA)&volt_calc_data[scopesettings.channel2.magnification][scopesettings.channel2.displayvoltperdiv];

        //Multiply with the scaling factor for the channel 2 settings
        volts = delta * vcd->mul_factor;

        //Channel 2 text has a variable position
        //Format the voltage for displaying
        scope_print_value(displaytext, volts, vcd->volt_scale, "V2 ", "V");
        display_text(10, ch2ypos, displaytext);
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_measurements(void)
{
  char   displaytext[10];

  //Use white text and font_0
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_0);

  //Need a scale table based on set sample rate


  if(scopesettings.channel1.enable && scopesettings.channel1.frequencyvalid)
  {
    //Format the frequency for displaying
    scope_print_value(displaytext, scopesettings.channel1.frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "F1 ", "Hz");
    display_text(10, 454, displaytext);
  }

  if(scopesettings.channel2.enable && scopesettings.channel2.frequencyvalid)
  {
    //Format the frequency for displaying
    scope_print_value(displaytext, scopesettings.channel2.frequency, freq_calc_data[scopesettings.samplerate].freq_scale, "F2 ", "Hz");
    display_text(380, 454, displaytext);
  }

}


//----------------------------------------------------------------------------------------------------------------------------------
//Simple non optimized function for string copy that returns the position of the terminator
//----------------------------------------------------------------------------------------------------------------------------------

char *strcpy(char *dst, const char *src)
{
  while(*src)
  {
    *dst++ = *src++;
  }

  //Terminate the copy
  *dst = 0;

  return(dst);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_print_value(char *buffer, uint32 value, uint32 scale, char *header, char *sign)
{
  //Copy the header into the string buffer
  buffer = strcpy(buffer, header);

  //Need to find the magnitude scale for the input
  //The calculations are based on fixed point
  while(value >= 100000)
  {
    //Skip to the next magnitude
    scale++;

    //Bring the value in range
    value /= 1000;
  }

  //Format the remainder for displaying. Only 3 digits are allowed to be displayed
  if(value < 1000)
  {
    //Less then 1000 means x.yy
    buffer = scope_print_decimal(buffer, value, 2);
  }
  else if(value < 10000)
  {
    //More then 1000 but less then 10000 means xx.y
    value /= 10;
    buffer = scope_print_decimal(buffer, value, 1);
  }
  else
  {
    //More then 10000 and less then 100000 means xxx
    value /= 100;
    buffer = scope_print_decimal(buffer, value, 0);
  }

  //Make sure scale is not out of range
  if(scale > 7)
  {
    scale = 7;
  }

  //Add the magnitude scaler
  buffer = strcpy(buffer, magnitude_scaler[scale]);

  //Add the type of measurement sign
  strcpy(buffer, sign);
}

//----------------------------------------------------------------------------------------------------------------------------------

char *scope_print_decimal(char *buffer, uint32 value, uint32 decimals)
{
  char    b[12];
  uint32  i = 12;   //Start beyond the array since the index is pre decremented
  uint32  s;

  //For value 0 no need to do the work
  if(value == 0)
  {
    //Value is zero so just set 0 character
    b[--i] = '0';
  }
  else
  {
    //Process the digits
    while(value)
    {
      //Set current digit to decreased index
      b[--i] = (value % 10) + '0';

      //Check if decimal point needs to be placed
      if(i == 12 - decimals)
      {
        //If so put it in
        b[--i] = '.';
      }

      //Take of the current digit
      value /= 10;
    }
  }

  //Determine the size of the string
  s = 12 - i;

  //Copy to the buffer
  memcpy(buffer, &b[i], s);

  //terminate the string
  buffer[s] = 0;

  //Return the position of the terminator to allow appending
  return(&buffer[s]);
}

//----------------------------------------------------------------------------------------------------------------------------------
// File display functions
//----------------------------------------------------------------------------------------------------------------------------------
//Simplest setup here is to put all important data in a struct and make it such that a pointer is used to point to the active one
//This way no memory needs to be copied
//Needs a bit of a re write but might improve things a bit
//Depends on how the pointer setup effects the main code

void scope_save_setup(PSCOPESETTINGS settings)
{
  //For now just copy the settings to the given struct
  memcpy(settings, &scopesettings, sizeof(SCOPESETTINGS));
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_restore_setup(PSCOPESETTINGS settings)
{
  //For now just copy the settings from the given struct
  memcpy(&scopesettings, settings, sizeof(SCOPESETTINGS));
}

//----------------------------------------------------------------------------------------------------------------------------------
//These functions are for handling the settings to and from file

void scope_prepare_setup_for_file(void)
{
  uint32 *ptr = viewfilesetupdata;
  uint32 index = 0;
  uint32 channel;
  uint32 measurement;
  uint32 checksum = 0;

  //Best to clear the buffer first since not all bytes are used
  memset((uint8 *)viewfilesetupdata, 0, sizeof(viewfilesetupdata));

  //Put in a version number for the waveform view file
  ptr[1] = WAVEFORM_FILE_VERSION;
  
  //Leave space for file version and checksum data
  index = CHANNEL1_SETTING_OFFSET;
  
  //Copy the needed channel 1 settings and measurements
  ptr[index++] = scopesettings.channel1.enable;
  ptr[index++] = scopesettings.channel1.displayvoltperdiv;
  ptr[index++] = scopesettings.channel1.samplevoltperdiv;
  ptr[index++] = scopesettings.channel1.fftenable;
  ptr[index++] = scopesettings.channel1.coupling;
  ptr[index++] = scopesettings.channel1.magnification;
  ptr[index++] = scopesettings.channel1.traceposition;
  ptr[index++] = scopesettings.channel1.min;
  ptr[index++] = scopesettings.channel1.max;
  ptr[index++] = scopesettings.channel1.average;
  ptr[index++] = scopesettings.channel1.center;
  ptr[index++] = scopesettings.channel1.peakpeak;
  ptr[index++] = scopesettings.channel1.frequencyvalid;
  ptr[index++] = scopesettings.channel1.frequency;
  ptr[index++] = scopesettings.channel1.lowtime;
  ptr[index++] = scopesettings.channel1.hightime;
  ptr[index++] = scopesettings.channel1.periodtime;

  //Leave some space for channel 1 settings changes
  index = CHANNEL2_SETTING_OFFSET;
  
  //Copy the needed channel 2 settings and measurements
  ptr[index++] = scopesettings.channel2.enable;
  ptr[index++] = scopesettings.channel2.displayvoltperdiv;
  ptr[index++] = scopesettings.channel2.samplevoltperdiv;
  ptr[index++] = scopesettings.channel2.fftenable;
  ptr[index++] = scopesettings.channel2.coupling;
  ptr[index++] = scopesettings.channel2.magnification;
  ptr[index++] = scopesettings.channel2.traceposition;
  ptr[index++] = scopesettings.channel2.min;
  ptr[index++] = scopesettings.channel2.max;
  ptr[index++] = scopesettings.channel2.average;
  ptr[index++] = scopesettings.channel2.center;
  ptr[index++] = scopesettings.channel2.peakpeak;
  ptr[index++] = scopesettings.channel2.frequencyvalid;
  ptr[index++] = scopesettings.channel2.frequency;
  ptr[index++] = scopesettings.channel2.lowtime;
  ptr[index++] = scopesettings.channel2.hightime;
  ptr[index++] = scopesettings.channel2.periodtime;

  //Leave some space for channel 2 settings changes
  index = TRIGGER_SETTING_OFFSET;
  
  //Copy the needed scope trigger settings
  ptr[index++] = scopesettings.timeperdiv;
  ptr[index++] = scopesettings.samplerate;
  ptr[index++] = scopesettings.triggermode;
  ptr[index++] = scopesettings.triggeredge;
  ptr[index++] = scopesettings.triggerchannel;
  ptr[index++] = scopesettings.triggerlevel;
  ptr[index++] = scopesettings.triggerhorizontalposition;
  ptr[index++] = scopesettings.triggerverticalposition;
  ptr[index++] = disp_have_trigger;
  ptr[index++] = disp_trigger_index;

  //Leave some space for trigger information changes
  index = OTHER_SETTING_OFFSET;
  
  //Copy the needed other scope settings
  ptr[index++] = scopesettings.movespeed;
  ptr[index++] = scopesettings.rightmenustate;
  ptr[index++] = scopesettings.screenbrightness;
  ptr[index++] = scopesettings.gridbrightness;
  ptr[index++] = scopesettings.alwaystrigger50;
  ptr[index++] = scopesettings.xymodedisplay;
  ptr[index++] = scopesettings.confirmationmode;

  //Leave some space for other scope settings changes
  index = CURSOR_SETTING_OFFSET;
  
  //Copy the cursor settings
  ptr[index++] = scopesettings.timecursorsenable;
  ptr[index++] = scopesettings.voltcursorsenable;
  ptr[index++] = scopesettings.timecursor1position;
  ptr[index++] = scopesettings.timecursor2position;
  ptr[index++] = scopesettings.voltcursor1position;
  ptr[index++] = scopesettings.voltcursor2position;
  
  //Leave some space for other cursor settings changes
  index = MEASUREMENT_SETTING_OFFSET;
  
  //Copy the measurements enable states
  for(channel=0;channel<2;channel++)
  {
    //12 measurements per channel
    for(measurement=0;measurement<12;measurement++)
    {
      //Copy the current measurement state and point to the next one
       ptr[index++] = scopesettings.measuresstate[channel][measurement];
    }
  }
  
  //Calculate a checksum over the settings data
  for(index=1;index<VIEW_NUMBER_OF_SETTINGS;index++)
  {
    checksum += ptr[index];
  }
  
  //Add the sample data too
  for(index=0;index<750;index++)
  {
    //Add both the channels
    checksum += channel1tracebuffer[index];
    checksum += channel2tracebuffer[index];
  }

  //Store the checksum at the beginning of the file
  ptr[0] = checksum;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_restore_setup_from_file(void)
{
  uint32 *ptr = viewfilesetupdata;
  uint32 index = 0;
  uint32 channel;
  uint32 measurement;

  //Leave space for file version and checksum data
  index = CHANNEL1_SETTING_OFFSET;
  
  //Copy the needed channel 1 settings and measurements
  scopesettings.channel1.enable            = ptr[index++];
  scopesettings.channel1.displayvoltperdiv = ptr[index++];
  scopesettings.channel1.samplevoltperdiv  = ptr[index++];
  scopesettings.channel1.fftenable         = ptr[index++];
  scopesettings.channel1.coupling          = ptr[index++];
  scopesettings.channel1.magnification     = ptr[index++];
  scopesettings.channel1.traceposition     = ptr[index++];
  scopesettings.channel1.min               = ptr[index++];
  scopesettings.channel1.max               = ptr[index++];
  scopesettings.channel1.average           = ptr[index++];
  scopesettings.channel1.center            = ptr[index++];
  scopesettings.channel1.peakpeak          = ptr[index++];
  scopesettings.channel1.frequencyvalid    = ptr[index++];
  scopesettings.channel1.frequency         = ptr[index++];
  scopesettings.channel1.lowtime           = ptr[index++];
  scopesettings.channel1.hightime          = ptr[index++];
  scopesettings.channel1.periodtime        = ptr[index++];

  //Leave some space for channel 1 settings changes
  index = CHANNEL2_SETTING_OFFSET;
  
  //Copy the needed channel 2 settings and measurements
  scopesettings.channel2.enable            = ptr[index++];
  scopesettings.channel2.displayvoltperdiv = ptr[index++];
  scopesettings.channel2.samplevoltperdiv  = ptr[index++];
  scopesettings.channel2.fftenable         = ptr[index++];
  scopesettings.channel2.coupling          = ptr[index++];
  scopesettings.channel2.magnification     = ptr[index++];
  scopesettings.channel2.traceposition     = ptr[index++];
  scopesettings.channel2.min               = ptr[index++];
  scopesettings.channel2.max               = ptr[index++];
  scopesettings.channel2.average           = ptr[index++];
  scopesettings.channel2.center            = ptr[index++];
  scopesettings.channel2.peakpeak          = ptr[index++];
  scopesettings.channel2.frequencyvalid    = ptr[index++];
  scopesettings.channel2.frequency         = ptr[index++];
  scopesettings.channel2.lowtime           = ptr[index++];
  scopesettings.channel2.hightime          = ptr[index++];
  scopesettings.channel2.periodtime        = ptr[index++];

  //Leave some space for channel 2 settings changes
  index = TRIGGER_SETTING_OFFSET;
  
  //Copy the needed scope trigger settings
  scopesettings.timeperdiv                = ptr[index++];
  scopesettings.samplerate                = ptr[index++];
  scopesettings.triggermode               = ptr[index++];
  scopesettings.triggeredge               = ptr[index++];
  scopesettings.triggerchannel            = ptr[index++];
  scopesettings.triggerlevel              = ptr[index++];
  scopesettings.triggerhorizontalposition = ptr[index++];
  scopesettings.triggerverticalposition   = ptr[index++];
  disp_have_trigger                       = ptr[index++];
  disp_trigger_index                      = ptr[index++];

  //Leave some space for trigger information changes
  index = OTHER_SETTING_OFFSET;
  
  //Copy the needed other scope settings
  scopesettings.movespeed        = ptr[index++];
  scopesettings.rightmenustate   = ptr[index++];
  scopesettings.screenbrightness = ptr[index++];
  scopesettings.gridbrightness   = ptr[index++];
  scopesettings.alwaystrigger50  = ptr[index++];
  scopesettings.xymodedisplay    = ptr[index++];
  scopesettings.confirmationmode = ptr[index++];

  //Leave some space for other scope settings changes
  index = CURSOR_SETTING_OFFSET;
  
  //Copy the cursor settings
  scopesettings.timecursorsenable   = ptr[index++];
  scopesettings.voltcursorsenable   = ptr[index++];
  scopesettings.timecursor1position = ptr[index++];
  scopesettings.timecursor2position = ptr[index++];
  scopesettings.voltcursor1position = ptr[index++];
  scopesettings.voltcursor2position = ptr[index++];
  
  //Leave some space for other cursor settings changes
  index = MEASUREMENT_SETTING_OFFSET;
  
  //Copy the measurements enable states
  for(channel=0;channel<2;channel++)
  {
    //12 measurements per channel
    for(measurement=0;measurement<12;measurement++)
    {
      //Copy the current measurement state and point to the next one
      scopesettings.measuresstate[channel][measurement] = ptr[index++];
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_check_waveform_file(void)
{
  uint32 index;
  uint32 checksum = 0;
  
  //Calculate a checksum over the settings data
  for(index=1;index<VIEW_NUMBER_OF_SETTINGS;index++)
  {
    checksum += viewfilesetupdata[index];
  }
  
  //Add the sample data too
  for(index=0;index<750;index++)
  {
    //Add both the channels
    checksum += channel1tracebuffer[index];
    checksum += channel2tracebuffer[index];
  }

  //Check if it matches the checksum in the file
  if(viewfilesetupdata[0] == checksum)
  {
    return(0);
  }
  
  //Something is wrong so signal it
  return(-1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_print_file_name(uint32 filenumber)
{
  char    b[12];
  uint32  i = 12;   //Start beyond the array since the index is pre decremented
  uint32  s;

  //For file number 0 no need to do the work
  if(filenumber == 0)
  {
    //Value is zero so just set a 0 character
    b[--i] = '0';
  }
  else
  {
    //Process the digits
    while(filenumber)
    {
      //Set current digit to decreased index
      b[--i] = (filenumber % 10) + '0';

      //Take of the current digit
      filenumber /= 10;
    }
  }

  //Determine the size of the decimal part
  s = 12 - i;

  //Copy the path name first
  memcpy(viewfilename, view_file_path[viewtype & VIEW_TYPE_MASK].name, view_file_path[viewtype & VIEW_TYPE_MASK].length);

  //Copy in the decimal file number
  memcpy(&viewfilename[view_file_path[viewtype & VIEW_TYPE_MASK].length], &b[i], s);

  //Add the extension
  memcpy(&viewfilename[view_file_path[viewtype & VIEW_TYPE_MASK].length + s], view_file_extension[viewtype & VIEW_TYPE_MASK], 5);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_load_thumbnail_file(void)
{
  int32  result;
  uint32 size;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, view_file_path[viewtype & VIEW_TYPE_MASK].name);

  //Check the status of the directory for this view type
  result = f_stat(viewfilename, 0);

  //See if there is an error
  if(result != FR_OK)
  {
    //If so check if the directory does not exist
    if(result == FR_NO_FILE)
    {
      //Create the directory
      result = f_mkdir(viewfilename);

      if(result != FR_OK)
      {
        //Show a message stating creating the directory failed
        scope_display_file_status_message(MESSAGE_DIRECTORY_CREATE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Set the name in the global buffer for message display
      strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

      //With the directory created it is also needed to create the thumbnail file
      result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

      if(result != FR_OK)
      {
        //Show a message stating creating the file failed
        scope_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Reset the number of available items
      viewavailableitems = 0;

      //Write the no thumbnails yet data
      result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

      //Close the file
      f_close(&viewfp);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }

      //No items to be loaded so done
      return(0);
    }
    else
    {
      //Show a message stating the file system failed
      scope_display_file_status_message(MESSAGE_FILE_SYSTEM_FAILED, 1);

      //No sense to continue, so return with an error
      return(-1);
    }
  }

  //Clear the file number list to avoid errors when swapping between the two types
  memset(viewfilenumberdata, 0, sizeof(viewfilenumberdata));

  //Set the name in the global buffer for message display
  strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

  //Try to open the thumbnail file for this view type
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check the result
  if(result == FR_OK)
  {
    //Opened ok, so read the number of items
    result = f_read(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

    if(result != FR_OK)
    {
      //Show a message stating reading the file failed
      scope_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

      //Close the file
      f_close(&viewfp);

      //No sense to continue, so return with an error
      return(-1);
    }

    //Based on the number of available items load the rest of the data
    if(viewavailableitems)
    {
      //Calculate the number of bytes to read for the file number list
      size = viewavailableitems * sizeof(uint16);

      //Check if there is an error
      if(size > VIEW_FILE_NUMBER_DATA_SIZE)
      {
        //Show a message stating that the thumbnail file is corrupt
        scope_display_file_status_message(MESSAGE_THUMBNAIL_FILE_CORRUPT, 0);

        //Reset the number of available items
        viewavailableitems = 0;

        //Write the no thumbnails yet data
        result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

        //Close the file
        f_close(&viewfp);

        if(result != FR_OK)
        {
          //Show a message stating writing the file failed
          scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

          //No sense to continue, so return with an error
          return(-1);
        }

        //No items to be loaded any more so done
        return(0);
      }

      //Read the file number data
      result = f_read(&viewfp, viewfilenumberdata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating reading the file failed
        scope_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Calculate the number of bytes to read for the thumbnail data
      size = viewavailableitems * sizeof(THUMBNAILDATA);

      //Check if there is an error
      if(size > VIEW_THUMBNAIL_DATA_SIZE)
      {
        //Show a message stating that the thumbnail file is corrupt
        scope_display_file_status_message(MESSAGE_THUMBNAIL_FILE_CORRUPT, 0);

        //Reset the number of available items
        viewavailableitems = 0;

        //Write the no thumbnails yet data
        result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

        //Close the file
        f_close(&viewfp);

        if(result != FR_OK)
        {
          //Show a message stating writing the file failed
          scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

          //No sense to continue, so return with an error
          return(-1);
        }

        //No items to be loaded any more so done
        return(0);
      }

      //Read the thumbnail data
      result = f_read(&viewfp, viewthumbnaildata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating reading the file failed
        scope_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }
    }

    //Close the file
    f_close(&viewfp);
  }
  //Failure then check if file does not exist
  else if(result == FR_NO_FILE)
  {
    //Need the file so create it
    result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

    //Check if file is created ok
    if(result == FR_OK)
    {
      //Reset the number of available items
      viewavailableitems = 0;

      //Write the no thumbnails yet data
      result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

      //Close the file
      f_close(&viewfp);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //No sense to continue, so return with an error
        return(-1);
      }
    }
    else
    {
      //Show a message stating creating the file failed
      scope_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);

      //No sense to continue, so return with an error
      return(-1);
    }
  }

  //Signal all went well
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_save_thumbnail_file(void)
{
  int32  result;
  uint32 size;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, thumbnail_file_names[viewtype & VIEW_TYPE_MASK]);

  //Try to open the thumbnail file for this view type
  result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

  //Only if the file is opened write to it
  if(result == FR_OK)
  {
    //Write the number of available items to the file
    result = f_write(&viewfp, &viewavailableitems, sizeof(viewavailableitems), 0);

    if(result != FR_OK)
    {
      //Show a message stating writing the file failed
      scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

      //Close the file
      f_close(&viewfp);

      //No sense to continue, so return with an error
      return(-1);
    }

    //Based on the number of available items write the rest of the data
    if(viewavailableitems)
    {
      //Calculate the number of bytes to write for the file number list
      size = viewavailableitems * sizeof(uint16);

      //Write the file number list to the file
      result = f_write(&viewfp, viewfilenumberdata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }

      //Calculate the number of bytes to write for the thumbnail data
      size = viewavailableitems * sizeof(THUMBNAILDATA);

      //Write the thumbnail data to the file
      result = f_write(&viewfp, viewthumbnaildata, size, 0);

      if(result != FR_OK)
      {
        //Show a message stating writing the file failed
        scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);

        //Close the file
        f_close(&viewfp);

        //No sense to continue, so return with an error
        return(-1);
      }
    }

    //Close the file
    f_close(&viewfp);
  }
  else
  {
    //Show a message stating the file system failed
    scope_display_file_status_message(MESSAGE_FILE_SYSTEM_FAILED, 1);

    //No sense to continue, so return with an error
    return(-1);
  }

  //Signal no problem occurred
  return(0);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_view_item_file(int32 type)
{
  uint32  newnumber;
  uint32  result;
  uint16 *fnptr;
  uint16 *eptr;

  //Save the current view type to be able to determine if the thumbnail file need to be reloaded
  uint32 currentviewtype = viewtype;

  //Switch to the given type
  viewtype = type;

  //Load the thumbnail file for this type. Needed for finding the file name and to add the thumbnail
  if(scope_load_thumbnail_file() != 0)
  {
    //Loading the thumbnail file failed so no sense in going on
    return;
  }

  //Check if there is still room for a new item
  if(viewavailableitems >= VIEW_MAX_ITEMS)
  {
    //Show the user there is no more room for a new item
    scope_display_file_status_message(MESSAGE_THUMBNAIL_FILE_FULL, 1);

    //No sense to continue
    return;
  }

  //Set the end pointer
  eptr = &viewfilenumberdata[viewavailableitems];

  //Find the first free file number
  //Most likely a more efficient solution for this problem exists, but this beats the original code where they try if a file number is free on the SD card with f_open
  for(newnumber=1;newnumber<VIEW_MAX_ITEMS;newnumber++)
  {
    //Start at the beginning of the list
    fnptr = viewfilenumberdata;

    //Go through the list to see if the current number is in the list
    while(fnptr < eptr)
    {
      //Check if this number is in the list
      if(*fnptr == newnumber)
      {
        //Found it, so quit the loop
        break;
      }

      //Select the next number entry
      fnptr++;
    }

    //Check if not found
    if(*fnptr != newnumber)
    {
      //Can use this number since it is not in the list
      break;
    }
  }

  //Bump all the entries in the list up
  memmove(&viewfilenumberdata[1], &viewfilenumberdata[0], viewavailableitems * sizeof(uint16));

  //Fill in the new number
  viewfilenumberdata[0] = newnumber;

  //Bump the thumbnails up to make room for the new one
  memmove(&viewthumbnaildata[1], &viewthumbnaildata[0], viewavailableitems * sizeof(THUMBNAILDATA));

  //Setup the filename for in the thumbnail
  scope_print_file_name(newnumber);

  //Create the thumbnail
  scope_create_thumbnail(&viewthumbnaildata[0]);

  //One more item in the list
  viewavailableitems++;

  //save the amended thumbnail file
  scope_save_thumbnail_file();

  //Copy the filename from the thumbnail filename, since the global one got written over in the saving of the thumbnail
  //Might need a re write of the message setup
  strcpy(viewfilename, viewthumbnaildata[0].filename);
  
  //Open the new file. On failure signal this and quit
  result = f_open(&viewfp, viewfilename, FA_CREATE_ALWAYS | FA_WRITE);

  //Check if file created without problems
  if(result == FR_OK)
  {
    //For pictures the bitmap header and the screen data needs to be written
    if(type == VIEW_TYPE_PICTURE)
    {
      //Write the bitmap header
      result = f_write(&viewfp, bmpheader, sizeof(bmpheader), 0);

      //Check if still ok to proceed
      if(result == FR_OK)
      {
        //Write the pixel data
        result = f_write(&viewfp, (uint8 *)maindisplaybuffer, PICTURE_DATA_SIZE, 0);
      }
    }
    else
    {
      //For the waveform the setup and the waveform data needs to be written
      //Save the settings for the trace portion of the data and write them to the file
      scope_prepare_setup_for_file();

      //Write the setup data to the file
      if((result = f_write(&viewfp, viewfilesetupdata, sizeof(viewfilesetupdata), 0)) == FR_OK)
      {
        //Write the trace data to the file
        //Save the channel 1 raw sample data
        if((result = f_write(&viewfp, (uint8 *)channel1tracebuffer, 3000, 0)) == FR_OK)
        {
          //Save the channel 2 raw sample data
          result = f_write(&viewfp, (uint8 *)channel2tracebuffer, 3000, 0);
        }
      }
    }

    //Close the file
    f_close(&viewfp);

    //Check if all went well
    if(result == FR_OK)
    {
      //Show the saved successful message
      scope_display_file_status_message(MESSAGE_SAVE_SUCCESSFUL, 0);
    }
    else
    {
      //Signal unable to write to the file
      scope_display_file_status_message(MESSAGE_FILE_WRITE_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to create the file
    scope_display_file_status_message(MESSAGE_FILE_CREATE_FAILED, 0);
  }

  //When a picture is saved while viewing a waveform, reload the waveform lists
  if((type == VIEW_TYPE_PICTURE) && (currentviewtype == VIEW_TYPE_WAVEFORM) && (scopesettings.waveviewmode == 1))
  {
    //Restore the previous view type
    viewtype = currentviewtype;

    //Load the thumbnail file
    scope_load_thumbnail_file();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_remove_item_from_thumbnails(uint32 delete)
{
  //Set the index to the next item
  uint32 nextindex = viewcurrentindex + 1;

  //Calculate the number of items to move
  uint32 count = (viewavailableitems - nextindex);

  //Only delete the file when requested
  if(delete)
  {
    //Set the name in the global buffer for message display
    strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

    //Delete the file from the SD card
    if(f_unlink(viewfilename) != FR_OK)
    {
      //Signal unable to create the file
      scope_display_file_status_message(MESSAGE_FILE_DELETE_FAILED, 0);
    }
  }

  //Bump all the entries in the file number list down
  memmove(&viewfilenumberdata[viewcurrentindex], &viewfilenumberdata[nextindex], count * sizeof(uint16));

  //Bump the thumbnails down to erase the removed one
  memmove(&viewthumbnaildata[viewcurrentindex], &viewthumbnaildata[nextindex], count * sizeof(THUMBNAILDATA));

  //One less item available
  viewavailableitems--;

  //Clear the freed up slot
  viewfilenumberdata[viewavailableitems] = 0;
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_load_trace_data(void)
{
  //Point to the file numbers
  uint16 *fnptr = (uint16 *)viewfilenumberdata;
  uint32 result;

  //Setup the file name for this view item
  scope_print_file_name(fnptr[viewcurrentindex]);

  //Try to open the file for reading
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check if file opened ok
  if(result == FR_OK)
  {
    //Checks on correct number of bytes read might be needed
    //Load the setup data to the file setup data buffer
    if((result = f_read(&viewfp, (uint8 *)viewfilesetupdata, sizeof(viewfilesetupdata), 0)) == FR_OK)
    {
      //Copy the loaded data to the settings
      scope_restore_setup_from_file();

      //Check if the version of the file is wrong
      if(viewfilesetupdata[1] != WAVEFORM_FILE_VERSION)
      {
        //No need to load the rest of the data
        result = WAVEFORM_FILE_ERROR;

        //Show the user the file is not correct
        scope_display_file_status_message(MESSAGE_WAV_VERSION_MISMATCH, 0);
      }
      else
      {
        //Load the channel 1 sample data      
        if((result = f_read(&viewfp, (uint8 *)channel1tracebuffer, 3000, 0)) == FR_OK)
        {
          //Load the channel 2 sample data
          if((result = f_read(&viewfp, (uint8 *)channel2tracebuffer, 3000, 0)) == FR_OK)
          {
            //Do a check on file validity
            if((result = scope_check_waveform_file()) == 0)
            {
              //Switch to stopped and waveform viewing mode
              scopesettings.runstate = 1;
              scopesettings.waveviewmode = 1;

              //Show the normal scope screen
              scope_setup_main_screen();

              //display the trace data
              scope_display_trace_data();
            }
            else
            {
              //Checksum error so signal that to the user
              result = WAVEFORM_FILE_ERROR;

              //Show the user the file is not correct
              scope_display_file_status_message(MESSAGE_WAV_CHECKSUM_ERROR, 0);
            }
          }
        }
      }
    }

    //Done with the file so close it
    f_close(&viewfp);

    //Check if one of the reads failed
    if((result != FR_OK) && (result != WAVEFORM_FILE_ERROR))
    {
      //Signal unable to write to the file
      scope_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to open the file
    scope_display_file_status_message(MESSAGE_FILE_OPEN_FAILED, 0);
  }

  //Check if all went well
  if(result == FR_OK)
  {
    //Tell it to the caller
    return(VIEW_TRACE_LOAD_OK);
  }

  //Remove the current item from the thumbnnails and delete the item from disk
  scope_remove_item_from_thumbnails(1);

  //Save the thumbnail file
  scope_save_thumbnail_file();

  return(VIEW_TRACE_LOAD_ERROR);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_load_bitmap_data(void)
{
  uint32 result;

  //Set the name in the global buffer for message display
  strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

  //Try to open the file for reading
  result = f_open(&viewfp, viewfilename, FA_READ);

  //Check if file opened ok
  if(result == FR_OK)
  {
    //Read the bitmap header to verify if the bitmap can be displayed
    result = f_read(&viewfp, viewbitmapheader, PICTURE_HEADER_SIZE, 0);

    //Check if still ok to proceed
    if(result == FR_OK)
    {
      //Check if the header matches what it should be
      if(memcmp(viewbitmapheader, bmpheader, PICTURE_HEADER_SIZE) == 0)
      {
        //Load the bitmap data directly onto the screen
        result = f_read(&viewfp, (uint8 *)maindisplaybuffer, PICTURE_DATA_SIZE, 0);
      }
      else
      {
        //Signal a header mismatch detected
        result = PICTURE_HEADER_MISMATCH;

        //Show the user the file is not correct
        scope_display_file_status_message(MESSAGE_BMP_HEADER_MISMATCH, 0);
      }
    }

    //Done with the file so close it
    f_close(&viewfp);

    //Check if one of the reads failed
    if((result != FR_OK) && (result != PICTURE_HEADER_MISMATCH))
    {
      //Signal unable to read from the file
      scope_display_file_status_message(MESSAGE_FILE_READ_FAILED, 0);
    }
  }
  else
  {
    //Signal unable to open the file
    scope_display_file_status_message(MESSAGE_FILE_OPEN_FAILED, 0);
  }

  //Check if all went well
  if(result == FR_OK)
  {
    //Tell it to the caller
    return(VIEW_BITMAP_LOAD_OK);
  }

  //Remove the current item from the thumbnnails and delete the item from disk
  scope_remove_item_from_thumbnails(1);

  //Save the thumbnail file
  scope_save_thumbnail_file();

  return(VIEW_BITMAP_LOAD_ERROR);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_sync_thumbnail_files(void)
{
  uint32 save = 0;

  //Handle the two types of list files
  for(viewtype=0;viewtype<VIEW_MAX_TYPES;viewtype++)
  {
    //Load the thumbnail file for this type
    if(scope_load_thumbnail_file() != 0)
    {
      //Loading the thumbnail file failed so no sense in going on for this type
      continue;
    }

    //Go through the items in the thumbnail file and check if the needed files still exist on the SD card
    for(viewcurrentindex=0;viewcurrentindex<viewavailableitems;)
    {
      //Set the name in the global buffer for message display
      strcpy(viewfilename, viewthumbnaildata[viewcurrentindex].filename);

      //Try to open the file. On failure remove it from the lists
      if(f_open(&viewfp, viewfilename, FA_READ) == FR_NO_FILE)
      {
        //Remove the current item from the thumbnails without delete, since it is already removed from the SD card
        scope_remove_item_from_thumbnails(0);

        //Signal saving of the thumbnail file is needed
        save = 1;
      }
      else
      {
        //File exists so close it
        f_close(&viewfp);
        
        //Point to the next item. Only needed if item still exists, because it is removed from the list otherwise
        viewcurrentindex++;
      }
    }

    //Check if there was a change
    if(save)
    {
      //Save the thumbnail file if so
      scope_save_thumbnail_file();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_initialize_and_display_thumbnails(void)
{
  //No pages yet
  viewpages = 0;

  //Calculate the number of pages available, based on number of items per page. 0 means 1 page
  //available items starts with 1 and with 16 items it would result in pages being 1, so need to subtract 1 before dividing
  if(viewavailableitems)
  {
    viewpages = (viewavailableitems - 1) / VIEW_ITEMS_PER_PAGE;
  }

  //Need to check if the current page is still valid
  if(viewpage > viewpages)
  {
    //Page no longer valid then use last page
    viewpage = viewpages;
  }

  //Display the thumbnails
  scope_display_thumbnails();
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_thumbnails(void)
{
  PTHUMBNAILDATA thumbnaildata;

  //Determine the first index based on the current page
  uint32 index = viewpage * VIEW_ITEMS_PER_PAGE;

  //Start with first item for drawing
  uint32 xpos = VIEW_ITEM_XSTART;
  uint32 ypos = VIEW_ITEM_YSTART;

  uint32 button, x, y;

  //Set black color for background
  display_set_fg_color(0x00000000);

  //Clear the thumbnail display area
  display_fill_rect(0, 0, 730, 480);

  //Check if there are items to display
  if(viewavailableitems)
  {
    //Determine the available items for the current page
    if(viewpage < viewpages)
    {
      //Not on the last page so full set available
      viewitemsonpage = VIEW_ITEMS_PER_PAGE;
    }
    else
    {
      //Get the remainder of items for the last page
      uint32 nofitems = viewavailableitems % VIEW_ITEMS_PER_PAGE;

      //See if a fraction of the max items per page is available
      if(nofitems)
      {
        //If so only display these
        viewitemsonpage = nofitems;
      }
      else
      {
        //If the remainder is zero there are max number of items on the last page
        viewitemsonpage = VIEW_ITEMS_PER_PAGE;
      }
    }

    //Determine the last index based on the available items on the current page
    uint32 lastindex = index + viewitemsonpage;

    //Draw the available items on the screen
    while(index < lastindex)
    {
      //Create a nicer thumbnail by drawing the menu buttons and some extra lines
      y = ypos + 1;

      //Main menu button blue
      display_set_fg_color(0x00000078);
      display_fill_rect(xpos, y, 20, 8);

      //Speed text
      display_fill_rect(xpos + 112, y, 10, 8);

      //Channel 1 button
      display_set_fg_color(CHANNEL1_COLOR);
      display_fill_rect(xpos + 38, y, 8, 8);

      //Channel 2 button
      display_set_fg_color(CHANNEL1_COLOR);
      display_fill_rect(xpos + 65, y, 8, 8);

      //Acquisition button
      display_set_fg_color(TRIGGER_COLOR);
      display_fill_rect(xpos + 92, y, 8, 8);

      //Trigger menu button
      display_fill_rect(xpos + 132, y, 8, 8);

      //Battery
      display_fill_rect(xpos + 160, y, 8, 4);

      //Light grey for the buttons
      display_set_fg_color(0x00303030);

      x = xpos + 173;
      y = ypos + 3;

      //Draw the right buttons
      for(button=0;button<8;button++)
      {
        display_fill_rect(x, y, 8, 10);

        y += 15;
      }

      //Set grey color for trace border
      display_set_fg_color(0x00909090);
      display_draw_rect(xpos + 2, ypos + 11, VIEW_ITEM_WIDTH - 13, VIEW_ITEM_HEIGHT - 25);

      //Draw a grid
      display_set_fg_color(0x00606060);

      //Draw the center lines
      display_draw_horz_line(ypos + 60, xpos + 3, xpos + 169);
      display_draw_vert_line(xpos + 86, ypos + 12, ypos + VIEW_ITEM_HEIGHT - 16);

      //Point to the current thumbnail
      thumbnaildata = &viewthumbnaildata[index];

      //Display the thumbnail
      //Need to make a distinction between normal display and xy display mode
      if(thumbnaildata->xydisplaymode == 0)
      {
        //Normal mode
        //To avoid errors make sure the positions are in range
        //Data is read back from file so could be modified
        if(thumbnaildata->disp_xstart < 3)
        {
          thumbnaildata->disp_xstart = 3;
        }

        if(thumbnaildata->disp_xend > 169)
        {
          thumbnaildata->disp_xend = 169;
        }

        //Set the x start position based on the given start x.
        uint32 xs = xpos + thumbnaildata->disp_xstart;
        uint32 xe = xpos + thumbnaildata->disp_xend;

        //Offset the trace data to below the signal area border
        y = ypos + 12;

        //Check if channel 1 is enabled
        if(thumbnaildata->channel1enable)
        {
          scope_display_thumbnail_data(xs, xe, y, CHANNEL1_COLOR, thumbnaildata->channel1data);
        }

        //Check if channel 2 is enabled
        if(thumbnaildata->channel2enable)
        {
          scope_display_thumbnail_data(xs, xe, y, CHANNEL2_COLOR, thumbnaildata->channel2data);
        }
      }
      else
      {
        //xy display mode so set the trace color for it
        display_set_fg_color(XYMODE_COLOR);

        //Point to the data of the two channels
        uint8 *channel1data = thumbnaildata->channel1data;
        uint8 *channel2data = thumbnaildata->channel2data;

        //Start with first sample
        uint32 sample = 0;

        //Center the xy display
        uint32 y = ypos + 12;

        //Keep the samples in registers
        register uint32 x1, x2, y1, y2;

        //Load the first samples
        x1 = *channel1data + xpos;
        y1 = *channel2data + y;

        //Point to the next samples
        channel1data++;
        channel2data++;

        //Draw the trace
        while(sample < 172)
        {
          //Get second samples
          x2 = *channel1data + xpos;
          y2 = *channel2data + y;

          //Draw all the lines
          display_draw_line(x1, y1, x2, y2);

          //Swap the samples
          x1 = x2;
          y1 = y2;

          //Point to the next samples
          channel1data++;
          channel2data++;

          //One sample done
          sample++;
        }
      }

      //Set white color for item border
      display_set_fg_color(0x00808000);

      //Draw the border
      display_draw_rect(xpos, ypos, VIEW_ITEM_WIDTH, VIEW_ITEM_HEIGHT);

      //Need to make a distinction between normal display and xy display mode for displaying the pointers
      if(thumbnaildata->xydisplaymode == 0)
      {
        //Channel pointers position bases
        x = xpos + 3;
        y = ypos + 12;

        //Check if channel 1 is enabled
        if(thumbnaildata->channel1enable)
        {
          //Limit the position to the extremes
          if(thumbnaildata->channel1traceposition > 92)
          {
            thumbnaildata->channel1traceposition = 92;
          }

          //If so draw its pointer
          scope_thumbnail_draw_pointer(x, y + thumbnaildata->channel1traceposition, THUMBNAIL_POINTER_RIGHT, CHANNEL1_COLOR);
        }

        //Check if channel 2 is enabled
        if(thumbnaildata->channel2enable)
        {
          //Limit the position to the extremes
          if(thumbnaildata->channel2traceposition > 92)
          {
            thumbnaildata->channel2traceposition = 92;
          }

          //If so draw its pointer
          scope_thumbnail_draw_pointer(x, y + thumbnaildata->channel2traceposition, THUMBNAIL_POINTER_RIGHT, CHANNEL2_COLOR);
        }

        //Trigger level position base
        x = xpos + 170;

        //Limit the position to the extremes
        if(thumbnaildata->triggerverticalposition > 92)
        {
          thumbnaildata->triggerverticalposition = 92;
        }

        //Draw the trigger level pointer
        scope_thumbnail_draw_pointer(x, y + thumbnaildata->triggerverticalposition, THUMBNAIL_POINTER_LEFT, TRIGGER_COLOR);

        //Limit the position to the extremes
        if(thumbnaildata->triggerhorizontalposition < 3)
        {
          thumbnaildata->triggerhorizontalposition = 3;
        }
        else if(thumbnaildata->triggerhorizontalposition > 165)
        {
          thumbnaildata->triggerhorizontalposition = 165;
        }

        //Draw the trigger position pointer
        scope_thumbnail_draw_pointer(xpos + thumbnaildata->triggerhorizontalposition, y, THUMBNAIL_POINTER_DOWN, TRIGGER_COLOR);
      }
      else
      {
//Draw the pointers here

      }

      //Display the file name in the bottom left corner
      display_set_fg_color(0x00FFFFFF);
      display_set_font(&font_2);
      display_text(xpos + 7, ypos + 105, thumbnaildata->filename);

      //Skip to next coordinates
      xpos += VIEW_ITEM_XNEXT;

      //Check if next row needs to be used
      if(xpos > VIEW_ITEM_XLAST)
      {
        //Reset x position to beginning of row
        xpos = VIEW_ITEM_XSTART;

        //Bump y position to next row
        ypos += VIEW_ITEM_YNEXT;
      }

      //Select next index
      index++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_thumbnail_data(uint32 xstart, uint32 xend, uint32 ypos, uint32 color, uint8 *buffer)
{
  register uint32 x;
  register uint32 sample1, sample2;

  //Set the trace color
  display_set_fg_color(color);

  //Get the first sample
  sample1 = *buffer++;

  //Position it within the thumbnail on screen
  sample1 += ypos;

  //Do while the samples last
  for(x=xstart;x<xend;x++)
  {
    //Get the second sample
    sample2 = *buffer++;

    //Position it within the thumbnail on screen
    sample2 += ypos;

    //Draw the line for these samples
    display_draw_line(x, sample1, x + 1, sample2);

    //Swap the samples
    sample1 = sample2;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_create_thumbnail(PTHUMBNAILDATA thumbnaildata)
{
  uint16 position;

  //Set the thumbnails filename
  strcpy(thumbnaildata->filename, viewfilename);

  //Calculate and limit pointer position for channel 1
  position = 441 - scopesettings.channel1.traceposition;

  //Limit on the top of the displayable region
  if(position < 46)
  {
    position = 46;
  }
  //Limit on the bottom of the displayable region
  else if(position > 441)
  {
    position = 441;
  }

  //Set the parameters for channel 1
  thumbnaildata->channel1enable        = scopesettings.channel1.enable;
  thumbnaildata->channel1traceposition = (uint8)(((position - 46) * 10000) / 42210);

  //Calculate and limit pointer position for channel 2
  position = 441 - scopesettings.channel2.traceposition;

  //Limit on the top of the displayable region
  if(position < 46)
  {
    position = 46;
  }
  //Limit on the bottom of the displayable region
  else if(position > 441)
  {
    position = 441;
  }

  //Set the parameters for channel 2
  thumbnaildata->channel2enable      = scopesettings.channel2.enable;
  thumbnaildata->channel2traceposition = (uint8)(((position - 46) * 10000) / 42210);

  //Calculate and limit pointer position for trigger level
  position = 441 - scopesettings.triggerverticalposition;

  //Limit on the top of the displayable region
  if(position < 46)
  {
    position = 46;
  }
  //Limit on the bottom of the displayable region
  else if(position > 441)
  {
    position = 441;
  }

  //Set trigger information
  thumbnaildata->triggerverticalposition   = (uint8)(((position - 46) * 10000) / 42210);
  thumbnaildata->triggerhorizontalposition = (scopesettings.triggerhorizontalposition * 10000) / 42899;

  //Set the xy display mode
  thumbnaildata->xydisplaymode = scopesettings.xymodedisplay;

  //Set the display start and end x positions. Conversion to thumbnail x coordinates is dividing by 4,2899
  thumbnaildata->disp_xstart = (disp_xstart * 10000) / 42899;
  thumbnaildata->disp_xend   = (disp_xend * 10000) / 42899;

  //Check which display mode is active
  if(scopesettings.xymodedisplay == 0)
  {
    //Normal mode so check on channel 1 being enabled
    if(scopesettings.channel1.enable)
    {
      //Process the trace points
      scope_thumbnail_set_trace_data(&scopesettings.channel1, thumbnaildata->channel1data);
    }

    //Check on channel 2 being enabled
    if(scopesettings.channel2.enable)
    {
      //Process the trace points
      scope_thumbnail_set_trace_data(&scopesettings.channel2, thumbnaildata->channel2data);
    }
  }
  else
  {
    //Use less samples to not overwrite the second buffer
    uint32 index = disp_trigger_index - 317;
    uint32 last = index + 728;

    uint8 *buffer1 = thumbnaildata->channel1data;
    uint8 *buffer2 = thumbnaildata->channel2data;
    
    //Copy and scale every 4th sample for this channel
    for(;index<last;index+=4)
    {
      //Adjust the samples to fit the thumbnail screen. Channel 1 is x, channel 2 is y
      *buffer1++ = (scope_get_x_sample(&scopesettings.channel1, index) * 10000) / 42210;
      *buffer2++ = ((scope_get_y_sample(&scopesettings.channel2, index) - 47) * 10000) / 42210;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_thumbnail_set_trace_data(PCHANNELSETTINGS settings, uint8 *buffer)
{
  int32  index;
  uint32 pattern;

  //Point to the first and second trace point for easy access
  PDISPLAYPOINTS ptr1 = &settings->tracepoints[0];
  PDISPLAYPOINTS ptr2 = &settings->tracepoints[1];

  //Process the points
  for(index=1;index<settings->noftracepoints;index++)
  {
    //FIll in the blanks between the given points
    scope_thumbnail_calculate_trace_data(ptr1->x, ptr1->y, ptr2->x, ptr2->y);

    //Select the next points
    ptr1++;
    ptr2++;
  }

  //Down sample the points in to the given buffer
  //This yields a max of 182 points, which is more then is displayed on the thumbnail screen
  for(index=disp_xstart,pattern=0;index<=disp_xend;index+=4,pattern++)
  {
    //Adjust the y point to fit the thumbnail screen. First trace y position on screen is 47. The available height on the thumbnail is 95 pixels so divide by 4,2210
    *buffer++ = (uint8)(((thumbnailtracedata[index] - 47) * 10000) / 42210);

    //Skip one more sample every third loop
    if(pattern == 2)
    {
      pattern = -1;
      index++;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_thumbnail_calculate_trace_data(int32 xstart, int32 ystart, int32 xend, int32 yend)
{
  register int32  x, dx;
  register int32  yacc;
  register int32  ystep;

  //Calculate delta x.
  dx = xend - xstart;

  //Calculate the y segment length
  ystep = ((yend - ystart) << 16) / dx;

  //Initialize the y accumulator for broken pixel accounting
  yacc = ystart << 16;

  //Set the start and end points
  thumbnailtracedata[xstart] = ystart;
  thumbnailtracedata[xend]   = yend;

  //Check if there are points in between
  if(dx > 2)
  {
    //Handle the in between x positions
    for(x=xstart+1;x<xend;x++)
    {
      //Calculate the y point of this segment
      yacc += ystep;

      //Set it in the buffer
      thumbnailtracedata[x] = yacc >> 16;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_thumbnail_draw_pointer(uint32 xpos, uint32 ypos, uint32 direction, uint32 color)
{
  uint32 x1, y1, x2, y2, w, h;

  //Set the pointer color
  display_set_fg_color(color);

  //Setup the coordinates based on the direction
  switch(direction)
  {
    //Pointing to the right
    default:
    case THUMBNAIL_POINTER_RIGHT:
      x1 = xpos;
      y1 = ypos;
      x2 = x1 + 4;
      y2 = y1 + 1;
      w = 4;
      h = 3;
      break;

    //Pointing to the left
    case THUMBNAIL_POINTER_LEFT:
      x1 = xpos - 4;
      y1 = ypos;
      x2 = x1 - 1;
      y2 = y1 + 1;
      w = 4;
      h = 3;
      break;

    //Pointing down
    case THUMBNAIL_POINTER_DOWN:
      x1 = xpos;
      y1 = ypos;
      x2 = x1 + 1;
      y2 = y1 + 4;
      w = 3;
      h = 4;
      break;
  }

  //Draw the body
  display_fill_rect(x1, y1, w, h);

  //Draw the point
  display_fill_rect(x2, y2, 1, 1);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_display_picture_item(void)
{
  //Display the new item
  if(scope_load_bitmap_data() == VIEW_BITMAP_LOAD_ERROR)
  {
    //Return on an error
    return(VIEW_BITMAP_LOAD_ERROR);
  }

  //And draw the bottom menu bar with a save of the background
  scope_setup_bottom_file_menu(VIEW_BOTTON_MENU_INIT);

  return(VIEW_BITMAP_LOAD_OK);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_selected_signs(void)
{
  uint32 index = 0;
  uint32 xpos = VIEW_ITEM_SELECTED_XSTART;
  uint32 ypos = VIEW_ITEM_SELECTED_YSTART;

  //Set the colors for displaying the selected sign. White sign on blue background
  display_set_fg_color(0x00FFFFFF);
  display_set_bg_color(0x000000FF);

  //Can't have more selects than items on the page
  while(index < viewitemsonpage)
  {
    //Handle the current item based on its state
    switch(viewitemselected[index])
    {
      case VIEW_ITEM_SELECTED_NOT_DISPLAYED:
        //Make a copy of the screen under the selected sign location
        display_copy_rect_from_screen(xpos, ypos, 30, 30);

        //Display the selected sign
        display_copy_icon_use_colors(select_sign_icon, xpos, ypos, 30, 30);

        //Switch to displayed state
        viewitemselected[index] = VIEW_ITEM_SELECTED_DISPLAYED;
        break;

      case VIEW_ITEM_NOT_SELECTED_DISPLAYED:
        //Restore the screen on the selected sign location
        display_copy_rect_to_screen(xpos, ypos, 30, 30);

        //Switch to not selected state
        viewitemselected[index] = VIEW_ITEM_NOT_SELECTED;
        break;
    }

    //Skip to next coordinates
    xpos += VIEW_ITEM_XNEXT;

    //Check if next row needs to be used
    if(xpos > VIEW_ITEM_XLAST)
    {
      //Reset x position to beginning of selected row
      xpos = VIEW_ITEM_SELECTED_XSTART;

      //Bump y position to next row
      ypos += VIEW_ITEM_YNEXT;
    }

    //Select next index
    index++;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_file_status_message(int32 msgid, int32 alwayswait)
{
  uint32 checkconfirmation = scopesettings.confirmationmode;

  //Check if need to wait is requested
  if(alwayswait)
  {
    //If so override the setting
    checkconfirmation = 1;
  }

  //Need to save the screen buffer pointer and set it to the actual screen
  //When displaying trace data to avoid flickering data is drawn in a different screen buffer
  display_save_screen_buffer();

  //Save the screen rectangle where the message will be displayed
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_set_destination_buffer(displaybuffer2);
  display_copy_rect_from_screen(260, 210, 280, 60);

  //Draw the background in grey
  display_set_fg_color(0x00202020);
  display_fill_rect(260, 210, 280, 60);

  //Draw the border in a lighter grey
  display_set_fg_color(0x00303030);
  display_draw_rect(260, 210, 280, 60);

  //White color for text and use font_3
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);

  switch(msgid)
  {
    case MESSAGE_SAVE_SUCCESSFUL:
      display_text(270, 220, "File saved successfully");

      //Don't wait for confirmation in case of success, unless requested
      checkconfirmation = alwayswait;
      break;

    case MESSAGE_FILE_CREATE_FAILED:
      display_text(270, 220, "Failed to create file");
      break;

    case MESSAGE_FILE_OPEN_FAILED:
      display_text(270, 220, "Failed to open file");
      break;

    case MESSAGE_FILE_WRITE_FAILED:
      display_text(270, 220, "Failed to write to file");
      break;

    case MESSAGE_FILE_READ_FAILED:
      display_text(270, 220, "Failed to read from file");
      break;

    case MESSAGE_FILE_SEEK_FAILED:
      display_text(270, 220, "Failed to seek in file");
      break;

    case MESSAGE_FILE_DELETE_FAILED:
      display_text(270, 220, "Failed to delete file");
      break;

    case MESSAGE_DIRECTORY_CREATE_FAILED:
      display_text(270, 220, "Failed to create directory");
      break;

    case MESSAGE_FILE_SYSTEM_FAILED:
      display_text(270, 220, "File system failure");
      break;

    case MESSAGE_THUMBNAIL_FILE_CORRUPT:
      display_text(270, 220, "Corrupt thumbnail file");
      break;

    case MESSAGE_THUMBNAIL_FILE_FULL:
      display_text(270, 220, "Thumbnail file is full");
      break;

    case MESSAGE_BMP_HEADER_MISMATCH:
      display_text(270, 220, "Bitmap header mismatch");
      break;
      
    case MESSAGE_WAV_VERSION_MISMATCH:
      display_text(270, 220, "Waveform file version mismatch");
      break;
      
    case MESSAGE_WAV_CHECKSUM_ERROR:
      display_text(270, 220, "Waveform file checksum error");
      break;
  }

  //Display the file name in question
  display_text(270, 245, viewfilename);

  //Maybe wait for touch to continue in case of an error message
  if(checkconfirmation)
  {
    //wait for touch
    while(1)
    {
      //Read the touch panel status
      tp_i2c_read_status();

      //Check if the panel is touched
      if(havetouch)
      {
        //Done so quit the loop
        break;
      }
    }

    //Need to wait for touch to release before returning
    tp_i2c_wait_for_touch_release();
  }
  else
  {
    //Wait for half a second
    timer0_delay(500);
  }

  //Restore the original screen
  display_set_source_buffer(displaybuffer2);
  display_copy_rect_to_screen(260, 210, 280, 60);

  //Need to restore the screen buffer pointer
  display_restore_screen_buffer();
}

//----------------------------------------------------------------------------------------------------------------------------------
// Configuration data functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_load_configuration_data(void)
{
#if 0
  //Get the settings data form the flash memory
  sys_spi_flash_read(0x001FD000, (uint8 *)settingsworkbuffer, sizeof(settingsworkbuffer));
#else
  //Load the settings data from its sector on the SD card
  if(sd_card_read(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer) != SD_OK)
  {
    settingsworkbuffer[2] = 0;
    settingsworkbuffer[3] = 0;
  }
#endif

  //Restore the settings from the loaded data
  scope_restore_config_data();

  //Set the FPGA commands for channel 1
  scopesettings.channel1.enablecommand     = 0x02;
  scopesettings.channel1.couplingcommand   = 0x34;
  scopesettings.channel1.voltperdivcommand = 0x33;
  scopesettings.channel1.offsetcommand     = 0x32;
  scopesettings.channel1.adc1command       = 0x20;
  scopesettings.channel1.adc2command       = 0x21;

  //Set the menu and button data for channel 1
  scopesettings.channel1.color        = CHANNEL1_COLOR;
  scopesettings.channel1.buttonxpos   = CH1_BUTTON_XPOS;
  scopesettings.channel1.menuxpos     = CH1_MENU_XPOS;
  scopesettings.channel1.voltdivypos  = CH1_VOLT_DIV_MENU_YPOS;
  scopesettings.channel1.touchedcolor = CH1_TOUCHED_COLOR;
  scopesettings.channel1.buttontext   = "CH1";

  //Set the trace and display buffer pointers for channel 1
  scopesettings.channel1.tracebuffer  = (uint8 *)channel1tracebuffer;
  scopesettings.channel1.tracepoints = channel1pointsbuffer;

  //Set the FPGA commands for channel 2
  scopesettings.channel2.enablecommand     = 0x03;
  scopesettings.channel2.couplingcommand   = 0x37;
  scopesettings.channel2.voltperdivcommand = 0x36;
  scopesettings.channel2.offsetcommand     = 0x35;
  scopesettings.channel2.adc1command       = 0x22;
  scopesettings.channel2.adc2command       = 0x23;

  //Set the menu and button data for channel 2
  scopesettings.channel2.color        = CHANNEL2_COLOR;
  scopesettings.channel2.buttonxpos   = CH2_BUTTON_XPOS;
  scopesettings.channel2.menuxpos     = CH2_MENU_XPOS;
  scopesettings.channel2.voltdivypos  = CH2_VOLT_DIV_MENU_YPOS;
  scopesettings.channel2.touchedcolor = CH2_TOUCHED_COLOR;
  scopesettings.channel2.buttontext   = "CH2";

  //Set the trace and display buffer pointers for channel 2
  scopesettings.channel2.tracebuffer = (uint8 *)channel2tracebuffer;
  scopesettings.channel2.tracepoints = channel2pointsbuffer;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_configuration_data(void)
{
  //Save the settings for writing to the flash
  scope_save_config_data();

#if 0
  //Write it to the flash
  sys_spi_flash_write(0x001FD000, (uint8 *)settingsworkbuffer, sizeof(settingsworkbuffer));
#else
  //Write the data to its sector on the SD card
  sd_card_write(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer);
#endif
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_reset_config_data(void)
{
  uint32 index;
  
  //Load a default configuration in case of settings in flash being corrupted

  //Enable channel 1, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in top part of the screen
  scopesettings.channel1.enable            = 1;
  scopesettings.channel1.coupling          = 0;
  scopesettings.channel1.magnification     = 0;
  scopesettings.channel1.displayvoltperdiv = 0;
  scopesettings.channel1.samplevoltperdiv  = 0;
  scopesettings.channel1.fftenable         = 0;
  scopesettings.channel1.traceposition     = 300;

  //Enable channel 2, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in bottom part of the screen
  scopesettings.channel2.enable            = 1;
  scopesettings.channel2.coupling          = 0;
  scopesettings.channel2.magnification     = 0;
  scopesettings.channel2.displayvoltperdiv = 0;
  scopesettings.channel2.samplevoltperdiv  = 0;
  scopesettings.channel2.fftenable         = 0;
  scopesettings.channel2.traceposition     = 100;

  //Set trigger mode to auto, trigger edge to rising, trigger channel to channel 1, trigger position and trigger screen offset to center of the screen
  scopesettings.triggermode     = 0;
  scopesettings.triggeredge     = 0;
  scopesettings.triggerchannel  = 0;
  scopesettings.triggerhorizontalposition = 362;
  scopesettings.triggerverticalposition   = 200;

  //Set move speed to fast
  scopesettings.movespeed = 0;

  //Set time base to 20uS/div
  scopesettings.timeperdiv = 12;

  //Set the related acquisition speed which is 5MHz
  scopesettings.samplerate = 5;

  //Enable some default measurements
  //Not yet implemented display wise and am thinking of a different way of doing it so left for later

  //Turn time cursor off and set some default positions
  scopesettings.timecursorsenable   = 0;
  scopesettings.timecursor1position = 183;
  scopesettings.timecursor2position = 547;

  //Turn volt cursor of and set some default positions
  scopesettings.voltcursorsenable   = 0;
  scopesettings.voltcursor1position = 167;
  scopesettings.voltcursor2position = 328;

  //Set right menu to normal state
  scopesettings.rightmenustate = 0;

  //Set screen brightness to high, grid brightness to low, always 50% trigger on, x-y display mode off and confirmation mode enabled
  scopesettings.screenbrightness = 80;
  scopesettings.gridbrightness   = 25;
  scopesettings.alwaystrigger50  = 1;
  scopesettings.xymodedisplay    = 0;
  scopesettings.confirmationmode = 1;
  
  //Set default channel calibration values
  for(index=0;index<7;index++)
  {
    //Set FPGA center level
    scopesettings.channel1.dc_calibration_offset[index] = 750;
    scopesettings.channel2.dc_calibration_offset[index] = 750;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_config_data(void)
{
  uint32  channel;
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Set a version number for checking if the settings match the current firmware
  settingsworkbuffer[2] = SETTING_SECTOR_VERSION_HIGH;
  settingsworkbuffer[3] = SETTING_SECTOR_VERSION_LOW;
  
  //Point to the channel 1 settings
  ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];
  
  //Save the channel 1 settings
  *ptr++ = scopesettings.channel1.enable;
  *ptr++ = scopesettings.channel1.coupling;
  *ptr++ = scopesettings.channel1.magnification;
  *ptr++ = scopesettings.channel1.displayvoltperdiv;
  *ptr++ = scopesettings.channel1.samplevoltperdiv;
  *ptr++ = scopesettings.channel1.fftenable;
  *ptr++ = scopesettings.channel1.traceposition;

  //Point to the channel 2 settings
  ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];
  
  //Save the channel 2 settings
  *ptr++ = scopesettings.channel2.enable;
  *ptr++ = scopesettings.channel2.coupling;
  *ptr++ = scopesettings.channel2.magnification;
  *ptr++ = scopesettings.channel2.displayvoltperdiv;
  *ptr++ = scopesettings.channel2.samplevoltperdiv;
  *ptr++ = scopesettings.channel2.fftenable;
  *ptr++ = scopesettings.channel2.traceposition;

  //Point to the trigger settings
  ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];
  
  //Save trigger settings
  *ptr++ = scopesettings.timeperdiv;
  *ptr++ = scopesettings.samplerate;
  *ptr++ = scopesettings.triggermode;
  *ptr++ = scopesettings.triggeredge;
  *ptr++ = scopesettings.triggerchannel;
  *ptr++ = scopesettings.triggerlevel;
  *ptr++ = scopesettings.triggerhorizontalposition;
  *ptr++ = scopesettings.triggerverticalposition;
  
  //Point to the other settings
  ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];
  
  //Save the other settings
  *ptr++ = scopesettings.movespeed;
  *ptr++ = scopesettings.rightmenustate;
  *ptr++ = scopesettings.confirmationmode;
  *ptr++ = scopesettings.screenbrightness;
  *ptr++ = scopesettings.gridbrightness;
  *ptr++ = scopesettings.alwaystrigger50;
  *ptr++ = scopesettings.xymodedisplay;

  //Point to the cursor settings
  ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];
  
  //Save the time cursor settings
  *ptr++ = scopesettings.timecursorsenable;
  *ptr++ = scopesettings.timecursor1position;
  *ptr++ = scopesettings.timecursor2position;

  //Save the volt cursor settings
  *ptr++ = scopesettings.voltcursorsenable;
  *ptr++ = scopesettings.voltcursor1position;
  *ptr++ = scopesettings.voltcursor2position;

  //Point to the first measurement enable setting
  ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

  //Save the measurements enable states
  for(channel=0;channel<2;channel++)
  {
    //12 measurements per channel
    for(index=0;index<12;index++)
    {
      //Copy the current measurement state and point to the next one
      *ptr++ = scopesettings.measuresstate[channel][index];
    }
  }

  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

  //Copy the working set values to the saved values
  for(index=0;index<6;index++,ptr++)
  {
    //Copy the data for both channels
    ptr[0] = scopesettings.channel1.dc_calibration_offset[index];
    ptr[6] = scopesettings.channel2.dc_calibration_offset[index];
  }

  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];
  
  //Save the ADC compensation values
  *ptr++ = scopesettings.channel1.adc1compensation;
  *ptr++ = scopesettings.channel1.adc2compensation;
  *ptr++ = scopesettings.channel2.adc1compensation;
  *ptr++ = scopesettings.channel2.adc2compensation;
  
  //Calculate a checksum over the settings data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Save the checksum
  settingsworkbuffer[0] = checksum >> 16;
  settingsworkbuffer[1] = checksum;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_restore_config_data(void)
{
  uint32  channel;
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Calculate a checksum over the loaded data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Check if the checksum is a match as well as the version number
  if((settingsworkbuffer[0] == (checksum >> 16)) && (settingsworkbuffer[1] == (checksum & 0xFFFF)) && (settingsworkbuffer[2] == SETTING_SECTOR_VERSION_HIGH) && (settingsworkbuffer[3] == SETTING_SECTOR_VERSION_LOW))
  {
    //Point to the channel 1 settings
    ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];

    //Restore the channel 1 settings
    scopesettings.channel1.enable            = *ptr++;
    scopesettings.channel1.coupling          = *ptr++;
    scopesettings.channel1.magnification     = *ptr++;
    scopesettings.channel1.displayvoltperdiv = *ptr++;
    scopesettings.channel1.samplevoltperdiv  = *ptr++;
    scopesettings.channel1.fftenable         = *ptr++;
    scopesettings.channel1.traceposition     = *ptr++;

    //Point to the channel 2 settings
    ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];

    //Restore the channel 2 settings
    scopesettings.channel2.enable            = *ptr++;
    scopesettings.channel2.coupling          = *ptr++;
    scopesettings.channel2.magnification     = *ptr++;
    scopesettings.channel2.displayvoltperdiv = *ptr++;
    scopesettings.channel2.samplevoltperdiv  = *ptr++;
    scopesettings.channel2.fftenable         = *ptr++;
    scopesettings.channel2.traceposition     = *ptr++;

    //Point to the trigger settings
    ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];

    //Restore trigger settings
    scopesettings.timeperdiv                = *ptr++;
    scopesettings.samplerate                = *ptr++;
    scopesettings.triggermode               = *ptr++;
    scopesettings.triggeredge               = *ptr++;
    scopesettings.triggerchannel            = *ptr++;
    scopesettings.triggerlevel              = *ptr++;
    scopesettings.triggerhorizontalposition = *ptr++;
    scopesettings.triggerverticalposition   = *ptr++;

    //Point to the other settings
    ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];

    //Restore the other settings
    scopesettings.movespeed        = *ptr++;
    scopesettings.rightmenustate   = *ptr++;
    scopesettings.confirmationmode = *ptr++;
    scopesettings.screenbrightness = *ptr++;
    scopesettings.gridbrightness   = *ptr++;
    scopesettings.alwaystrigger50  = *ptr++;
    scopesettings.xymodedisplay    = *ptr++;

    //Point to the cursor settings
    ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];

    //Restore the time cursor settings
    scopesettings.timecursorsenable   = *ptr++;
    scopesettings.timecursor1position = *ptr++;
    scopesettings.timecursor2position = *ptr++;

    //Restore the volt cursor settings
    scopesettings.voltcursorsenable   = *ptr++;
    scopesettings.voltcursor1position = *ptr++;
    scopesettings.voltcursor2position = *ptr++;

    //Point to the first measurement enable setting
    ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

    //Restore the measurements enable states
    for(channel=0;channel<2;channel++)
    {
      //12 measurements per channel
      for(index=0;index<12;index++)
      {
        //Copy the current measurement state and point to the next one
        scopesettings.measuresstate[channel][index] = *ptr++;
      }
    }

    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

    //Restore the working set values from the saved values
    for(index=0;index<6;index++,ptr++)
    {
      //Copy the data for both channels
      scopesettings.channel1.dc_calibration_offset[index] = ptr[0];
      scopesettings.channel2.dc_calibration_offset[index] = ptr[6];
    }

    //The last entry is a copy of the fore last value
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];
    
    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];

    //Restore the ADC compensation values
    scopesettings.channel1.adc1compensation = *ptr++;
    scopesettings.channel1.adc2compensation = *ptr++;
    scopesettings.channel2.adc1compensation = *ptr++;
    scopesettings.channel2.adc2compensation = *ptr++;
  }
  else
  {
    //Load a default set on failure
    scope_reset_config_data();
    
    //Save it to the SD card
    scope_save_configuration_data();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

#ifndef USE_TP_CONFIG
#ifdef SAVE_TP_CONFIG
void save_tp_config(void)
{
  //Create a file for the touch panel configuration. Fails if it already exists
  if(f_open(&viewfp, "FNIRSI_1013D_tp_config.bin", FA_CREATE_NEW | FA_WRITE | FA_READ) == FR_OK)
  {
    //Write the touch panel configuration to the sd card
    f_write(&viewfp, tp_config_data, sizeof(tp_config_data), 0);

    //Close the file to finish the write
    f_close(&viewfp);
  }
}
#endif
#endif

//----------------------------------------------------------------------------------------------------------------------------------
