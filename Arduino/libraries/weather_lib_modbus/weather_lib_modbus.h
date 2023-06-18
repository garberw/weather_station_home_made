#ifndef weather_lib_modbus_h
#define weather_lib_modbus_h
/*
  Modbus RTU Weather Library

  This library provides Modbus RTU communication;
  client = weather_dri_wmod
  server = weather_dri_sensor_light; weather_dri_sensor_rain; prefab temp; prefab wind;
  client requests     weather sensor data;
  server replies with weather sensor data;

  servers with sensors include;
  ws_XXXX;  additional;  sensor chip;      modbus server;
  wind;                  renke;            built in;
  temp;     humidity;    sht30;            built in;
  light;    UV;          ltr390; tsl2591;  arduino;
  rain;                  misol;            arduino;
*/

#include <weather_lib_modbus_macros.h>
#include <weather_lib_data.h>

class weather_sensor {
public:
  void calc_timeout              (void);
  void ModRTU_CRC                (uint16_t &cHL);
  int  compare_CRC               (int cHL);
  int  compare_data              (frame_idx f2);
  void setup                     (device_idx ws, byte len);
  void zero                      (void);
  void print                     (format_string title);
  void pack_CRC                  (uint16_t cHL);
  void pack_client_request_frame (void);
  void pack_server_reply_frame   (void);
  int  write                     (int &nwritten);
  void peek_byte                 (byte imodbus, byte &B);
  bool match_gap                 (byte &ngap);
  bool match_byte                (byte &imodbus, bool specific, byte BT, byte &B);
  int  match_modbus              (byte &igap, byte &imodbus, device_idx1 &ws, weather_ctx1 &wr);
  void print_ws                  (device_idx1 ws);
  void print_wr                  (weather_ctx1 wr);
  void print_ws_wr               (format_string name, device_idx1 ws, weather_ctx1 wr);
  int  read                      (int timeout_read_msec, weather_ctx wr, bool &match);
  void select_sensor             (weather_ctx wr, frame_idx fr);
  static int  static_setup_frame_array(void);
  //  static void select_frame   (frame_idx fr);
  //  void select_ctx            (weather_ctx wr);
public:
  // modbus device address = index in weather_modbus.m_weather_buf;
  device_idx  m_device_idx;      // a.k.a. ws
  weather_ctx m_weather_ctx;     // a.k.a. wr
  byte        m_data_length_words;
  // frame length in bytes;
  int         m_nread;
  byte        m_MAX;
  byte        m_MAX_CTX;
  byte        m_MAX_CRX;
  // give up when imodbus >= DATA_MAX;
  bool m_overflow_occur;
  // give up when m_time_read > m_timeout_read_usec;
  bool m_timeout_occur;
  // give up when m_time_read > m_timeout_read_usec;
  uint32_t m_time_read_usec;
  // timeout for blocking read microseconds;
  uint32_t m_timeout_read_usec;
  // timeout for read inter-character;
  uint32_t m_timeout_T15_usec;
  // timeout for frame after writing entire reply;
  uint32_t m_timeout_T35_usec;
  // pretend it has FRAME_MAX but skip extras;
  //  static SoftwareSerial RS485Serial;
  static frame_idx m_frame_idx;
}; // class weather_sensor;

