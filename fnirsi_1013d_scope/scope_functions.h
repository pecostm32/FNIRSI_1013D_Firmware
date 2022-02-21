//----------------------------------------------------------------------------------------------------------------------------------

#ifndef SCOPE_FUNCTIONS_H
#define SCOPE_FUNCTIONS_H

//----------------------------------------------------------------------------------------------------------------------------------

#include "fnirsi_1013d_scope.h"
#include "variables.h"

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_display_lib(void);

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_main_screen(void);

void scope_setup_view_screen(void);

void scope_setup_usb_screen(void);

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_right_control_menu(void);

void scope_setup_right_file_menu(void);

//----------------------------------------------------------------------------------------------------------------------------------

void scope_setup_bottom_file_menu(int mode);

//----------------------------------------------------------------------------------------------------------------------------------
// Right side bar functions
//----------------------------------------------------------------------------------------------------------------------------------
  
void scope_control_button(int mode);
void scope_run_stop_button(int mode);
void scope_auto_set_button(int mode);
void scope_previous_wave_button(int mode);
void scope_next_wave_button(int mode);
void scope_t_cursor_button(int mode);
void scope_v_cursor_button(int mode);
void scope_measures_button(int mode);
void scope_save_picture_button(int mode);
void scope_save_wave_button(int mode);
void scope_delete_wave_button(int mode);
void scope_50_percent_trigger_button(int mode);
void scope_show_grid_button(int mode);

void scope_channel_sensitivity_control(PCHANNELSETTINGS settings, int type, int mode);

void scope_return_button(int mode);
void scope_select_all_button(int mode);
void scope_select_button(int mode);
void scope_delete_button(int mode);
void scope_page_up_button(int mode);
void scope_page_down_button(int mode);

//----------------------------------------------------------------------------------------------------------------------------------
// Bitmap control bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_bmp_return_button(int mode);
void scope_bmp_delete_button(int mode);
void scope_bmp_previous_button(int mode);
void scope_bmp_next_button(int mode);

//----------------------------------------------------------------------------------------------------------------------------------
// Top bar functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_menu_button(int mode);
void scope_main_return_button(int mode);
void scope_run_stop_text(void);

