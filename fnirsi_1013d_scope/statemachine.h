//----------------------------------------------------------------------------------------------------------------------------------

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------

void touch_handler(void);

void scan_for_touch(void);

void handle_main_menu_touch(void);
void handle_channel_menu_touch(PCHANNELSETTINGS settings);
void handle_acquisition_menu_touch(void);
void handle_trigger_menu_touch(void);

void handle_right_basic_menu_touch(void);
void handle_right_volts_div_menu_touch(void);

void handle_measures_menu_touch(void);

void handle_view_mode_touch(void);       //need to finish this function (trace display for waveform view) Check the comments!!
                                         //waveform view functionality also needs to be added to handle_right_basic_menu_touch

void handle_picture_view_touch(void);

int32 handle_confirm_delete(void);

//----------------------------------------------------------------------------------------------------------------------------------

void close_open_menus(uint32 closemain);

//----------------------------------------------------------------------------------------------------------------------------------

void move_trigger_point_position(void);

void change_channel_1_offset(void);
void change_channel_2_offset(void);

void change_trigger_level_offset(void);

void move_left_time_cursor_position(void);
void move_right_time_cursor_position(void);

void move_top_volt_cursor_position(void);
void move_bottom_volt_cursor_position(void);

//----------------------------------------------------------------------------------------------------------------------------------

void match_volt_per_div_settings(PCHANNELSETTINGS settings);

void set_trigger_mode(uint32 mode);

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* STATEMACHINE_H */

