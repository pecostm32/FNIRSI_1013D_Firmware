//----------------------------------------------------------------------------------------------------------------------------------

//Scan the touch panel

//Main state is displaying the scope data

//On touch the first action is to see where the touch is, so an array of boxes might be needed
//Difficulty lies in when a trace or cursor is selected.

//----------------------------------------------------------------------------------------------------------------------------------

#include "statemachine.h"
#include "touchpanel.h"
#include "timer.h"
#include "fpga_control.h"
#include "scope_functions.h"
#include "power_and_battery.h"
#include "display_lib.h"

#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------
//touch handler

void touch_handler(void)
{
  //Check on touch state
  if(touchstate == TOUCH_STATE_INACTIVE)
  {
    //No touch yet so scan for it
    scan_for_touch();

    //When movement of object initiated previous settings need to be saved for updating
    if(touchstate)
    {
      //Check if the trigger point position can be moved
      if(touchstate & TOUCH_STATE_MOVE_TRIGGER_POINT)
      {
        //Save the current position for it
        previous_trigger_point_position = scopesettings.triggerhorizontalposition;
      }

      //Save the data for the selected object
      switch(touchstate & TOUCH_STATE_MASK)
      {
        case TOUCH_STATE_X_Y_MODE:
          previous_channel_1_offset = scopesettings.channel1.traceposition;
          previous_channel_2_offset = scopesettings.channel2.traceposition;
          break;

        case TOUCH_STATE_MOVE_CHANNEL_1:
          previous_channel_1_offset = scopesettings.channel1.traceposition;

          //Check if trigger on this channel and if so save it's current offset
          if(scopesettings.triggerchannel == 0)
          {
            previous_trigger_level_offset = scopesettings.triggerverticalposition;
          }
          break;

        case TOUCH_STATE_MOVE_CHANNEL_2:
          previous_channel_2_offset = scopesettings.channel2.traceposition;

          //Check if trigger on this channel and if so save it's current offset
          if(scopesettings.triggerchannel)
          {
            previous_trigger_level_offset = scopesettings.triggerverticalposition;
          }
          break;

        case TOUCH_STATE_MOVE_TRIGGER_LEVEL:
          previous_trigger_level_offset = scopesettings.triggerverticalposition;
          break;

        case TOUCH_STATE_MOVE_TIME_CURSOR_LEFT:
          previous_left_time_cursor_position = scopesettings.timecursor1position;
          break;

        case TOUCH_STATE_MOVE_TIME_CURSOR_RIGHT:
          previous_right_time_cursor_position = scopesettings.timecursor2position;
          break;

        case TOUCH_STATE_MOVE_VOLT_CURSOR_TOP:
          previous_top_volt_cursor_position = scopesettings.voltcursor1position;
          break;

        case TOUCH_STATE_MOVE_VOLT_CURSOR_BOTTOM:
          previous_bottom_volt_cursor_position = scopesettings.voltcursor2position;
          break;
      }
    }
  }
  else
  {
    //At this point the movement of the traces, cursors and pointers is handled
    //Read the touch panel status to get a new position or be done
    tp_i2c_read_status();

    //When no longer touched reset and quit
    if(havetouch == 0)
    {
      //Signal done with moving
      touchstate = TOUCH_STATE_INACTIVE;

      //That's it for now
      return;
    }

    //Slow things down a bit. This is not a proper solution but will do for now
    timer0_delay(50);

    //Check if the trigger point position can be moved
    if(touchstate & TOUCH_STATE_MOVE_TRIGGER_POINT)
    {
      //Move the trigger point position
      move_trigger_point_position();
    }

    //Handle the selected object
    switch(touchstate & TOUCH_STATE_MASK)
    {
      case TOUCH_STATE_X_Y_MODE:
        change_channel_1_offset();
        change_channel_2_offset();
        break;

      case TOUCH_STATE_MOVE_CHANNEL_1:
        change_channel_1_offset();

        //Check if trigger on this channel and if so move it accordingly
        if(scopesettings.triggerchannel == 0)
        {
          change_trigger_level_offset();
        }
        break;

      case TOUCH_STATE_MOVE_CHANNEL_2:
        change_channel_2_offset();

        //Check if trigger on this channel and if so move it accordingly
        if(scopesettings.triggerchannel)
        {
          change_trigger_level_offset();
        }
        break;

      case TOUCH_STATE_MOVE_TRIGGER_LEVEL:
        change_trigger_level_offset();
        break;

      case TOUCH_STATE_MOVE_TIME_CURSOR_LEFT:
        move_left_time_cursor_position();
        break;

      case TOUCH_STATE_MOVE_TIME_CURSOR_RIGHT:
        move_right_time_cursor_position();
        break;

      case TOUCH_STATE_MOVE_VOLT_CURSOR_TOP:
        move_top_volt_cursor_position();
        break;

      case TOUCH_STATE_MOVE_VOLT_CURSOR_BOTTOM:
        move_bottom_volt_cursor_position();
        break;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
//Top menu bar ranges from 0,0 to 730,46
//Right menu bar ranges from 730,0 to 800,480

void scan_for_touch(void)
{
  uint32 offset;

  //Read the touch panel status
  tp_i2c_read_status();

  //Check if the panel is not touched
  if(havetouch == 0)
  {
    //Quit if so
    return;
  }

  //Draw directly to screen
  display_set_screen_buffer((uint16 *)maindisplaybuffer);

  //Scan for where the touch is applied
  if((xtouch <= 730) && (ytouch <= 46))
  {
    //Top menu bar, so handle that
    //Check if in main menu button range
    if(xtouch <= 80)
    {
      //Check if in normal state
      if(scopesettings.waveviewmode == 0)
      {
        //Set the button active
        scope_menu_button(1);

        //Wait until touch is released
        tp_i2c_wait_for_touch_release();

        //Set the button inactive
        scope_menu_button(0);

        //Save the screen rectangle where the menu will be displayed
        display_set_destination_buffer(displaybuffer2);
        display_copy_rect_from_screen(0, 46, 149, 233);

        //Go and setup the channel 1 menu
        scope_open_main_menu();

        //Go and handle the menu touch
        handle_main_menu_touch();

        //Restore the screen when done
        display_set_source_buffer(displaybuffer2);
        display_copy_rect_to_screen(0, 46, 149, 233);
      }
      else
      {
        //Set the button active
        scope_main_return_button(1);

        //Wait until touch is released
        tp_i2c_wait_for_touch_release();

        //Switch back to the main menu button and set it inactive
        scope_menu_button(0);

        //Reset the wave view mode back to normal state
        scopesettings.waveviewmode = 0;
      }
    }
    //Check if in channel 1 settings button range
    else if((xtouch >= CH1_BUTTON_XPOS) && (xtouch <= CH1_BUTTON_XPOS + CH_BUTTON_BG_WIDTH))
    {
      //Set the button active
      scope_channel_settings(&scopesettings.channel1, 1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Set the button inactive
      scope_channel_settings(&scopesettings.channel1, 0);

      //Save the screen rectangle where the menu will be displayed
      display_set_destination_buffer(displaybuffer2);
      display_copy_rect_from_screen(CH1_MENU_XPOS, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

      //Go and setup the channel 1 menu
      scope_open_channel_menu(&scopesettings.channel1);

      //Go and handle the menu touch
      handle_channel_menu_touch(&scopesettings.channel1);

      //Restore the screen when done
      display_set_source_buffer(displaybuffer2);
      display_copy_rect_to_screen(CH1_MENU_XPOS, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);
    }
    //Check if in channel 2 settings button range
    else if((xtouch >= CH2_BUTTON_XPOS) && (xtouch <= CH2_BUTTON_XPOS + CH_BUTTON_BG_WIDTH))
    {
      //Set the button active
      scope_channel_settings(&scopesettings.channel2, 1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Set the button inactive
      scope_channel_settings(&scopesettings.channel2, 0);

      //Save the screen rectangle where the menu will be displayed
      display_set_destination_buffer(displaybuffer2);
      display_copy_rect_from_screen(CH2_MENU_XPOS, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);

      //Go and setup the channel 1 menu
      scope_open_channel_menu(&scopesettings.channel2);

      //Go and handle the menu touch
      handle_channel_menu_touch(&scopesettings.channel2);

      //Restore the screen when done
      display_set_source_buffer(displaybuffer2);
      display_copy_rect_to_screen(CH2_MENU_XPOS, CH_MENU_YPOS, CH_MENU_WIDTH, CH_MENU_HEIGHT);
    }
    //Check if in acquisition button range
    else if((xtouch >= ACQ_BUTTON_XPOS) && (xtouch <= ACQ_BUTTON_XPOS + ACQ_BUTTON_BG_WIDTH))
    {
      //Set the button active
      scope_acqusition_settings(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Set the button inactive
      scope_acqusition_settings(0);

      //Save the screen rectangle where the menu will be displayed
      display_set_destination_buffer(displaybuffer2);
      display_copy_rect_from_screen(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);

      //Go and setup the channel 1 menu
      scope_open_acquisition_menu();

      //Go and handle the menu touch
      handle_acquisition_menu_touch();

      //Restore the screen when done
      display_set_source_buffer(displaybuffer2);
      display_copy_rect_to_screen(ACQ_MENU_XPOS, ACQ_MENU_YPOS, ACQ_MENU_WIDTH, ACQ_MENU_HEIGHT);
    }
    //Check if in move speed button range
    else if((xtouch >= 493) && (xtouch <= 533))
    {
      //Set the button active
      scope_move_speed(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Toggle the speed
      scopesettings.movespeed ^= 1;

      //Set the button inactive and show the new state
      scope_move_speed(0);
    }
    //Check if in trigger settings button range
    else if((xtouch >= 570) && (xtouch <= 700))
    {
      //Set the button active
      scope_trigger_settings(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Set the button inactive
      scope_trigger_settings(0);

      //Save the screen rectangle where the menu will be displayed
      display_set_destination_buffer(displaybuffer2);
      display_copy_rect_from_screen(560, 46, 172, 186);

      //Go and setup the channel 1 menu
      scope_open_trigger_menu();

      //Go and handle the menu touch
      handle_trigger_menu_touch();

      //Restore the screen when done
      display_set_source_buffer(displaybuffer2);
      display_copy_rect_to_screen(560, 46, 172, 186);
    }
  }
  else if(xtouch > 730)
  {
    //Check if control button is touched
    if((ytouch >= 3) && (ytouch <= 57))
    {
      //Highlight the button if touched
      scope_control_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_control_button(0);

      //Toggle the right menu state
      scopesettings.rightmenustate ^= 1;

      //Display the changed state
      scope_setup_right_control_menu();
    }
    //Not then handle the menus based on the selected menu state
    else
    {
      //Right menu bar, so handle it according to the menu state
      if(scopesettings.rightmenustate == 0)
      {
        //Basic control state
        handle_right_basic_menu_touch();
      }
      else
      {
        //Volts per div adjust state
        handle_right_volts_div_menu_touch();
      }
    }
  }
  else
  {
    //Main screen part, so handle that
    //Get timer ticks to detect touch duration
    previoustimerticks = timer0_get_ticks();

    //Set first touch position for movement detection
    previousxtouch = xtouch;
    previousytouch = ytouch;

    //Wait for touch to end and detect movement within the trace display section
    while(havetouch)
    {
      //Scan the touch panel for touch
      tp_i2c_read_status();

      //Check if there still is touch
      if(havetouch)
      {
        //Calculate the absolute x displacement
        if(xtouch < previousxtouch)
        {
          xtouchdisplacement = previousxtouch - xtouch;
        }
        else
        {
          xtouchdisplacement = xtouch - previousxtouch;
        }

        //Calculate the absolute y displacement
        if(ytouch < previousytouch)
        {
          ytouchdisplacement = previousytouch - ytouch;
        }
        else
        {
          ytouchdisplacement = ytouch - previousytouch;
        }

        //Check which of the two is the biggest
        if(xtouchdisplacement > ytouchdisplacement)
        {
          //Use the x displacement
          maxdisplacement = xtouchdisplacement;
        }
        else
        {
          //Use the y displacement
          maxdisplacement = ytouchdisplacement;
        }

        //Check if moved more than 2 pixels
        if(maxdisplacement > 2)
        {
          //Signal a movement of touch
          touchstate = TOUCH_STATE_HAVE_DISPLACEMENT;

          //Go and handle it
          break;
        }
      }
    }

    //Check if no big displacement detected. If there was, touchstate will not be zero
    if(touchstate == 0)
    {
      //See if it was short touch for time base change. Needs to be less then 200mS for that
      if((timer0_get_ticks() - previoustimerticks) < 200)
      {
        //Change the time base setting
        scope_adjust_timebase();
      }

      //Done for now.
      return;
    }

    //Check if there is displacement that needs to be processed for trace and cursor movement
    if(touchstate)
    {
      //Check if display in normal mode
      if(scopesettings.xymodedisplay == 0)
      {
        //Calculate absolute distances for the active traces and cursors

        //Need to make the trace offset in the same orientation as the touch
        offset = 449 - scopesettings.channel1.traceposition;

        //Check if touch point below the trace
        if(ytouch < offset)
        {
          //If so take the touch point of the offset for the distance
          distance_channel_1 = offset - ytouch;
        }
        else
        {
          //Otherwise take the offset of the touch point for the distance
          distance_channel_1 =  ytouch - offset;
        }

        //Need to make the trace offset in the same orientation as the touch
        offset = 449 - scopesettings.channel2.traceposition;

        //Check if touch point below the trace
        if(ytouch < offset)
        {
          //If so take the touch point of the offset for the distance
          distance_channel_2 = offset - ytouch;
        }
        else
        {
          //Otherwise take the offset of the touch point for the distance
          distance_channel_2 =  ytouch - offset;
        }

        //Check on x below 60 to decide early on which trace to move
        if(xtouch < 60)
        {
          //Check if touch closer to channel 1 center then channel 2 center
          if((distance_channel_1 < 30) && (distance_channel_1 < distance_channel_2))
          {
            //Signal channel 1 trace is being moved and that the trigger point position can also be moved
            touchstate = TOUCH_STATE_MOVE_CHANNEL_1 | TOUCH_STATE_MOVE_TRIGGER_POINT;

            //Go and handle it
            return;
          }
          //Not close to channel 1 or closer to channel 2 so check if close to channel 2
          else if(distance_channel_2 < 30)
          {
            //Signal channel 2 trace is being moved and that the trigger point position can also be moved
            touchstate = TOUCH_STATE_MOVE_CHANNEL_2 | TOUCH_STATE_MOVE_TRIGGER_POINT;

            //Go and handle it
            return;
          }
        }


        //This bit needs to be skipped for time base 200mS/div - 20mS/div
        //Hardware trigger seems to be available only from 10mS/div - 10nS/div



        //Check on x above 660 to decide early on move of trigger level offset
        if(xtouch > 660)
        {
          //Need to make the trigger level offset in the same orientation as the touch
          offset = 448 - scopesettings.triggerverticalposition;

          //Check if touch point below the trace
          if(ytouch < offset)
          {
            //If so take the touch point of the offset for the distance
            distance_trigger_level = offset - ytouch;
          }
          else
          {
            //Otherwise take the offset of the touch point for the distance
            distance_trigger_level =  ytouch - offset;
          }

          //Check if close to trigger level pointer
          if(distance_trigger_level < 30)
          {
            //Signal trigger level is being moved
            touchstate = TOUCH_STATE_MOVE_TRIGGER_LEVEL;

            //Go and handle it
            return;
          }
        }

        //Check if time cursor is enabled
        if(scopesettings.timecursorsenable)
        {
          //Check if touch point left of the left cursor
          if(xtouch < scopesettings.timecursor1position)
          {
            //If so take the touch point of the offset for the distance
            distance_time_cursor_left = scopesettings.timecursor1position - xtouch;
          }
          else
          {
            //Otherwise take the offset of the touch point for the distance
            distance_time_cursor_left =  xtouch - scopesettings.timecursor1position;
          }

          //Check if touch point left of the right cursor
          if(xtouch < scopesettings.timecursor2position)
          {
            //If so take the touch point of the offset for the distance
            distance_time_cursor_right = scopesettings.timecursor2position - xtouch;
          }
          else
          {
            //Otherwise take the offset of the touch point for the distance
            distance_time_cursor_right =  xtouch - scopesettings.timecursor2position;
          }

          //Check if touch closer to left cursor then right cursor
          if((distance_time_cursor_left < 30) && (distance_time_cursor_left < distance_time_cursor_right))
          {
            //Signal left time cursor is being moved
            touchstate = TOUCH_STATE_MOVE_TIME_CURSOR_LEFT;

            //Go and handle it
            return;
          }
          //Not close to left cursor or closer to right cursor so check if close to right cursor
          else if(distance_time_cursor_right < 30)
          {
            //Signal right time cursor is being moved
            touchstate = TOUCH_STATE_MOVE_TIME_CURSOR_RIGHT;

            //Go and handle it
            return;
          }
        }

        //Check if volt cursor is enabled
        if(scopesettings.voltcursorsenable)
        {
          //Check if touch point left of the left cursor
          if(ytouch < scopesettings.voltcursor1position)
          {
            //If so take the touch point of the offset for the distance
            distance_volt_cursor_top = scopesettings.voltcursor1position - ytouch;
          }
          else
          {
            //Otherwise take the offset of the touch point for the distance
            distance_volt_cursor_top =  ytouch - scopesettings.voltcursor1position;
          }

          //Check if touch point left of the right cursor
          if(previousytouch < scopesettings.voltcursor2position)
          {
            //If so take the touch point of the offset for the distance
            distance_volt_cursor_bottom = scopesettings.voltcursor2position - ytouch;
          }
          else
          {
            //Otherwise take the offset of the touch point for the distance
            distance_volt_cursor_bottom =  ytouch - scopesettings.voltcursor2position;
          }

          //Check if touch closer to top cursor then bottom cursor
          if((distance_volt_cursor_top < 30) && (distance_volt_cursor_top < distance_volt_cursor_bottom))
          {
            //Signal top volt cursor is being moved
            touchstate = TOUCH_STATE_MOVE_VOLT_CURSOR_TOP;

            //Go and handle it
            return;
          }
          //Not close to top cursor or closer to bottom cursor so check if close to bottom cursor
          else if(distance_volt_cursor_bottom < 30)
          {
            //Signal bottom volt cursor is being moved
            touchstate = TOUCH_STATE_MOVE_VOLT_CURSOR_BOTTOM;

            //Go and handle it
            return;
          }
        }

        //Down to here for the skip

        //No other object selected so far then check again on the signal traces
        //When channel 1 or channel2 or only x movement signal that the trigger point can also be moved.
        touchstate = TOUCH_STATE_MOVE_TRIGGER_POINT;

        //Check if touch closer to channel 1 center then channel 2 center
        if((distance_channel_1 < 30) && (distance_channel_1 < distance_channel_2))
        {
          //Signal channel 1 trace is being moved
          touchstate |= TOUCH_STATE_MOVE_CHANNEL_1;
        }
        //Not close to channel 1 or closer to channel 2 so check if close to channel 2
        else if(distance_channel_2 < 30)
        {
          //Signal channel 2 trace is being moved
          touchstate |= TOUCH_STATE_MOVE_CHANNEL_2;
        }
      }
      else
      {
        //Display in x-y mode, so signal this in touchstate
        touchstate = TOUCH_STATE_X_Y_MODE;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_main_menu_touch(void)
{
  //Stay in the menu as long as there is no touch outside the menu
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the menu field
      if((xtouch >= 2) && (xtouch <= 149) && (ytouch >= 46) && (ytouch <= 279))
      {
        //Check if on system settings
        if((ytouch >= 46) && (ytouch <= 105))
        {
          //Check if already open
          if(systemsettingsmenuopen == 0)
          {
            //Set the button active
            scope_main_menu_system_settings(1);

            //Wait until touch is released
            tp_i2c_wait_for_touch_release();

            //Save the screen under the menu
            display_set_destination_buffer(displaybuffer2);
            display_copy_rect_from_screen(150, 46, 244, 353);

            //Show the system settings menu
            scope_open_system_settings_menu();

            //Signal the system settings menu is opened
            systemsettingsmenuopen = 1;
          }
        }
        //Check if on picture view
        else if((ytouch >= 107) && (ytouch <= 164))
        {
          //Close any other open menu
          close_open_menus(1);

          //Set the button active
          scope_main_menu_picture_view(1);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Signal viewing of pictures
          viewtype = VIEW_TYPE_PICTURE;

          //Switch to picture view screen and stay there until return is pressed
          scope_setup_view_screen();

          //Need to exit the loop here
          return;
        }
        //Check if on waveform view
        else if((ytouch >= 166) && (ytouch <= 223))
        {
          //Close any other open menu
          close_open_menus(1);

          //Set the button active
          scope_main_menu_waveform_view(1);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Signal viewing of pictures
          viewtype = VIEW_TYPE_WAVEFORM;

          //Switch to waveform view screen and stay there until return is pressed
          scope_setup_view_screen();

          //Need to exit the loop here
          return;
        }
        //Check if on USB connection
        else if((ytouch >= 225) && (ytouch <= 278))
        {
          //Close any other open menu
          close_open_menus(1);

          //Set the button active
          scope_main_menu_usb_connection(1);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Restore the main screen before entering the USB screen
          display_set_source_buffer(displaybuffer2);
          display_copy_rect_to_screen(0, 46, 149, 233);

          //And make a copy of it to return to
          display_set_destination_buffer(displaybuffer2);
          display_copy_rect_from_screen(0, 0, 800, 480);

          //Show and handle the usb connection here
          scope_setup_usb_screen();

          //Copy back the original screen to continue were left of
          display_set_source_buffer(displaybuffer2);
          display_copy_rect_to_screen(0, 0, 800, 480);

          //Need to exit the loop here
          return;
        }
      }
      //Check on system settings menu opened and being touched
      else if(systemsettingsmenuopen && (xtouch >= 150) && (xtouch <= 394) && (ytouch >= 46) && (ytouch <= 399))
      {
        //Check if on screen brightness
        if((ytouch >= 47) && (ytouch <= 103))
        {
          //Check if already open
          if(screenbrightnessopen == 0)
          {
            //Close any of the sub menus if open
            close_open_menus(0);

            //Set this item active
            scope_system_settings_screen_brightness_item(1);

            //Wait until touch is released
            tp_i2c_wait_for_touch_release();

            //Show the screen brightness slider
            scope_open_slider(395, 46, scopesettings.screenbrightness);

            //Signal the screen brightness slider is opened
            screenbrightnessopen = 1;
          }
        }
        //Check if on grid brightness
        else if((ytouch >= 105) && (ytouch <= 162))
        {
          //Check if already open
          if(gridbrightnessopen == 0)
          {
            //Close any of the sub menus if open
            close_open_menus(0);

            //Set this item active
            scope_system_settings_grid_brightness_item(1);

            //Wait until touch is released
            tp_i2c_wait_for_touch_release();

            //Show the screen brightness slider
            scope_open_slider(395, 104, scopesettings.gridbrightness);

            //Signal the screen brightness slider is opened
            gridbrightnessopen = 1;
          }
        }
        //Check if on always trigger 50%
        else if((ytouch >= 164) && (ytouch <= 221))
        {
          //Close any of the sub menus if open
          close_open_menus(0);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Toggle the always trigger 50% state
          scopesettings.alwaystrigger50 ^= 1;

          //Show the state
          scope_display_slide_button(326, 183, scopesettings.alwaystrigger50);
        }
        //Check if on baseline calibration
        else if((ytouch >= 223) && (ytouch <= 280))
        {
          //Check if already open
          if(calibrationopen == 0)
          {
            //Close any of the sub menus if open
            close_open_menus(0);

            //Set this item active
            scope_system_settings_calibration_item(1);

            //Wait until touch is released
            tp_i2c_wait_for_touch_release();

            //Show the start text
            scope_open_calibration_start_text();

            //Signal the calibration text is opened
            calibrationopen = 1;
          }
        }
        //Check if on x-y mode display
        else if((ytouch >= 282) && (ytouch <= 339))
        {
          //Close any of the sub menus if open
          close_open_menus(0);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Toggle the x-y mode display state
          scopesettings.xymodedisplay ^= 1;

          //Show the state
          scope_display_slide_button(326, 299, scopesettings.xymodedisplay);
        }
        //check on notification confirmation
        else if((ytouch >= 341) && (ytouch <= 398))
        {
          //Close any of the sub menus if open
          close_open_menus(0);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Toggle the notification confirmation mode
          scopesettings.confirmationmode ^= 1;

          //Show the state
          scope_display_slide_button(326, 358, scopesettings.confirmationmode);
        }
      }
      //Check on screen brightness slider opened and being touched
      else if(screenbrightnessopen && (xtouch >= 395) && (xtouch <= 726) && (ytouch >= 46) && (ytouch <= 104))
      {
        //Move the slider to a new position and check if there was a change in position
        if(scope_move_slider(395, 46, &scopesettings.screenbrightness))
        {
          //Update the setting in the system settings menu
          scope_system_settings_screen_brightness_value();

          //Update the actual screen brightness
          fpga_set_translated_brightness();
        }
      }
      //Check on grid brightness slider opened and being touched
      else if(gridbrightnessopen && (xtouch >= 395) && (xtouch <= 726) && (ytouch >= 105) && (ytouch <= 163))
      {
        //Move the slider to a new position and check if there was a change in position
        if(scope_move_slider(395, 104, &scopesettings.gridbrightness))
        {
          //Update the setting in the system settings menu
          scope_system_settings_grid_brightness_value();

          //Update the actual grid brightness if needed. There is no direct update of the background!
        }
      }
      //Check on calibration start text opened and being touched
      else if((calibrationopen == 1) && (xtouch >= 395) && (xtouch <= 594) && (ytouch >= 223) && (ytouch <= 280))
      {
        //Check if touch is on the button
        if((xtouch >= 517) && (xtouch <= 583) && (ytouch >= 230) && (ytouch <= 272))
        {
          //Highlight the button
          scope_display_ok_button(517, 230, 1);

          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Show the baseline calibration active text
          scope_show_calibrating_text();
          
          //Start the calibration process
          if(scope_do_baseline_calibration() == 1)
          {
            //Need to write the calibration and configuration data to FLASH
//          signal_sys_ok();
          }

          //Need a variable for the calib status and the next function should be adapted to show failure when needed
          
          //Show the calibration successful text if all went well
          scope_show_calibration_done_text();

          //Signal calibration has been done
          calibrationopen = 2;
        }
      }
      //Check on calibration done text opened and being touched
      else if((calibrationopen == 2) && (xtouch >= 395) && (xtouch <= 505) && (ytouch >= 223) && (ytouch <= 280))
      {
        //Nothing to do here so wait until touch is released
        tp_i2c_wait_for_touch_release();
      }
      else
      {
        //Close any of the menus if open
        close_open_menus(1);

        //Touch outside the menu so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_channel_menu_touch(PCHANNELSETTINGS settings)
{
  //Stay in the menu as long as there is no touch outside the menu
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the menu field
      if((xtouch >= settings->menuxpos) && (xtouch <= settings->menuxpos + CH_MENU_WIDTH) && (ytouch >= CH_MENU_YPOS) && (ytouch <= CH_MENU_YPOS + CH_MENU_HEIGHT))
      {
        //Check on channel enable or disable
        if((ytouch >= CH_MENU_YPOS + 16) && (ytouch <= CH_MENU_YPOS + 38))
        {
          //Check on enable
          if((xtouch >= settings->menuxpos + 78) && (xtouch <= settings->menuxpos + 110))
          {
            //Enable the channel
            settings->enable = 1;

            //Update the trigger channel selection in the FPGA as needed
            fpga_swap_trigger_channel();

            //Set the trigger vertical position to match the trigger channel position
            scope_calculate_trigger_vertical_position();
            
            //Display this
            scope_channel_enable_select(settings);
            scope_channel_settings(settings, 0);
            scope_trigger_settings(0);
          }
          //Check on disable
          else if((xtouch >= settings->menuxpos + 130) && (xtouch <= settings->menuxpos + 162))
          {
            //Disable the channel
            settings->enable = 0;

            //Update the trigger channel selection in the FPGA as needed
            fpga_swap_trigger_channel();

            //Set the trigger vertical position to match the trigger channel position
            scope_calculate_trigger_vertical_position();
            
#if 0
            //Reset the volts/div setting to max
            settings->displayvoltperdiv = 0;

            //Only when in runstate 0 this should be copied to the FPGA
            
            //Set volts per div in the FPGA
            fpga_set_channel_voltperdiv(settings);
            
            //Also needs setting of the channel offset in hte FPGA
#endif

            //Display this
            scope_channel_enable_select(settings);
            scope_channel_settings(settings, 0);
            scope_trigger_settings(0);
          }
        }
        //Check on fft enable or disable
        else if((ytouch >= CH_MENU_YPOS + 78) && (ytouch <= CH_MENU_YPOS + 100))
        {
          //Check on enable
          if((xtouch >= settings->menuxpos + 78) && (xtouch <= settings->menuxpos + 110))
          {
            //Enable the channel
            settings->fftenable = 1;

            //Display this
            scope_channel_fft_show(settings);
          }
          //Check on disable
          else if((xtouch >= settings->menuxpos + 130) && (xtouch <= settings->menuxpos + 162))
          {
            //Disable the channel
            settings->fftenable = 0;

            //Display this
            scope_channel_fft_show(settings);
          }
        }
        //Check on coupling DC or AD, and in normal view mode
        else if((ytouch >= CH_MENU_YPOS + 142) && (ytouch <= CH_MENU_YPOS + 164) && (scopesettings.waveviewmode == 0))
        {
          //Check on DC coupling
          if((xtouch >= settings->menuxpos + 78) && (xtouch <= settings->menuxpos + 110))
          {
            //Set the channel to DC coupling
            settings->coupling = 0;

            //Update the FPGA
            fpga_set_channel_coupling(settings);

            //Display this
            scope_channel_coupling_select(settings);
            scope_channel_settings(settings, 0);
          }
          //Check on AC coupling
          else if((xtouch >= settings->menuxpos + 130) && (xtouch <= settings->menuxpos + 162))
          {
            //Set the channel to AC coupling
            settings->coupling = 1;

            //Update the FPGA
            fpga_set_channel_coupling(settings);

            //Display this
            scope_channel_coupling_select(settings);
            scope_channel_settings(settings, 0);
          }
        }
        //Check on probe magnification setting, and in normal view mode
        else if((ytouch >= CH_MENU_YPOS + 199) && (ytouch <= CH_MENU_YPOS + 237) && (scopesettings.waveviewmode == 0))
        {
          //Check on 1x
          if((xtouch >= settings->menuxpos + 78) && (xtouch <= settings->menuxpos + 98))
          {
            //Enable the channel
            settings->magnification = 0;

            //Display this
            scope_channel_probe_magnification_select(settings);
            scope_channel_settings(settings, 0);
          }
          //Check on 10x
          else if((xtouch >= settings->menuxpos + 109) && (xtouch <= settings->menuxpos + 132))
          {
            //Disable the channel
            settings->magnification = 1;

            //Display this
            scope_channel_probe_magnification_select(settings);
            scope_channel_settings(settings, 0);
          }
          //Check on 100x
          else if((xtouch >= settings->menuxpos + 138) && (xtouch <= settings->menuxpos + 168))
          {
            //Disable the channel
            settings->magnification = 2;

            //Display this
            scope_channel_probe_magnification_select(settings);
            scope_channel_settings(settings, 0);
          }
        }

        //Wait until touch is released before checking on a new position
        tp_i2c_wait_for_touch_release();
      }
      else
      {
        //Touch outside the menu so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_acquisition_menu_touch(void)
{
  uint32 i,x,y;

  //Stay in the menu as long as there is no touch outside the menu
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the menu field
      if((xtouch >= ACQ_MENU_XPOS) && (xtouch <= ACQ_MENU_XPOS + ACQ_MENU_WIDTH) && (ytouch >= ACQ_MENU_YPOS) && (ytouch <= ACQ_MENU_YPOS + ACQ_MENU_HEIGHT))
      {
        //Check on acquisition frequency being set
        if((ytouch >= ACQ_MENU_YPOS + 32) && (ytouch <= ACQ_MENU_YPOS + 149))
        {
          //Check if in run state. Changing this setting is only allowed when running
          if(scopesettings.runstate == 0)
          {
            //Check on the different bounding boxes for the separate settings
            for(i=0;i<(sizeof(acquisition_speed_texts) / sizeof(int8 *));i++)
            {
              //Calculate the coordinates for this setting
              x = ((i & 3) * 72) + 10 + ACQ_MENU_XPOS;
              y = ((i >> 2) * 23) + 33 + ACQ_MENU_YPOS;

              //Check if touch within this bounding box
              if((xtouch >= x) && (xtouch <= x + 68) && (ytouch >= y) && (ytouch <= y + 20))
              {
                //Set the new speed
                scopesettings.samplerate = i;

                //Set the FPGA sample rate setting without changing the current time per div setting
                //This way the sampling is actually done with the set speed, but the display stays at the set time per div
                fpga_set_sample_rate(i);

                //Display the new setting
                scope_acquisition_speed_select();

                //Update the viable time per div settings
                scope_acquisition_timeperdiv_select();

                //Update the top menu bar display
                scope_acqusition_settings(0);
                break;
              }
            }
          }
        }
        //Check on time per div being set
        else if((ytouch >= ACQ_MENU_YPOS + 190) && (ytouch <= ACQ_MENU_YPOS + 329))
        {
          //Check on the different bounding boxes for the separate settings
          for(i=0;i<(sizeof(time_div_texts) / sizeof(int8 *));i++)
          {
            //Calculate the coordinates for this setting
            x = ((i & 3) * 72) + 10 + ACQ_MENU_XPOS;
            y = ((i >> 2) * 23) + 191 + ACQ_MENU_YPOS;

            //Check if touch within this bounding box
            if((xtouch >= x) && (xtouch <= x + 68) && (ytouch >= y) && (ytouch <= y + 20))
            {
              //Set the new time per div
              scopesettings.timeperdiv = ((sizeof(time_div_texts) / sizeof(int8 *)) - 1) - i;

              //Display the new setting
              scope_acquisition_timeperdiv_select();
              scope_acqusition_settings(0);
              break;
            }
          }
        }

        //Wait until touch is released before checking on a new position
        tp_i2c_wait_for_touch_release();
      }
      else
      {
        //Touch outside the menu so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_trigger_menu_touch(void)
{
  //Stay in the menu as long as there is no touch outside the menu
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the menu field
      if((xtouch >= 560) && (xtouch <= 732) && (ytouch >= 46) && (ytouch <= 232))
      {
        //Check on trigger mode, and not in waveform view
        if((ytouch >= 57) && (ytouch <= 95) && (scopesettings.waveviewmode == 0))
        {
          //Check on auto
          if((xtouch >= 629) && (xtouch <= 649))
          {
            //Set to auto mode
            set_trigger_mode(0);
          }
          //Check on single
          else if((xtouch >= 661) && (xtouch <= 681))
          {
            //Set to single mode
            set_trigger_mode(1);
          }
          //Check on normal
          else if((xtouch >= 692) && (xtouch <= 713))
          {
            //Set to normal mode
            set_trigger_mode(2);
          }
        }
        //Check on trigger edge, and not in waveform view
        else if((ytouch >= 125) && (ytouch <= 147) && (scopesettings.waveviewmode == 0))
        {
          //Check on rising
          if((xtouch >= 626) && (xtouch <= 666))
          {
            //Set the trigger edge to rising
            scopesettings.triggeredge = 0;

            //Update the FPGA
            fpga_set_trigger_edge();

            //Display this
            scope_trigger_edge_select();
            scope_trigger_settings(0);
          }
          //Check on falling
          else if((xtouch >= 671) && (xtouch <= 716))
          {
            //Set the trigger edge to falling
            scopesettings.triggeredge = 1;

            //Update the FPGA
            fpga_set_trigger_edge();

            //Display this
            scope_trigger_edge_select();
            scope_trigger_settings(0);
          }
        }
        //Check on trigger channel, and not in waveform view
        else if((ytouch >= 188) && (ytouch <= 210) && (scopesettings.waveviewmode == 0))
        {
          //Check on channel 1
          if((xtouch >= 632) && (xtouch <= 664))
          {
            //Only when the channel is enabled
            if(scopesettings.channel1.enable)
            {
              //Set the channel 1 as trigger source
              scopesettings.triggerchannel = 0;

              //Update the FPGA
              fpga_set_trigger_channel();
              
              //Set the trigger vertical position to match channel 1 trace position
              scope_calculate_trigger_vertical_position();

              //Display this
              scope_trigger_channel_select();
              scope_trigger_settings(0);
            }
          }
          //Check on channel 2
          else if((xtouch >= 680) && (xtouch <= 712))
          {
            //Only when the channel is enabled
            if(scopesettings.channel2.enable)
            {
              //Set channel 2 as trigger source
              scopesettings.triggerchannel = 1;

              //Update the FPGA
              fpga_set_trigger_channel();

              //Set the trigger vertical position to match channel 2 trace position
              scope_calculate_trigger_vertical_position();

              //Display this
              scope_trigger_channel_select();
              scope_trigger_settings(0);
            }
          }
        }

        //Wait until touch is released before checking on a new position
        tp_i2c_wait_for_touch_release();
      }
      else
      {
        //Touch outside the menu so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_right_basic_menu_touch(void)
{
  //Check if run/stop or page up button is touched
  if((ytouch >= 63) && (ytouch <= 117))
  {
    //Check on wave view state for which button is shown
    if(scopesettings.waveviewmode == 0)
    {
      //Run/stop so highlight that button if touched
      scope_run_stop_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_run_stop_button(0);

      //Toggle the run state
      scopesettings.runstate ^= 1;

      //When the scope is switched back to running state, change the sample rate to the current time per div setting
      if(scopesettings.runstate == 0)
      {
        //Set the sample rate that belongs to the selected time per div setting
        scopesettings.samplerate = time_per_div_sample_rate[scopesettings.timeperdiv];
        
        //Show this on the screen
        scope_acqusition_settings(0);
        
        //Match the volts per division settings for both channels
        //Is needed when vertical zoom has been used
        match_volt_per_div_settings(&scopesettings.channel1);
        match_volt_per_div_settings(&scopesettings.channel2);
      }

      //Display the changed state
      scope_run_stop_text();
    }
    else
    {
      //Previous waveform so highlight that button if touched
      scope_previous_wave_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_previous_wave_button(0);

      //Check if not on first item
      if(viewcurrentindex > 0)
      {
        //Select the previous one
        viewcurrentindex--;

        //Load the waveform data for the newly selected item
        if(scope_load_trace_data() == VIEW_TRACE_LOAD_ERROR)
        {
          //On error signal need to return to main item view mode
          scopesettings.waveviewmode = 0;
        }
      }
    }
  }
  //Check if auto set or page down button is touched
  else if((ytouch >= 123) && (ytouch <= 177))
  {
    //Check on wave view state for which button is shown
    if(scopesettings.waveviewmode == 0)
    {
      //Auto set so highlight that button if touched
      scope_auto_set_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_auto_set_button(0);

      //Run auto setup
      scope_do_auto_setup();
    }
    else
    {
      //Next waveform so highlight that button if touched
      scope_next_wave_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_next_wave_button(0);

      //Check if not on last item
      //Index starts on zero, available items on 1
      if(viewcurrentindex < (viewavailableitems - 1))
      {
        //Select the next one
        viewcurrentindex++;

        //Load the waveform data for the newly selected item
        if(scope_load_trace_data() == VIEW_TRACE_LOAD_ERROR)
        {
          //On error signal need to return to main item view mode
          scopesettings.waveviewmode = 0;
        }
      }
    }
  }
  //Check if time cursor button is touched
  else if((ytouch >= 183) && (ytouch <= 237))
  {
    //Highlight the button if touched
    scope_t_cursor_button(1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_t_cursor_button(0);

    //Toggle the enable setting
    scopesettings.timecursorsenable ^= 1;
  }
  //Check if volt cursor button is touched
  else if((ytouch >= 243) && (ytouch <= 297))
  {
    //Highlight the button if touched
    scope_v_cursor_button(1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_v_cursor_button(0);

    //Toggle the enable setting
    scopesettings.voltcursorsenable ^= 1;
  }
  //Check if measures button is touched
  else if((ytouch >= 303) && (ytouch <= 357))
  {
    //Highlight the button if touched
    scope_measures_button(1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_measures_button(0);

    //Save the screen rectangle where the menu will be displayed
    display_set_destination_buffer(displaybuffer2);
    display_copy_rect_from_screen(231, 263, 499, 214);

    //Go and setup the channel 1 menu
    scope_open_measures_menu();

    //Go and handle the menu touch
    handle_measures_menu_touch();

    //Restore the screen when done
    display_set_source_buffer(displaybuffer2);
    display_copy_rect_to_screen(231, 263, 499, 214);
  }
  //Check if save picture button is touched
  else if((ytouch >= 363) && (ytouch <= 417))
  {
    //Highlight the button if touched
    scope_save_picture_button(1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_save_picture_button(0);

    //Save the picture on the SD card
    scope_save_view_item_file(VIEW_TYPE_PICTURE);
  }
  //Check if save or delete wave button is touched
  else if((ytouch >= 423) && (ytouch <= 477))
  {
    //Check on wave view state for which button is shown
    if(scopesettings.waveviewmode == 0)
    {
      //Save wave so highlight that button if touched
      scope_save_wave_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_save_wave_button(0);

      //Save the waveform on the SD card
      scope_save_view_item_file(VIEW_TYPE_WAVEFORM);
    }
    else
    {
      //Delete wave so highlight that button if touched
      scope_delete_wave_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_delete_wave_button(0);

      //Ask user for confirmation of the delete
      if(handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
      {
        //Need to remove this item from the thumbnails
        scope_remove_item_from_thumbnails(1);

        //save the thumbnail file
        scope_save_thumbnail_file();

        //No more items available then return
        if(viewavailableitems == 0)
        {
          //Signal need to return to main item view mode
          scopesettings.waveviewmode = 0;
        }
        else
        {
          //Otherwise need to see if there is a next image to display or the previous one needs to be used
          if(viewcurrentindex >= viewavailableitems)
          {
            //Use the last available one
            //Index starts on zero, available items on 1
            viewcurrentindex = viewavailableitems - 1;
          }

          //Load the waveform data for the newly selected item
          if(scope_load_trace_data() == VIEW_TRACE_LOAD_ERROR)
          {
            //On error signal need to return to main item view mode
            scopesettings.waveviewmode = 0;
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_right_volts_div_menu_touch(void)
{
  //Check if channel 1 V+ button is touched
  if((ytouch >= 78) && (ytouch <= 140))
  {
    //Highlight the button if touched
    scope_channel_sensitivity_control(&scopesettings.channel1, 0, 1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_channel_sensitivity_control(&scopesettings.channel1, 0, 0);

    //Only if channel is enabled
    if(scopesettings.channel1.enable)
    {
      //Check if setting not already on max
      if(scopesettings.channel1.displayvoltperdiv < 6)
      {
        //Step up to the next setting. (Lowering the setting)
        scopesettings.channel1.displayvoltperdiv++;

        //Show the change on the screen
        scope_channel_settings(&scopesettings.channel1, 0);

        //Only update the FPGA in run mode
        //For waveform view mode the stop state is forced and can't be changed
        if(scopesettings.runstate == 0)
        {
          //Copy the display setting to the sample setting
          scopesettings.channel1.samplevoltperdiv = scopesettings.channel1.displayvoltperdiv;
          
          //Set the volts per div for this channel
          fpga_set_channel_voltperdiv(&scopesettings.channel1);
          
          //Since the DC offset is influenced set that too
          fpga_set_channel_offset(&scopesettings.channel1);

          //Wait 50ms to allow the circuit to settle
          timer0_delay(50);
        }
      }
    }
  }
  //Check if channel 1 V- button is touched
  else if((ytouch >= 163) && (ytouch <= 223))
  {
    //Highlight the button if touched
    scope_channel_sensitivity_control(&scopesettings.channel1, 1, 1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_channel_sensitivity_control(&scopesettings.channel1, 1, 0);

    //Only if channel is enabled
    if(scopesettings.channel1.enable)
    {
      //Check if setting not already on min
      if(scopesettings.channel1.displayvoltperdiv > 0)
      {
        //Original code has some limit on updating when the scope is not running on the time base setting or a previous stored volts per div setting
        //The latter maybe has to do with a stored wave that is being looked at
        //For now just allowing constant change within the max limit

        //Step down to the next setting. (Increasing the setting)
        scopesettings.channel1.displayvoltperdiv--;

        //Show the change on the screen
        scope_channel_settings(&scopesettings.channel1, 0);

        //Only update the FPGA in run mode
        //For waveform view mode the stop state is forced and can't be changed
        if(scopesettings.runstate == 0)
        {
          //Copy the display setting to the sample setting
          scopesettings.channel1.samplevoltperdiv = scopesettings.channel1.displayvoltperdiv;
          
          //Set the volts per div for this channel
          fpga_set_channel_voltperdiv(&scopesettings.channel1);
          
          //Since the DC offset is influenced set that too
          fpga_set_channel_offset(&scopesettings.channel1);

          //Wait 50ms to allow the circuit to settle
          timer0_delay(50);
        }
      }
    }
  }
  //Check if channel 2 V+ button is touched
  else if((ytouch >= 258) && (ytouch <= 320))
  {
    //Highlight the button if touched
    scope_channel_sensitivity_control(&scopesettings.channel2, 0, 1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_channel_sensitivity_control(&scopesettings.channel2, 0, 0);

    //Only if channel is enabled
    if(scopesettings.channel2.enable)
    {
      //Check if setting not already on max
      if(scopesettings.channel2.displayvoltperdiv < 6)
      {
        //Step up to the next setting. (Lowering the setting)
        scopesettings.channel2.displayvoltperdiv++;

        //Show the change on the screen
        scope_channel_settings(&scopesettings.channel2, 0);

        //Only update the FPGA in run mode
        //For waveform view mode the stop state is forced and can't be changed
        if(scopesettings.runstate == 0)
        {
          //Copy the display setting to the sample setting
          scopesettings.channel2.samplevoltperdiv = scopesettings.channel2.displayvoltperdiv;
          
          //Set the volts per div for this channel
          fpga_set_channel_voltperdiv(&scopesettings.channel2);
          
          //Since the DC offset is influenced set that too
          fpga_set_channel_offset(&scopesettings.channel2);

          //Wait 50ms to allow the circuit to settle
          timer0_delay(50);
        }
      }
    }
  }
  //Check if channel 2 V- button is touched
  else if((ytouch >= 343) && (ytouch <= 403))
  {
    //Highlight the button if touched
    scope_channel_sensitivity_control(&scopesettings.channel2, 1, 1);

    //Wait until touch is released
    tp_i2c_wait_for_touch_release();

    //Button back to inactive state
    scope_channel_sensitivity_control(&scopesettings.channel2, 1, 0);

    //Only if channel is enabled
    if(scopesettings.channel2.enable)
    {
      //Check if setting not already on min
      if(scopesettings.channel2.displayvoltperdiv > 0)
      {
        //Step down to the next setting. (Increasing the setting)
        scopesettings.channel2.displayvoltperdiv--;

        //Show the change on the screen
        scope_channel_settings(&scopesettings.channel2, 0);

        //Only update the FPGA in run mode
        //For waveform view mode the stop state is forced and can't be changed
        if(scopesettings.runstate == 0)
        {
          //Copy the display setting to the sample setting
          scopesettings.channel2.samplevoltperdiv = scopesettings.channel2.displayvoltperdiv;
          
          //Set the volts per div for this channel
          fpga_set_channel_voltperdiv(&scopesettings.channel2);
          
          //Since the DC offset is influenced set that too
          fpga_set_channel_offset(&scopesettings.channel2);

          //Wait 50ms to allow the circuit to settle
          timer0_delay(50);
        }
      }
    }
  }
  //Check if 50% trigger or show grid button is touched
  else if((ytouch >= 423) && (ytouch <= 477))
  {
    //Check on wave view state for which button is shown
    if(scopesettings.waveviewmode == 0)
    {
      //50% trigger so highlight that button if touched
      scope_50_percent_trigger_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_50_percent_trigger_button(0);

      //Only when scope is running
      if(scopesettings.runstate == 0)
      {
        //Set the trigger level for the active trigger channel
        scope_do_50_percent_trigger_setup();
      }
    }
    else
    {
      //Show grid so highlight that button if touched
      scope_show_grid_button(1);

      //Wait until touch is released
      tp_i2c_wait_for_touch_release();

      //Button back to inactive state
      scope_show_grid_button(0);
      
      //Toggle the gird enable status
      scopesettings.gridenable ^= 1;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

const TOUCHCOORDS measures_item_touch_coords[2][12] =
{
  {
    //Channel 1 coordinates are on the right
    {482, 543, 289, 349}, {545, 605, 289, 349}, {607, 667, 289, 349}, {669, 729, 289, 349},
    {482, 543, 351, 411}, {545, 605, 351, 411}, {607, 667, 351, 411}, {669, 729, 351, 411},
    {482, 543, 413, 476}, {545, 605, 413, 476}, {607, 667, 413, 476}, {669, 729, 413, 476},
  },
  {
    //Channel 2 coordinates are on the left
    {232, 293, 289, 349}, {295, 355, 289, 349}, {357, 417, 289, 349}, {418, 480, 289, 349},
    {232, 293, 351, 411}, {295, 355, 351, 411}, {357, 417, 351, 411}, {418, 480, 351, 411},
    {232, 293, 413, 476}, {295, 355, 413, 476}, {357, 417, 413, 476}, {418, 480, 413, 476},
  }
};

//----------------------------------------------------------------------------------------------------------------------------------

void handle_measures_menu_touch(void)
{
  int channel;
  int item;
  int found;

  //Stay in the menu as long as there is no touch outside the menu
  while(1)
  {
    //Scan the touch panel for touch
    tp_i2c_read_status();

    //Check if there is touch
    if(havetouch)
    {
      //Check if touch within the menu field
      if((xtouch >= 231) && (xtouch <= 730) && (ytouch >= 264) && (ytouch <= 477))
      {
        found = 0;
        channel = 0;

        //Check the touch positions for the separate items until one is found
        while((found == 0) && (channel < 2))
        {
          item = 0;

          //For each channel 12 items
          while((found == 0) && (item<12))
          {
            //Check if touch is on this item
            if((xtouch >= measures_item_touch_coords[channel][item].x1) && (xtouch <= measures_item_touch_coords[channel][item].x2) &&
               (ytouch >= measures_item_touch_coords[channel][item].y1) && (ytouch <= measures_item_touch_coords[channel][item].y2))
            {
              //Toggle the item
              scopesettings.measuresstate[channel][item] ^= 1;

              //There is an array for showing the items on screen. It works on a first come first serve system.
              //It looks for the first free spot and inserts itself there. This gives a messy image of the two channels interleaving with settings
              //On disable the list is shifted to fill the gap
              //For this an items in list counter might be useful

              //A nicer setup is to reserve separate sections per channel on the screen for the enabled items and always show them in a fixed
              //order. Makes reading them easier


              //Draw the changed item
              scope_measures_menu_item(channel, item);

              //Signal item found
              found = 1;
            }

            //Next item
            item++;
          }

          //Next channel
          channel++;
        }

        //Wait until touch is released before checking on a new position
        tp_i2c_wait_for_touch_release();
      }
      else
      {
        //Touch outside the menu so wait until touch is released and then quit
        tp_i2c_wait_for_touch_release();

        return;
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

const TOUCHCOORDS view_item_touch_coords[16] =
{
  {  4, 178,   4, 116}, {186, 360,   4, 116}, {368, 542,   4, 116}, {550, 726,   4, 116},
  {  4, 178, 124, 236}, {186, 360, 124, 236}, {368, 542, 124, 236}, {550, 726, 124, 236},
  {  4, 178, 244, 356}, {186, 360, 244, 356}, {368, 542, 244, 356}, {550, 726, 244, 356},
  {  4, 178, 364, 476}, {186, 360, 364, 476}, {368, 542, 364, 476}, {550, 726, 364, 476},
};

//----------------------------------------------------------------------------------------------------------------------------------

void handle_view_mode_touch(void)
{
  int32 index;
  int32 found;

  //Stay in this mode as long as the return is not touched
  while(1)
  {
    //Read the touch panel status
    tp_i2c_read_status();

    //Check if the panel is touched
    if(havetouch)
    {
      //Check if touch is in right menu area.
      if((xtouch >= 730))
      {
        //Check if the return button is touched
        if((ytouch >= 4) && (ytouch <= 76))
        {
          //Highlight the button to show it is touched
          scope_return_button(1);
          
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();

          //Back to inactive state
          scope_return_button(0);
          
          //DOne with item view so return
          return;
        }
        //Else check if the select all button is touched
        else if((ytouch >= 84) && (ytouch <= 156))
        {
          //Highlight the button
          scope_select_all_button(1);
          
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();
          
          //Deselect the possibly enabled select button
          scope_select_button(0);

          //Depending on the state take action
          if((viewselectmode == VIEW_SELECT_NONE) || (viewselectmode == VIEW_SELECT_INDIVIDUAL))
          {
            //Enable the button
            scope_select_all_button(2);

            //Switch to all items selected mode
            viewselectmode = VIEW_SELECT_ALL;

            //Make sure all selected signs are displayed
            for(index=0;index<viewitemsonpage;index++)
            {
              //Check if the selected sign is not selected
              if(viewitemselected[index] == VIEW_ITEM_NOT_SELECTED)
              {
                //If so make sure it gets displayed
                viewitemselected[index] = VIEW_ITEM_SELECTED_NOT_DISPLAYED;
              }
            }
          }
          else
          {
            //Make sure the button is no longer highlighted
            scope_select_all_button(0);

            //Switch to all items deselected mode
            viewselectmode = VIEW_SELECT_NONE;

            //Not in a selected mode any more so clear the active selected signs
            for(index=0;index<viewitemsonpage;index++)
            {
              //Check if the selected sign is actually displayed
              if(viewitemselected[index] == VIEW_ITEM_SELECTED_DISPLAYED)
              {
                //If so make sure it gets cleared
                viewitemselected[index] = VIEW_ITEM_NOT_SELECTED_DISPLAYED;
              }
            }
          }

          //Update the selected signs
          scope_display_selected_signs();
        }
        //Else check if the select button is touched
        else if((ytouch >= 164) && (ytouch <= 236))
        {
          //Highlight the button to show it is touched
          scope_select_button(1);
            
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();
          
          //Deselect the possibly highlighted select all button
          scope_select_all_button(0);

          //Check if all items are selected or select mode is on
          if(viewselectmode)
          {
            //Make sure the select button is not highlighted
            scope_select_button(0);

            //Switch to all items deselected mode
            viewselectmode = VIEW_SELECT_NONE;

            //Not in a selected mode any more so clear the active selected signs
            for(index=0;index<viewitemsonpage;index++)
            {
              //Check if the selected sign is actually displayed
              if(viewitemselected[index] == VIEW_ITEM_SELECTED_DISPLAYED)
              {
                //If so make sure it gets cleared
                viewitemselected[index] = VIEW_ITEM_NOT_SELECTED_DISPLAYED;
              }
            }
          }
          else
          {
            //Nothing selected so switch to individual select mode
            viewselectmode = VIEW_SELECT_INDIVIDUAL;

            //Highlight the select button
            scope_select_button(2);
          }

          //Update the selected signs
          scope_display_selected_signs();
        }
        //Else check if the delete button is touched
        else if((ytouch >= 244) && (ytouch <= 316))
        {
          //Highlight the button to show it is touched
          scope_delete_button(1);
            
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();
          
          //Back to inactive button
          scope_delete_button(0);
          
          //Need to see if there are items selected for delete
          if(viewselectmode)
          {
            //A select mode is active so check the list to see if there are items selected
            //This is not done in the original code. Activate the single select mode and without selecting an item press delete. The confirm menu is shown
            for(index=0,found=0;index<viewitemsonpage;index++)
            {
              //Check if the current item is selected
              if(viewitemselected[index] == VIEW_ITEM_SELECTED_DISPLAYED)
              {
                //Signal there is at least one item to delete
                found = 1;
                break;
              }
            }

            //Check if there is an item to delete
            if(found)
            {
              //Ask the user if the items should be deleted
              if(handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
              {
                //User opted for delete so do this for the selected items
                //Start with the last item on the page to avoid problems with the file number list being modified
                for(index=viewitemsonpage-1;index>=0;index--)
                {
                  //Check if the current item is selected
                  if(viewitemselected[index] == VIEW_ITEM_SELECTED_DISPLAYED)
                  {
                    //Set the current index for this file
                    viewcurrentindex = index + (viewpage * VIEW_ITEMS_PER_PAGE);

                    //Remove the current item from the thumbnails and delete the item from disk
                    scope_remove_item_from_thumbnails(1);
                  }
                }

                //Save the thumbnail file
                scope_save_thumbnail_file();

                //Clear the select flags
                memset(viewitemselected, VIEW_ITEM_NOT_SELECTED, VIEW_ITEMS_PER_PAGE);

                //Clear the select state and the button highlights
                viewselectmode = VIEW_SELECT_NONE;
                scope_select_all_button(0);
                scope_select_button(0);

                //Redisplay the thumbnails
                scope_initialize_and_display_thumbnails();
              }
            }
          }
        }
        //Else check if the page up button is touched
        else if((ytouch >= 324) && (ytouch <= 396))
        {
          //Highlight the button to show it is touched
          scope_page_up_button(1);
            
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();
          
          //Back to inactive button
          scope_page_up_button(0);
          
          //Check if there is a previous page
          if(viewpage > 0)
          {
            //Clear the select mode
            viewselectmode = 0;

            //Deselect the possibly highlighted buttons
            scope_select_all_button(0);
            scope_select_button(0);

            //Clear the item selected flags
            memset(viewitemselected, 0, VIEW_ITEMS_PER_PAGE);

            //Select the previous page
            viewpage--;

            //Display the previous page of thumbnails
            scope_display_thumbnails();
          }
        }
        //Else check if the page down button is touched
        else if((ytouch >= 404) && (ytouch <= 476))
        {
          //Highlight the button to show it is touched
          scope_page_down_button(1);
            
          //Wait until touch is released
          tp_i2c_wait_for_touch_release();
          
          //Back to inactive button
          scope_page_down_button(0);
          
          //Check if there is a next page
          if(viewpage < viewpages)
          {
            //Clear the select mode
            viewselectmode = 0;

            //Deselect the possibly highlighted buttons
            scope_select_all_button(0);
            scope_select_button(0);

            //Clear the item selected flags
            memset(viewitemselected, 0, VIEW_ITEMS_PER_PAGE);

            //Select the next page
            viewpage++;

            //Display the next page of thumbnails
            scope_display_thumbnails();
          }
        }
      }
      else
      {
        //Start with no item touched yet and check on first item
        found = 0;
        index = 0;

        //Do until found or last available item checked
        while(index < viewitemsonpage)
        {
          //Check if touch is on this item
          if((xtouch >= view_item_touch_coords[index].x1) && (xtouch <= view_item_touch_coords[index].x2) &&
             (ytouch >= view_item_touch_coords[index].y1) && (ytouch <= view_item_touch_coords[index].y2))
          {
            //Signal item found
            found = 1;

            //Quit the loop when found
            break;
          }

          //Next item
          index++;
        }

        //Need to wait for touch to release before selecting or opening the item
        tp_i2c_wait_for_touch_release();
        
        //Check if touch was on one of the available items
        if(found)
        {
          //Check if in a select mode
          if(viewselectmode)
          {
            //Yes, so toggle the select on this item
            if(viewitemselected[index] == VIEW_ITEM_SELECTED_DISPLAYED)
            {
              //If displayed make sure it gets cleared
              viewitemselected[index] = VIEW_ITEM_NOT_SELECTED_DISPLAYED;
            }
            else
            {
              //If not make sure it gets displayed
              viewitemselected[index] = VIEW_ITEM_SELECTED_NOT_DISPLAYED;
            }

            //Update the selected signs
            //Could improve on this with a single sign update function
            scope_display_selected_signs();
          }
          else
          {
            //Set the current index for this file
            viewcurrentindex = index + (viewpage * VIEW_ITEMS_PER_PAGE);

            //Loaded ok, so take action based on the type of the opened item
            if(viewtype == VIEW_TYPE_PICTURE)
            {
              //Load the bitmap in the screen memory
              if(scope_load_bitmap_data() == VIEW_BITMAP_LOAD_OK)
              {
                //On initialization draw the bottom menu bar with a save of the background
                scope_setup_bottom_file_menu(VIEW_BOTTON_MENU_INIT);

                //Handle the touch
                handle_picture_view_touch();
              }
            }
            else
            {
              //Try to load the trace data for the file indicated by the current index and view type
              if(scope_load_trace_data() == VIEW_TRACE_LOAD_OK)
              {
                //waveform view needs to be handled here as long as the flag is set
                while(scopesettings.waveviewmode)
                {
                  //Use the main touch handler for waveform view
                  touch_handler();

                  //Update the display as needed
                  scope_display_trace_data();

                  //Monitor the battery status
                  battery_check_status();
                }
              }
            }

            //Display the file actions menu on the right side of the screen
            scope_setup_right_file_menu();

            //Display the available thumbnails for the current view type
            scope_initialize_and_display_thumbnails();
          }
        }
      }
    }

    //Need to wait for touch to release before checking again
    tp_i2c_wait_for_touch_release();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void handle_picture_view_touch(void)
{
  //Stay in this mode as long as the return is not touched
  while(1)
  {
    //Read the touch panel status
    tp_i2c_read_status();

    //Check if the panel is touched
    if(havetouch)
    {
      //Check if bottom view menu is shown
      if(viewbottommenustate & VIEW_BOTTON_MENU_SHOW)
      {
        //Check if touch outside of the menu bar
        if(ytouch < 420)
        {
          //Wait until touch is released before taking action
          tp_i2c_wait_for_touch_release();
          
          //Toggle the menu visibility
          viewbottommenustate ^= VIEW_BOTTON_MENU_SHOW;

          //Update accordingly
          scope_setup_bottom_file_menu(viewbottommenustate);
        }
        else
        {
          //Check if in return button region
          if((xtouch > 40) && (xtouch < 160))
          {
            //Highlight the button
            scope_bmp_return_button(1);
            
            //Wait until touch is released before taking action
            tp_i2c_wait_for_touch_release();

            //Back to inactive state
            scope_bmp_return_button(0);
            
            //Clear the menu state. This is needed for the emulator
            viewbottommenustate = VIEW_BOTTON_MENU_HIDE;
            
            //Just return
            return;
          }
          //Check if in delete button region
          else if((xtouch > 240) && (xtouch < 360))
          {
            //Highlight the button
            scope_bmp_delete_button(1);
            
            //Wait until touch is released before taking action
            tp_i2c_wait_for_touch_release();

            //Back to inactive state
            scope_bmp_delete_button(0);
            
            //Ask user for confirmation of the delete
            if(handle_confirm_delete() == VIEW_CONFIRM_DELETE_YES)
            {
              //Need to remove this item from the thumbnails
              scope_remove_item_from_thumbnails(1);

              //save the thumbnails file
              scope_save_thumbnail_file();

              //No more items available then return
              if(viewavailableitems == 0)
              {
                return;
              }

              //Then need to see if there is a next image to display or the previous one needs to be used
              if(viewcurrentindex >= viewavailableitems)
              {
                //Use the last available one
                //Index starts on zero, available items on 1
                viewcurrentindex = viewavailableitems - 1;
              }

              //Display the new picture item
              if(scope_display_picture_item() == VIEW_BITMAP_LOAD_ERROR)
              {
                //On error return to main view
                return;
              }
            }
          }
          //Check if in previous item button region
          else if((xtouch > 440) && (xtouch < 560))
          {
            //Highlight the button
            scope_bmp_previous_button(1);
            
            //Wait until touch is released before taking action
            tp_i2c_wait_for_touch_release();

            //Back to inactive state
            scope_bmp_previous_button(0);
            
            //Check if not on first item
            if(viewcurrentindex > 0)
            {
              //Select the previous one
              viewcurrentindex--;

              //Display the new picture item
              if(scope_display_picture_item() == VIEW_BITMAP_LOAD_ERROR)
              {
                //On error return to main view
                return;
              }
            }
          }
          //Check if in next item button region
          else if((xtouch > 640) && (xtouch < 760))
          {
            //Highlight the button
            scope_bmp_next_button(1);
            
            //Wait until touch is released before taking action
            tp_i2c_wait_for_touch_release();

            //Back to inactive state
            scope_bmp_next_button(0);
            
            //Check if not on last item
            //Index starts on zero, available items on 1
            if(viewcurrentindex < (viewavailableitems - 1))
            {
              //Select the next one
              viewcurrentindex++;

              //Display the new picture item
              if(scope_display_picture_item() == VIEW_BITMAP_LOAD_ERROR)
              {
                //On error return to main view
                return;
              }
            }
          }
        }
      }
      else
      {
        //If not shown, show it again
        scope_setup_bottom_file_menu(VIEW_BOTTON_MENU_SHOW);
      }
    }

    //Need to wait for touch to release before checking again
    tp_i2c_wait_for_touch_release();
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 handle_confirm_delete(void)
{
  int32 choice = 0;

  //Save the screen rectangle where the menu will be displayed
  display_copy_rect_from_screen(310, 192, 180, 96);

  //display the confirm delete menu
  //Draw the background in grey
  display_set_fg_color(0x00202020);
  display_fill_rect(310, 192, 180, 96);

  //Draw the border in a lighter grey
  display_set_fg_color(0x00303030);
  display_draw_rect(310, 192, 180, 96);

  //Draw the buttons
  display_fill_rounded_rect(320, 228, 74, 50, 2);
  display_fill_rounded_rect(405, 228, 74, 50, 2);

  //White color for text and use font_3
  display_set_fg_color(0x00FFFFFF);
  display_set_font(&font_3);
  display_text(340, 204, "Confirm to delete?");
  display_text(348, 246, "NO");
  display_text(431, 246, "YES");

  //wait for touch
  while(1)
  {
    //Read the touch panel status
    tp_i2c_read_status();

    //Check if the panel is touched
    if(havetouch)
    {
      //Check if touch is on "NO"
      if((xtouch >= 324) && (xtouch <= 390) && (ytouch >= 230) && (ytouch <= 276))
      {
        //Highlight the button
        display_set_fg_color(ITEM_ACTIVE_COLOR);
        display_fill_rounded_rect(320, 228, 74, 50, 2);
        
        //With the text in black
        display_set_fg_color(0x00000000);
        display_text(348, 246, "NO");
        
        //Set the chosen option
        choice = VIEW_CONFIRM_DELETE_NO;

        //Done so quit the loop
        break;
      }
      //Else check if touch on "YES"
      else if((xtouch >= 409) && (xtouch <= 475) && (ytouch >= 230) && (ytouch <= 276))
      {
        //Highlight the button
        display_set_fg_color(ITEM_ACTIVE_COLOR);
        display_fill_rounded_rect(405, 228, 74, 50, 2);
        
        //With the text in black
        display_set_fg_color(0x00000000);
        display_text(431, 246, "YES");
        
        //Set the chosen option
        choice = VIEW_CONFIRM_DELETE_YES;

        //Done so quit the loop
        break;
      }

      //Need to wait for touch to release before checking again
      tp_i2c_wait_for_touch_release();
    }
  }

  //Need to wait for touch to release before returning
  tp_i2c_wait_for_touch_release();

  //Restore the original screen
  display_copy_rect_to_screen(310, 192, 180, 96);

  //return the choice
  return(choice);
}

//----------------------------------------------------------------------------------------------------------------------------------

void close_open_menus(uint32 closemain)
{
  //Only one of these menu's will be opened at any time

  //Check if the screen brightness slider is open
  if(screenbrightnessopen)
  {
    //Set the button inactive
    scope_system_settings_screen_brightness_item(0);

    //Restore the screen under the screen brightness slider
    display_set_source_buffer(displaybuffer2);
    display_copy_rect_to_screen(395, 46, 331, 58);

    //Signal it is closed
    screenbrightnessopen = 0;
  }
  //Check if the grid brightness slider is open
  else if(gridbrightnessopen)
  {
    //Set the button inactive
    scope_system_settings_grid_brightness_item(0);

    //Restore the screen under the grid brightness slider
    display_set_source_buffer(displaybuffer2);
    display_copy_rect_to_screen(395, 104, 331, 58);

    //Signal it is closed
    gridbrightnessopen = 0;
  }
  //Check if the calibration text is open
  else if(calibrationopen)
  {
    //Set the button inactive
    scope_system_settings_calibration_item(0);

    //Restore the screen under the calibration text
    display_set_source_buffer(displaybuffer2);
    display_copy_rect_to_screen(395, 222, 199, 59);

    //Signal it is closed
    calibrationopen = 0;
  }

  //Check if system settings menu has been opened and needs to be closed
  if(closemain && systemsettingsmenuopen)
  {
    //Restore the screen under the system settings menu when done
    display_set_source_buffer(displaybuffer2);
    display_copy_rect_to_screen(150, 46, 244, 353);

    //Clear the flag so it will be opened next time
    systemsettingsmenuopen = 0;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void move_trigger_point_position(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = xtouch - previousxtouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_trigger_point_position + diff;

  //Limit it on the trace portion of the screen
  if(position < 2)
  {
    //So not below 2
    position = 2;
  }
  else if(position > 712)
  {
    //And not above 712;
    position = 712;
  }

  //Update the current position
  scopesettings.triggerhorizontalposition = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void change_channel_1_offset(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with based on the display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //For normal display use y displacement
    diff = ytouch - previousytouch;
  }
  else
  {
    //For x-y mode display use x displacement
    diff = previousxtouch - xtouch;
  }

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_channel_1_offset - diff;

  //Limit it on the trace portion of the screen
  if(position < 6)
  {
    //So not below 6
    position = 6;
  }
  else if(position > 394)
  {
    //And not above 394;
    position = 394;
  }

  //Update the current position
  scopesettings.channel1.traceposition = position;

  //Write it to the FPGA
  fpga_set_channel_offset(&scopesettings.channel1);
}

//----------------------------------------------------------------------------------------------------------------------------------

void change_channel_2_offset(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = ytouch - previousytouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_channel_2_offset - diff;

  //Limit it on the trace portion of the screen
  if(position < 6)
  {
    //So not below 6
    position = 6;
  }
  else if(position > 394)
  {
    //And not above 394;
    position = 394;
  }

  //Update the current position
  scopesettings.channel2.traceposition = position;

  //Write it to the FPGA
  fpga_set_channel_offset(&scopesettings.channel2);
}

//----------------------------------------------------------------------------------------------------------------------------------

void change_trigger_level_offset(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = ytouch - previousytouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_trigger_level_offset - diff;

  //Limit it on the trace portion of the screen
  if(position < 6)
  {
    //So not below 6
    position = 6;
  }
  else if(position > 394)
  {
    //And not above 394;
    position = 394;
  }

  //Update the current position
  scopesettings.triggerverticalposition = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void move_left_time_cursor_position(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = xtouch - previousxtouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_left_time_cursor_position + diff;

  //Limit it on the trace portion of the screen and the right time cursor
  if(position < 3)
  {
    //So not below 3
    position = 3;
  }
  else if(position >= scopesettings.timecursor2position)
  {
    //And not right of the right cursor;
    position = scopesettings.timecursor2position - 1;
  }

  //Update the current position
  scopesettings.timecursor1position = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void move_right_time_cursor_position(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = xtouch - previousxtouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_right_time_cursor_position + diff;

  //Limit it on the trace portion of the screen and the left cursor
  if(position <= scopesettings.timecursor1position)
  {
    //So not left of the left cursor
    position = scopesettings.timecursor1position + 1;
  }
  else if(position > 722)
  {
    //And not beyond end of the screen;
    position = 722;
  }

  //Update the current position
  scopesettings.timecursor2position = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void move_top_volt_cursor_position(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = ytouch - previousytouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_top_volt_cursor_position + diff;

  //Limit it on the trace portion of the screen and not below the bottom cursor
  if(position < 47)
  {
    //So not below 47
    position = 47;
  }
  else if(position >= scopesettings.voltcursor2position)
  {
    //And not below the bottom cursor;
    position = scopesettings.voltcursor2position - 1;
  }

  //Update the current position
  scopesettings.voltcursor1position = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void move_bottom_volt_cursor_position(void)
{
  int32 diff;
  int32 position;

  //Calculate the distance to move the setting with
  diff = ytouch - previousytouch;

  //Make it based on move speed
  if(scopesettings.movespeed)
  {
    //For slow divide by 5
    diff /= 5;
  }

  //Calculate the new position
  position = (int32)previous_bottom_volt_cursor_position + diff;

  //Limit it on the trace portion of the screen and not above the top cursor
  if(position <= scopesettings.voltcursor1position)
  {
    //So not above the top cursor
    position = scopesettings.voltcursor1position + 1;
  }
  else if(position > 447)
  {
    //And not above 447;
    position = 447;
  }

  //Update the current position
  scopesettings.voltcursor2position = position;
}

//----------------------------------------------------------------------------------------------------------------------------------

void match_volt_per_div_settings(PCHANNELSETTINGS settings)
{
  //Check if the sample volts per division differs from the display volts per division setting
  if(settings->samplevoltperdiv != settings->displayvoltperdiv)
  {
    //Match them if so and change them in the FPGA
    settings->samplevoltperdiv = settings->displayvoltperdiv;

    //Set the volts per div for this channel
    fpga_set_channel_voltperdiv(settings);

    //Since the DC offset is influenced set that too
    fpga_set_channel_offset(settings);

    //Wait 50ms to allow the circuit to settle
    timer0_delay(50);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void set_trigger_mode(uint32 mode)
{
  //Set the new mode
  scopesettings.triggermode = mode;

  //Set the new mode in the hardware
  fpga_set_trigger_mode();

  //Make sure the scope is running
  scopesettings.runstate = 0;

  //Show this on the screen
  scope_run_stop_text();

  //Match the volts per division settings for both channels
  //Is needed when vertical zoom has been used
  match_volt_per_div_settings(&scopesettings.channel1);
  match_volt_per_div_settings(&scopesettings.channel2);
  
  //Display this
  scope_trigger_mode_select();
  scope_trigger_settings(0);
}

//----------------------------------------------------------------------------------------------------------------------------------