void scope_channel_settings(PCHANNELSETTINGS settings, int mode);
void scope_acqusition_settings(int mode);
void scope_move_speed(int mode);
void scope_trigger_settings(int mode);
void scope_battery_status(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Menu functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_main_menu(void);
void scope_main_menu_system_settings(int mode);
void scope_main_menu_picture_view(int mode);
void scope_main_menu_waveform_view(int mode);
void scope_main_menu_usb_connection(int mode);

void scope_open_channel_menu(PCHANNELSETTINGS settings);
void scope_channel_enable_select(PCHANNELSETTINGS settings);
void scope_channel_fft_show(PCHANNELSETTINGS settings);
void scope_channel_coupling_select(PCHANNELSETTINGS settings);
void scope_channel_probe_magnification_select(PCHANNELSETTINGS settings);

void scope_open_acquisition_menu(void);
void scope_acquisition_speed_select(void);
void scope_acquisition_timeperdiv_select(void);

void scope_open_trigger_menu(void);
void scope_trigger_mode_select(void);
void scope_trigger_edge_select(void);
void scope_trigger_channel_select(void);

void scope_open_system_settings_menu(void);
void scope_system_settings_screen_brightness_item(int mode);
void scope_system_settings_screen_brightness_value(void);
void scope_system_settings_grid_brightness_item(int mode);
void scope_system_settings_grid_brightness_value(void);
void scope_system_settings_trigger_50_item(void);
void scope_system_settings_calibration_item(int mode);
void scope_system_settings_x_y_mode_item(void);
void scope_system_settings_confirmation_item(void);

void scope_open_calibration_start_text(void);
void scope_show_calibrating_text(void);
void scope_show_calibration_done_text(void);

void scope_open_measures_menu(void);
void scope_measures_menu_item(int channel, int item);

//----------------------------------------------------------------------------------------------------------------------------------

void scope_open_slider(uint16 xpos, uint16 ypos, uint8 position);
void scope_display_slider(uint16 xpos, uint16 ypos, uint8 position);
int scope_move_slider(uint16 xpos, uint16 ypos, uint8 *position);

void scope_display_slide_button(uint16 xpos, uint16 ypos, uint8 state);

void scope_display_ok_button(uint16 xpos, uint16 ypos, uint8 mode);

//----------------------------------------------------------------------------------------------------------------------------------

void scope_adjust_timebase(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Grid and cursor functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_draw_grid(void);
void scope_draw_pointers(void);
void scope_draw_time_cursors(void);
void scope_draw_volt_cursors(void);

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data processing functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_calculate_trigger_vertical_position();

void scope_acquire_trace_data(void);

void scope_process_trigger(uint32 count);

uint32 scope_do_baseline_calibration(void);
uint32 scope_do_channel_calibration(void);

void scope_do_50_percent_trigger_setup(void);

void scope_do_auto_setup(void);

uint32 scope_check_channel_range(PCHANNELSETTINGS settings);

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data display functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_trace_data(void);

int32 scope_get_x_sample(PCHANNELSETTINGS settings, int32 index);
int32 scope_get_y_sample(PCHANNELSETTINGS settings, int32 index);

void scope_display_channel_trace(PCHANNELSETTINGS settings);

void scope_display_cursor_measurements(void);

void scope_display_measurements(void);
void scope_display_channel_measurements(PCHANNELSETTINGS settings, uint8 *measuresstate, uint32 xstart, uint32 color);

void scope_display_vmax(PCHANNELSETTINGS settings);
void scope_display_vmin(PCHANNELSETTINGS settings);
void scope_display_vavg(PCHANNELSETTINGS settings);
void scope_display_vrms(PCHANNELSETTINGS settings);
void scope_display_vpp(PCHANNELSETTINGS settings);
void scope_display_vp(PCHANNELSETTINGS settings);
void scope_display_freq(PCHANNELSETTINGS settings);
void scope_display_cycle(PCHANNELSETTINGS settings);
void scope_display_time_plus(PCHANNELSETTINGS settings);
void scope_display_time_min(PCHANNELSETTINGS settings);
void scope_display_duty_plus(PCHANNELSETTINGS settings);
void scope_display_duty_min(PCHANNELSETTINGS settings);

void scope_display_voltage(PCHANNELSETTINGS settings, int32 value);

void scope_print_value(char *buffer, int32 value, uint32 scale, char *header, char *sign);

char *scope_print_decimal(char *buffer, int32 value, uint32 decimalplace, uint32 negative);

//----------------------------------------------------------------------------------------------------------------------------------
// File display functions
//----------------------------------------------------------------------------------------------------------------------------------

//These two functions are for save guarding the operational settings when switched to waveform view mode
void scope_save_setup(PSCOPESETTINGS settings);
void scope_restore_setup(PSCOPESETTINGS settings);

//These two functions are for the system settings, preparing for and restoring from file
void scope_prepare_setup_for_file(void);
void scope_restore_setup_from_file(void);
int32 scope_check_waveform_file(void);

void scope_print_file_name(uint32 filenumber);

int32 scope_load_thumbnail_file(void);
int32 scope_save_thumbnail_file(void);

void scope_save_view_item_file(int32 type);

void scope_remove_item_from_thumbnails(uint32 delete);

int32 scope_load_trace_data(void);

int32 scope_load_bitmap_data(void);

void scope_sync_thumbnail_files(void);

void scope_initialize_and_display_thumbnails(void);

void scope_display_thumbnails(void);

void scope_display_thumbnail_data(uint32 xstart, uint32 xend, uint32 ypos, uint32 color, uint8 *buffer);

void scope_create_thumbnail(PTHUMBNAILDATA thumbnaildata);

void scope_thumbnail_set_trace_data(PCHANNELSETTINGS settings, uint8 *buffer);
void scope_thumbnail_calculate_trace_data(int32 xstart, int32 ystart, int32 xend, int32 yend);

void scope_thumbnail_draw_pointer(uint32 xpos, uint32 ypos, uint32 direction, uint32 color);

int32 scope_display_picture_item(void);

void scope_display_selected_signs(void);

void scope_display_file_status_message(int32 msgid, int32 alwayswait);

//----------------------------------------------------------------------------------------------------------------------------------
// Configuration data functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_load_configuration_data(void);
void scope_save_configuration_data(void);

void scope_reset_config_data(void);
void scope_save_config_data(void);
void scope_restore_config_data(void);

//----------------------------------------------------------------------------------------------------------------------------------

#ifndef USE_TP_CONFIG
#ifdef SAVE_TP_CONFIG
void save_tp_config(void);
#endif
#endif

//----------------------------------------------------------------------------------------------------------------------------------

#endif /* SCOPE_FUNCTIONS_H */
