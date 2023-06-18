#ifndef weather_lib_radio_h
#define weather_lib_radio_h
/*
  Weather Radio

  This sketch helps use LoRa radio to relay weather sensor data.
  The modbus RTU client reads from the modbus RTU servers (weather sensors).
  The modbus RTU client also acts as a LoRa server.
  The LoRa server outputs sensor values to the serial monitor and
  also relays or transmits (TX) to the arduino LoRa client (RX) inside the house.  
  The sensors include:

  temperature and humidity 
  wind
  light and UV
  rain

  This client arduino (RX) is hooked up by usb to a computer running weewx.
  (fixme specify weewx driver).
*/

#include <weather_lib_data.h>
#include <weather_lib_counter.h>

// fixme RADIO_FRAME_MAX not in header

class weather_radio {
public:
  // bool m_blink_led;
  // void setup(bool blink_led = false);
  void setup(void);
  void pack_weewx_frame(weather_data &wdat,
                        loop_counter_outside &loopc_out,
                        loop_counter_inside &loopc_in);
  void pack_radio_frame(weather_data &wdat, loop_counter_outside &loopc_out);
  void unpack_radio_frame(weather_data &wdat, loop_counter_outside &loopc_out);
  int  transmit_then_ack(void);
  int  receive_then_ack(void);
  void transmit_weewx_frame(void);
private:
  int  check_term(int idx);
  int  check_wacky_macros(void);
  void pack_word(int &idx, bool done, uint16_t w);
  void unpack_word(int &idx, bool done, uint16_t &w);
  void pack_float(int &idx, bool done, float fff);
  void unpack_float(int &idx, bool done, float &fff);
  void pack_header(int &idx);
  void pack_term(int &idx);
  void unpack_header(int &idx);
  void unpack_term(int &idx);
  void pack_outside_done(int &idx, loop_counter_outside &loopc_out);
  void unpack_outside_done(int &idx, loop_counter_outside &loopc_out);
  void pack_WIS(int &idx, weather_data &wdat, bool done, WIS &idx_set);
  void unpack_WIS(int &idx, weather_data &wdat, bool done, WIS &idx_set);
  void pack_outside_frame(int &idx, weather_data &wdat, loop_counter_outside &loopc_out);
  void pack_inside_frame(int &idx, weather_data &wdat, loop_counter_inside &loopc_in);
  void unpack_outside_frame(int &idx, weather_data &wdat, loop_counter_outside &loopc_out);
  void print_tx_frame_to_serial(void);
  void print_rx_frame_to_serial(void);
}; // class weather_radio

#endif
// eee eof
