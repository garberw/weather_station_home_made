#ifndef weather_lib_util_ser_print_h
#define weather_lib_util_ser_print_h
#include <weather_lib_util.h>
static serial_print_entry display_table[] = {
  { true, F("weather_dri_wmod.ino"), F("process_upstream_radio") },
  { true, F("weather_wra_pas_co2.cpp"), F("copy_to_wdat") },
  { true, F("weather_lib_counter.h"), F("loop_counter_single") },
  { true, F("weather_lib_util.cpp"), F("copy_flash_to_char") },
  { true, F("weather_lib_counter.h"), F("finished_wind") },
  { true, F("weather_dri_server_light.ino"), F("loop") },
  { true, F("weather_lib_util.cpp"), F("used_now") },
  { true, F("weather_lib_util.cpp"), F("weather_error_return") },
  { true, F("weather_lib_array.h"), F("get_MAX") },
  { true, F("weather_wra_ltr390.cpp"), F("lux_linear") },
  { true, F("weather_lib_data.h"), F("set_iaq_text") },
  { true, F("weather_wra_misol.cpp"), F("count_bucket_tip_ISR") },
  { true, F("weather_lib_modbus.h"), F("buf_compare_CRC") },
  { true, F("weather_lib_modbus.cpp"), F("zero") },
  { true, F("weather_lib_util.cpp"), F("divider_line") },
  { true, F("weather_wra_misol.h"), F("rain_mm") },
  { true, F("weather_lib_counter.h"), F("finished_rain") },
  { true, F("weather_wra_bme688.cpp"), F("print_version") },
  { true, F("weather_lib_data.cpp"), F("get") },
  { true, F("weather_lib_array.h"), F("set") },
  { true, F("weather_lib_radio.cpp"), F("unpack_outside_frame") },
  { true, F("weather_lib_modbus.cpp"), F("select_client_output") },
  { true, F("weather_lib_radio.cpp"), F("select_frame_weewx_out") },
  { true, F("weather_lib_modbus.cpp"), F("read") },
  { true, F("weather_lib_util.cpp"), F("print") },
  { true, F("weather_wra_bme688.cpp"), F("setup") },
  { true, F("weather_lib_modbus.cpp"), F("match_flush_input_buffer") },
  { true, F("weather_lib_util.cpp"), F("divider_line_error_lead") },
  { true, F("weather_lib_radio.cpp"), F("print_tx_frame_to_serial") },
  { true, F("weather_lib_util.h"), F("clear_all") },
  { true, F("weather_wra_tsys.cpp"), F("advanced_read_rain") },
  { true, F("weather_wra_tsl2591.cpp"), F("lux_linear_visible") },
  { true, F("weather_wra_pas_co2.cpp"), F("wait_for_reading") },
  { true, F("weather_dri_client.ino"), F("setup") },
  { true, F("weather_lib_util.cpp"), F("weather_error_catch") },
  { true, F("weather_lib_counter.h"), F("give_up") },
  { true, F("weather_wra_tsl2591.cpp"), F("label_light_rho") },
  { true, F("weather_wra_misol.cpp"), F("advanced_read") },
  { true, F("weather_lib_util.h"), F("serial_print_lock") },
  { true, F("weather_lib_modbus.h"), F("setup_frame_array") },
  { true, F("weather_dri_atlas.ino"), F("loop") },
  { true, F("weather_wra_ltr390.h"), F("integration_time") },
  { true, F("weather_lib_modbus.cpp"), F("setup") },
  { true, F("weather_lib_array.h"), F("get_size") },
  { true, F("weather_lib_util.cpp"), F("setup") },
  { true, F("weather_lib_data.cpp"), F("store") },
  { true, F("weather_lib_counter.h"), F("count_loopc") },
  { true, F("weather_lib_modbus.cpp"), F("crc_debug") },
  { true, F("weather_lib_modbus.cpp"), F("pack_client_request_frame") },
  { true, F("weather_lib_counter.h"), F("print_done") },
  { true, F("weather_lib_counter.h"), F("loop_counter_outside") },
  { true, F("weather_lib_data.cpp"), F("print_idx") },
  { true, F("weather_dri_server_rain.ino"), F("process_upstream_sensor") },
  { true, F("weather_lib_radio.cpp"), F("pack_radio_frame") },
  { true, F("weather_lib_modbus.cpp"), F("pack_weather_data") },
  { true, F("weather_lib_util.cpp"), F("divider_line_error_lead1") },
  { true, F("weather_lib_data.h"), F("print_outside_temp0") },
  { true, F("weather_lib_util.cpp"), F("save_names1") },
  { true, F("weather_lib_modbus.h"), F("frame_idx_fr") },
  { true, F("weather_dri_server_rain.ino"), F("setup") },
  { true, F("weather_lib_modbus.cpp"), F("select_server_temp") },
  { true, F("weather_lib_util.cpp"), F("diagnostic") },
  { true, F("weather_lib_data.cpp"), F("print_entry") },
  { true, F("weather_lib_radio.cpp"), F("unpack_term") },
  { true, F("weather_lib_util.cpp"), F("backtrace") },
  { true, F("weather_wra_ltr390.cpp"), F("UV_index") },
  { true, F("weather_lib_data.h"), F("print_outside_light0") },
  { true, F("weather_lib_radio.cpp"), F("setup") },
  { true, F("weather_lib_modbus.cpp"), F("match_byte") },
  { true, F("weather_lib_radio.cpp"), F("select_frame_radio_out") },
  { true, F("weather_lib_counter.h"), F("count_rain") },
  { true, F("weather_lib_data.cpp"), F("print_outside_sensor") },
  { true, F("weather_dri_wmod.ino"), F("process_upstream_sensor") },
  { true, F("weather_lib_data.h"), F("WIS") },
  { true, F("weather_lib_data.h"), F("print_inside_radio") },
  { true, F("weather_lib_util.cpp"), F("memcpy_pack_float_into_uint16_t") },
  { true, F("weather_lib_radio.cpp"), F("common_frame") },
  { true, F("weather_lib_util.cpp"), F("weather_halt") },
  { true, F("weather_dri_green.ino"), F("setup") },
  { true, F("weather_lib_data.h"), F("print_outside_rain0") },
  { true, F("weather_lib_util.cpp"), F("watchdog_setup") },
  { true, F("weather_dri_wmod.ino"), F("setup") },
  { true, F("weather_lib_array.h"), F("name") },
  { true, F("weather_lib_counter.h"), F("count_wind") },
  { true, F("weather_lib_modbus.cpp"), F("client_tx_rx") },
  { true, F("weather_lib_data.cpp"), F("print_weewx_val") },
  { true, F("weather_lib_data.cpp"), F("print_weewx_WIS") },
  { true, F("weather_lib_radio.cpp"), F("ascii_to_hex") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_word") },
  { true, F("weather_wra_ltr390.cpp"), F("resolution_integration") },
  { true, F("weather_lib_rtc.cpp"), F("sync") },
  { true, F("weather_lib_radio.cpp"), F("pack_outside_done") },
  { true, F("weather_lib_array.h"), F("~weather_array") },
  { true, F("weather_lib_data.cpp"), F("print_item") },
  { true, F("weather_lib_radio.cpp"), F("print_ascii_input_char") },
  { true, F("weather_lib_util.cpp"), F("ser_println") },
  { true, F("weather_lib_util.cpp"), F("setup_communication_ser0") },
  { true, F("weather_wra_bme688.cpp"), F("advanced_read") },
  { true, F("weather_lib_data.h"), F("print_inside_all") },
  { true, F("weather_lib_radio.cpp"), F("pack_header") },
  { true, F("weather_dri_client.ino"), F("process_upstream_modbus") },
  { true, F("weather_lib_data.cpp"), F("print_inside_outside_all_weewx") },
  { true, F("weather_wra_ltr390.cpp"), F("new_data_available") },
  { true, F("weather_wra_ltr390.cpp"), F("gain_factor") },
  { true, F("weather_lib_data.cpp"), F("get_name_mine") },
  { true, F("weather_dri_server_light.ino"), F("process_upstream_sensor") },
  { true, F("weather_lib_data.h"), F("print_outside_light1") },
  { true, F("weather_lib_radio.cpp"), F("unpack_word") },
  { true, F("weather_lib_radio.cpp"), F("check_wacky_macros") },
  { true, F("weather_wra_tsys.cpp"), F("advanced_read") },
  { true, F("weather_lib_modbus.cpp"), F("print") },
  { true, F("weather_dri_server_light.ino"), F("setup") },
  { true, F("weather_lib_counter.h"), F("finished_temp") },
  { true, F("weather_lib_radio.cpp"), F("get_data") },
  { true, F("weather_lib_array.h"), F("setup") },
  { true, F("weather_lib_radio.cpp"), F("unpack_float") },
  { true, F("weather_lib_modbus.cpp"), F("print_ws_wr") },
  { true, F("weather_lib_util.cpp"), F("hash_function") },
  { true, F("weather_lib_radio.cpp"), F("receive_then_ack") },
  { true, F("weather_lib_timer.cpp"), F("stop") },
  { true, F("weather_lib_modbus.h"), F("device_idx_ws") },
  { true, F("weather_lib_util.cpp"), F("check_halt") },
  { true, F("weather_wra_tsl2591.cpp"), F("print_gain") },
  { true, F("weather_lib_modbus.h"), F("buf_pack_CRC") },
  { true, F("weather_lib_modbus.h"), F("weather_ctx_wr") },
  { true, F("weather_lib_util.cpp"), F("memcpy_unpack_uint16_t_into_float") },
  { true, F("weather_wra_ltr390.cpp"), F("setup") },
  { true, F("weather_lib_modbus.h"), F("buf_print") },
  { true, F("weather_lib_util.cpp"), F("store") },
  { true, F("weather_wra_tsys.cpp"), F("setup") },
  { true, F("weather_lib_modbus.cpp"), F("match_gap") },
  { true, F("weather_lib_radio.cpp"), F("transmit_weewx_frame") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_register_float") },
  { true, F("weather_lib_counter.cpp"), F("label_done_temp") },
  { true, F("weather_lib_timer.cpp"), F("start") },
  { true, F("weather_wra_tsl2591.cpp"), F("setup") },
  { true, F("weather_lib_data.cpp"), F("set") },
  { true, F("weather_lib_modbus.cpp"), F("pack_word") },
  { true, F("weather_wra_bme688.cpp"), F("eeprom_test") },
  { true, F("weather_lib_radio.cpp"), F("hex_to_ascii") },
  { true, F("weather_dri_server_rain.ino"), F("process_downstream_modbus") },
  { true, F("weather_wra_tsys.cpp"), F("advanced_read_client") },
  { true, F("weather_wra_misol.cpp"), F("setup_data") },
  { true, F("weather_lib_array.h"), F("at") },
  { true, F("weather_lib_counter.h"), F("set_all_done_false") },
  { true, F("weather_wra_ltr390.cpp"), F("UVI_factor") },
  { true, F("weather_lib_counter.h"), F("finished_all") },
  { true, F("weather_lib_modbus.h"), F("buf_compare_data") },
  { true, F("weather_wra_misol.cpp"), F("setup_pins") },
  { true, F("weather_wra_misol.cpp"), F("copy_to_wdat") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_double_word") },
  { true, F("weather_dri_atlas.ino"), F("setup") },
  { true, F("weather_lib_modbus.cpp"), F("match_modbus") },
  { true, F("weather_wra_tsl2591.cpp"), F("lux_linear_advanced_factor") },
  { true, F("weather_lib_data.cpp"), F("setup") },
  { true, F("weather_wra_tsys.cpp"), F("advanced_read_light") },
  { true, F("weather_lib_radio.cpp"), F("pack_inside_frame") },
  { true, F("weather_lib_data.h"), F("print_outside_tsys_light") },
  { true, F("weather_wra_ltr390.cpp"), F("set_mode_als") },
  { true, F("weather_lib_counter.h"), F("finished") },
  { true, F("weather_lib_data.h"), F("print_outside_rain1") },
  { true, F("weather_dri_client.ino"), F("process_downstream_radio") },
  { true, F("weather_lib_modbus.cpp"), F("calc_timeout") },
  { true, F("weather_lib_modbus.cpp"), F("compare_CRC") },
  { true, F("weather_lib_modbus.cpp"), F("select_sensor") },
  { true, F("weather_lib_radio.cpp"), F("pack_word") },
  { true, F("weather_lib_util.cpp"), F("setup_delay") },
  { true, F("weather_lib_modbus.cpp"), F("modbus_registers") },
  { true, F("weather_lib_modbus.cpp"), F("pack_double_word") },
  { true, F("weather_lib_rtc.cpp"), F("setup") },
  { true, F("weather_lib_radio.cpp"), F("pack_outside_frame") },
  { true, F("weather_wra_pas_co2.cpp"), F("start_next_reading") },
  { true, F("weather_lib_modbus.cpp"), F("static_setup_frame_array") },
  { true, F("weather_lib_modbus.cpp"), F("pack_sensor") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_weather_data_uint16_t") },
  { true, F("weather_lib_util.cpp"), F("watchdog_reset") },
  { true, F("weather_lib_counter.h"), F("set_all_count_zero") },
  { true, F("weather_lib_radio.cpp"), F("at") },
  { true, F("weather_lib_array.h"), F("resize") },
  { true, F("weather_lib_radio.cpp"), F("pack_term") },
  { true, F("weather_wra_tsl2591.cpp"), F("print_timing") },
  { true, F("weather_lib_array.h"), F("get_data") },
  { true, F("weather_lib_radio.cpp"), F("pack_WIS") },
  { true, F("weather_wra_bme688.cpp"), F("update_state") },
  { true, F("weather_lib_util.h"), F("any_set") },
  { true, F("weather_wra_tsl2591.cpp"), F("print_details") },
  { true, F("weather_wra_pas_co2.cpp"), F("advanced_read") },
  { true, F("weather_dri_server_rain.ino"), F("process_upstream_tsys") },
  { true, F("weather_lib_counter.h"), F("finished_tsys") },
  { true, F("weather_lib_data.h"), F("print_outside_tsys_client") },
  { true, F("weather_lib_counter.h"), F("increment") },
  { true, F("weather_lib_data.h"), F("print_outside_tsys_rain") },
  { true, F("weather_lib_data.h"), F("print_outside_all1") },
  { true, F("weather_lib_array.h"), F("weather_array") },
  { true, F("weather_lib_modbus.cpp"), F("crc_debug_sensor") },
  { true, F("weather_lib_modbus.h"), F("buf_write") },
  { true, F("weather_lib_util.h"), F("ser_println") },
  { true, F("weather_lib_util.cpp"), F("setup_communication_ser1") },
  { true, F("weather_wra_ltr390.cpp"), F("set_mode_uvs") },
  { true, F("weather_lib_modbus.h"), F("buf_nread") },
  { true, F("weather_lib_array.h"), F("get") },
  { true, F("weather_lib_counter.h"), F("count_tsys") },
  { true, F("weather_lib_modbus.cpp"), F("server_rx_tx") },
  { true, F("weather_lib_util.cpp"), F("push_used_now") },
  { true, F("weather_lib_util.cpp"), F("check_stack") },
  { true, F("weather_wra_misol.h"), F("new_tips") },
  { true, F("weather_dri_server_light.ino"), F("process_downstream_modbus") },
  { true, F("weather_lib_radio.cpp"), F("select_frame_radio_in") },
  { true, F("weather_lib_modbus.cpp"), F("pack_register_float") },
  { true, F("weather_wra_pas_co2.cpp"), F("setup") },
  { true, F("weather_lib_counter.h"), F("loop_counter_inside") },
  { true, F("weather_lib_util.cpp"), F("pack_long_into_uint16_t") },
  { true, F("weather_wra_tsl2591.cpp"), F("lux_linear_visible_factor") },
  { true, F("weather_wra_tsl2591.cpp"), F("gain_factor") },
  { true, F("weather_lib_data.cpp"), F("print_weewx_VAL") },
  { true, F("weather_wra_ltr390.cpp"), F("advanced_read") },
  { true, F("weather_lib_modbus.h"), F("buf_pack_server_reply_frame") },
  { true, F("weather_lib_util.h"), F("serial_print_lock_cache") },
  { true, F("weather_lib_modbus.cpp"), F("pack_register") },
  { true, F("weather_lib_util.cpp"), F("ser_setup") },
  { true, F("weather_lib_counter.h"), F("count_temp") },
  { true, F("weather_lib_counter.h"), F("finished_light") },
  { true, F("weather_wra_tsl2591.cpp"), F("advanced_read") },
  { true, F("weather_lib_util.cpp"), F("weather_error_condition") },
  { true, F("weather_dri_client.ino"), F("loop") },
  { true, F("weather_wra_bme688.cpp"), F("copy_to_wdat") },
  { true, F("weather_lib_util.cpp"), F("print_hex_16") },
  { true, F("weather_wra_ltr390.cpp"), F("resolution_factor") },
  { true, F("weather_lib_util.h"), F("weather_exception") },
  { true, F("weather_lib_counter.h"), F("count_light") },
  { true, F("weather_lib_radio.cpp"), F("print_frame_to_serial") },
  { true, F("weather_lib_util.h"), F("~serial_print_lock") },
  { true, F("weather_lib_modbus.cpp"), F("pack_server_reply_frame") },
  { true, F("weather_lib_radio.cpp"), F("unpack_WIS") },
  { true, F("weather_lib_radio.cpp"), F("pack_weewx_frame") },
  { true, F("weather_lib_util.cpp"), F("load") },
  { true, F("weather_lib_radio.cpp"), F("unpack_outside_done") },
  { true, F("weather_lib_rtc.cpp"), F("unix_epoch") },
  { true, F("weather_lib_data.cpp"), F("print") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_sensor") },
  { true, F("weather_lib_modbus.cpp"), F("pack_CRC") },
  { true, F("weather_lib_util.cpp"), F("print_uint32") },
  { true, F("weather_wra_bme688.cpp"), F("check_iaq_sensor_status") },
  { true, F("weather_lib_radio.cpp"), F("unpack_header") },
  { true, F("weather_lib_data.h"), F("print_outside_wind0") },
  { true, F("weather_lib_modbus.h"), F("buf_ModRTU_CRC") },
  { true, F("weather_lib_modbus.h"), F("weather_modbus") },
  { true, F("weather_lib_modbus.cpp"), F("setup_buffers") },
  { true, F("weather_lib_modbus.cpp"), F("print_ws") },
  { true, F("weather_lib_util.cpp"), F("divider_line_loop_begin") },
  { true, F("weather_lib_radio.cpp"), F("check_term") },
  { true, F("weather_lib_util.h"), F("serial_print") },
  { true, F("weather_lib_modbus.cpp"), F("select_server_input") },
  { true, F("weather_wra_tsl2591.cpp"), F("integration_time") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_weather_data") },
  { true, F("weather_dri_server_rain.ino"), F("loop") },
  { true, F("weather_lib_modbus.cpp"), F("peek_byte") },
  { true, F("weather_dri_server_light.ino"), F("process_upstream_tsys") },
  { true, F("weather_lib_counter.cpp"), F("label_done_light") },
  { true, F("weather_lib_modbus.cpp"), F("select_server_output") },
  { true, F("weather_lib_util.cpp"), F("divider_line_result") },
  { true, F("weather_lib_modbus.cpp"), F("ModRTU_CRC") },
  { true, F("weather_lib_util.cpp"), F("print_dec_8") },
  { true, F("weather_dri_client.ino"), F("process_upstream_tsys") },
  { true, F("weather_lib_modbus.h"), F("buf_zero") },
  { true, F("weather_lib_util.cpp"), F("save_names2") },
  { true, F("weather_lib_modbus.cpp"), F("select_client_input") },
  { true, F("weather_lib_radio.cpp"), F("transmit_then_ack") },
  { true, F("weather_wra_tsl2591.cpp"), F("configure_sensor") },
  { true, F("weather_lib_util.cpp"), F("pop_used_now") },
  { true, F("weather_lib_data.cpp"), F("print_entry_fixed") },
  { true, F("weather_lib_rtc.cpp"), F("reset_timeout") },
  { true, F("weather_lib_modbus.cpp"), F("pack_byte") },
  { true, F("weather_dri_green.ino"), F("loop") },
  { true, F("weather_lib_util.cpp"), F("pause_on_start") },
  { true, F("weather_wra_bme688.cpp"), F("altitude") },
  { true, F("weather_dri_wmod.ino"), F("loop") },
  { true, F("weather_lib_data.h"), F("print_item") },
  { true, F("weather_lib_timer.h"), F("setup") },
  { true, F("weather_lib_radio.cpp"), F("pack_float") },
  { true, F("weather_dri_wmod.ino"), F("process_downstream_weewx") },
  { true, F("weather_lib_timer.h"), F("weather_timer") },
  { true, F("weather_lib_util.h"), F("ser_print") },
  { true, F("weather_lib_util.cpp"), F("weather_error_fatal") },
  { true, F("weather_lib_util.cpp"), F("print_hex_8") },
  { true, F("weather_lib_modbus.cpp"), F("write") },
  { true, F("weather_wra_tsl2591.cpp"), F("lux_linear_full") },
  { true, F("weather_wra_bme688.cpp"), F("iaq_text") },
  { true, F("weather_wra_misol.h"), F("rain_inch") },
  { true, F("weather_lib_modbus.h"), F("buf_read") },
  { true, F("weather_lib_counter.cpp"), F("label_done_rain") },
  { true, F("weather_lib_modbus.h"), F("buf_pack_client_request_frame") },
  { true, F("weather_lib_counter.cpp"), F("label_done_wind") },
  { true, F("weather_wra_ltr390.cpp"), F("LUX_factor") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_byte") },
  { true, F("weather_lib_util.cpp"), F("divider_line_sensor") },
  { true, F("weather_lib_data.cpp"), F("print_outside_tsys") },
  { true, F("weather_lib_data.cpp"), F("print_idx_fixed") },
  { true, F("weather_lib_data.cpp"), F("set_all_data_zero") },
  { true, F("weather_lib_modbus.cpp"), F("print_wr") },
  { true, F("weather_lib_util.cpp"), F("copy_char_to_char") },
  { true, F("weather_wra_bme688.cpp"), F("load_state") },
  { true, F("weather_wra_tsl2591.cpp"), F("lux_linear_full_factor") },
  { true, F("weather_lib_radio.cpp"), F("print_rx_frame_to_serial") },
  { true, F("weather_lib_radio.cpp"), F("unpack_radio_frame") },
  { true, F("weather_lib_util.cpp"), F("push") },
  { true, F("weather_lib_data.cpp"), F("get_name_weewx") },
  { true, F("weather_lib_rtc.cpp"), F("calc_timeout") },
  { true, F("weather_lib_modbus.cpp"), F("compare_data") },
  { true, F("weather_lib_modbus.cpp"), F("unpack_register") },
  { true, F("weather_lib_data.cpp"), F("print_inside_all_weewx") },
};
#endif
// eee eof