// CTX = client transmit (request)
// CRX = client receive  (reply)
class weather_modbus {
public:
  weather_modbus            (void) { }
  // call this first
  void setup_frame_array    (void)                { weather_sensor::static_setup_frame_array(); }
  // call this second
  void setup_buffers        (void);
  void select_client_input  (device_idx ws);
  void select_client_output (device_idx ws);
  void select_server_input  (device_idx ws);
  void select_server_output (device_idx ws);
  void select_server_temp   (device_idx ws);
  int  client_tx_rx         (device_idx ws, int timeout_read_msec);
  int  server_rx_tx         (device_idx ws, int timeout_read_msec);
  int  crc_debug            (void);
  /*
  void select_wind_ctx      (void)                { select_ws_wr(ws_WIND, wr_CTX);  }
  void select_wind_crx      (void)                { select_ws_wr(ws_WIND, wr_CRX);  }
  void select_temp_ctx      (void)                { select_ws_wr(ws_TEMP, wr_CTX);  }
  void select_temp_crx      (void)                { select_ws_wr(ws_TEMP, wr_CRX);  }
  void select_light_ctx     (void)                { select_ws_wr(ws_LIGH, wr_CTX);  }
  void select_light_crx     (void)                { select_ws_wr(ws_LIGH, wr_CRX);  }
  void select_rain_ctx      (void)                { select_ws_wr(ws_RAIN, wr_CTX);  }
  void select_rain_crx      (void)                { select_ws_wr(ws_RAIN, wr_CRX);  }
  */
  void buf_zero             (void)                { m_pattern->zero(); }
  int  buf_compare_data     (frame_idx fr )       { return m_pattern->compare_data(fr); }
  void buf_ModRTU_CRC       (uint16_t &cHL)       { m_pattern->ModRTU_CRC(cHL); }
  void buf_pack_CRC         (uint16_t  cHL)       { m_pattern->pack_CRC(cHL); }
  int  buf_compare_CRC      (uint16_t  cHL)       { return m_pattern->compare_CRC(cHL); }
  int  buf_write            (int &nwritten)       { return m_pattern->write(nwritten); }
  int  buf_nread            (void         )       { return m_pattern->m_nread; }
  void buf_print            (format_string title) { m_pattern->print(title); }
  // fixme SELECT
  weather_ctx weather_ctx_wr(void         )       { return m_pattern->m_weather_ctx; }
  device_idx device_idx_ws  (void         )       { return m_pattern->m_device_idx;  }
  frame_idx frame_idx_fr    (void         )       { return m_pattern->m_frame_idx;   }
  int  buf_read             (int timeout_read_msec, weather_ctx wr, bool &match) {
    return m_pattern->read(timeout_read_msec, wr, match);
  }
  void pack_sensor  (device_idx ws, weather_data &wdat);
  void pack_tsys    (device_idx ws, weather_data &wdat);
  void unpack_sensor(device_idx ws, weather_data &wdat);
  void unpack_tsys  (device_idx ws, weather_data &wdat);
  /*
  void pack_wind    (weather_data &wdat) { pack_weather_data(ws_WIND, wdat); }
  void pack_temp    (weather_data &wdat) { pack_weather_data(ws_TEMP, wdat); }
  void pack_light   (weather_data &wdat) { pack_weather_data(ws_LIGH, wdat); }
  void pack_rain    (weather_data &wdat) { pack_weather_data(ws_RAIN, wdat); }
  void unpack_wind  (weather_data &wdat) { unpack_weather_data(ws_WIND, wdat); }
  void unpack_temp  (weather_data &wdat) { unpack_weather_data(ws_TEMP, wdat); }
  void unpack_light (weather_data &wdat) { unpack_weather_data(ws_LIGH, wdat); }
  void unpack_rain  (weather_data &wdat) { unpack_weather_data(ws_RAIN, wdat); }
  */
private:
  // fixme SELECT
  // static void select_frame(frame_idx fr) { weather_sensor::select_frame(fr); }
  // void select_ws_wr(device_idx ws, weather_ctx wr);
  int  crc_debug_sensor(format_string msg, device_idx ws, uint16_t cHL);
  void buf_pack_client_request_frame(void) { m_pattern->pack_client_request_frame(); }
  void buf_pack_server_reply_frame  (void) { m_pattern->pack_server_reply_frame  (); }
  // modbus device address = index in weather_modbus.m_weather_buf;
  // never use index 0
  device_idx m_device_idx;
  //  weather_ctx m_weather_ctx;
  weather_sensor *m_pattern;
}; // class weather_modbus;

void match_flush_input_buffer(byte &nflush);

#endif
// eee eof
