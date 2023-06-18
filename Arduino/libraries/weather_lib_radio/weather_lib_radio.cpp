/*
  Weather Radio

  This sketch uses LoRa radio to relay weather sensor data.
  The modbus RTU client reads from the modbus RTU servers (weather sensors).
  The modbus RTU client also acts as a LoRa server.
  The LoRa server outputs sensor values to the serial monitor and
  also relays or transmits (TX) to the arduino LoRa client (RX) inside the house.  

  This client arduino (RX) is hooked up by usb to a computer running weewx.
  weewx driver WeewxModbus
  in weewx.conf 'driver=user.weewx_wmod'
  to install extension weewx_wmod_master.tar.gz 
  tar czf weewx_wmod_master.tar.gz weewx_wmod_master/
  wee_extension --install weewx_wmod_master.tar.gz
  wee_config --reconfigure --driver=user.weewx_wmod --no-prompt

  Example sketch showing how to create a simple messaging client (transmitter)
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.
*/
/*
  // orignal values
  // currently 80 bytes + added 18 tsys words = 152 bytes total
  #define SIZE_PACK_OUTSIDE_FRAME ((20+18) * SIZE_PACK_WORD)
  // currently 56 bytes
  #define SIZE_PACK_INSIDE_FRAME (14 * SIZE_PACK_WORD)
*/

#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <weather_lib_util.h>
#include <weather_lib_array.h>
#include <weather_lib_radio.h>
#include <weather_lib_data.h>

/* macros begin ----------------------------------------------- */

// #define pin_RFM95_CS   4   // pin chip select
// #define pin_RFM95_RST  2   // pin reset
// #define pin_RFM95_INT  3   // pin interrupt
#define pin_RFM95_CS   5   // pin chip select
#define pin_RFM95_RST  7   // pin reset
#define pin_RFM95_INT  9   // pin interrupt
// #define pin_LED       13   // LED
  
// Change to 434.0 or other frequency;
// TX freq must match RX freq;
#define RF95_FREQ 444.0

// SIZE_PACK number of hex chars does not include trailing '\0';
// MAX       number of hex chars does     include trailing '\0';
// invariant:  m_cframe->at(idx) == '\0';  at function return;

// modbus word = uint16_t = 2 bytes = 4 hex chars;
#define SIZE_PACK_WORD 4

// float = 4 bytes = 8 hex chars;
#define SIZE_PACK_FLOAT 8

// currently 2 bytes
#define SIZE_PACK_HEADER 2

// currently 1 bytes 
#define SIZE_PACK_TERM 1

// currently 4 bytes
#define SIZE_PACK_DONE (1 * SIZE_PACK_WORD)

// SP = (# floats * SIZE_PACK_FLOAT)
#define PACK_RATIO 2
#define SIZE_PACK_OUT_WIND  (4 * SIZE_PACK_FLOAT)  // 4 floats; SIZE_PACK_OUT_WIND=32
#define SIZE_PACK_OUT_TEMP  (2 * SIZE_PACK_FLOAT)
#define SIZE_PACK_OUT_LIGH  (7 * SIZE_PACK_FLOAT)
#define SIZE_PACK_OUT_RAIN  (5 * SIZE_PACK_FLOAT)   // fixme was 2
#define SIZE_PACK_OUT_TSYSC (3 * SIZE_PACK_FLOAT)
#define SIZE_PACK_OUT_TSYSL (3 * SIZE_PACK_FLOAT)
#define SIZE_PACK_OUT_TSYSR (3 * SIZE_PACK_FLOAT)
#define SIZE_PACK_IN_RADIO  (8 * SIZE_PACK_FLOAT)

// currently 27 * 8 = 216 hex chars total
#define SIZE_PACK_OUTSIDE_FRAME (SIZE_PACK_OUT_WIND + SIZE_PACK_OUT_TEMP + SIZE_PACK_OUT_LIGH + SIZE_PACK_OUT_RAIN + SIZE_PACK_OUT_TSYSC + SIZE_PACK_OUT_TSYSL + SIZE_PACK_OUT_TSYSR)

// currently 8 * 8 = 64 hex chars
#define SIZE_PACK_INSIDE_FRAME (SIZE_PACK_IN_RADIO)

// currently 2 + 216 + 64 + 1 = 283 hex chars
#define SIZE_PACK_WEEWX_FRAME (SIZE_PACK_HEADER + SIZE_PACK_OUTSIDE_FRAME + SIZE_PACK_INSIDE_FRAME + SIZE_PACK_TERM)

// currently 2 + 4 + 216 + 1 = 223 hex chars
#define SIZE_PACK_RADIO_FRAME (SIZE_PACK_HEADER + SIZE_PACK_DONE + SIZE_PACK_OUTSIDE_FRAME + SIZE_PACK_TERM)

// fixme check if this includes trailing '\0'
// RH_RF95_MAX_MESSAGE_LEN = 251
#define RADIO_FRAME_MAX RH_RF95_MAX_MESSAGE_LEN

// SIZE_PACK_WEEWX_FRAME +1 for trailing '\0'
// just make it as small as possible
#define WEEWX_FRAME_MAX 284

#define GENERAL_FRAME_MAX 284

const char RADIO_FRAME_HEADER[] = "!!";
const char RADIO_FRAME_TERM[] =  "\n";

/* macros end ----------------------------------------------- */
/* typedefs begin ----------------------------------------------- */

extern weather_array<uint8_t> m_common_array;

class common_frame {
public:
  common_frame(format_string name_dest,
               format_string name_i_vs_o,
               format_string name_full,
               format_string name_size_pack,
               format_string name_max,
               int           size_pack,
               int           max) :
    m_name_dest      (name_dest),
    m_name_i_vs_o    (name_i_vs_o),
    m_name_full      (name_full),
    m_name_size_pack (name_size_pack),
    m_name_max       (name_max),
    m_size_pack      (size_pack),
    m_max            (max),
    m_length         (0),
    m_buf            (& ::m_common_array)
  { }
  format_string m_name_dest;
  format_string m_name_i_vs_o;
  format_string m_name_full;
  format_string m_name_size_pack;
  format_string m_name_max;
  int m_size_pack;
  int m_max;
  int m_length;
  weather_array<uint8_t> *m_buf;
  uint8_t &at(int idx) { return m_buf->at(idx); }
  uint8_t *get_data(void) { return m_buf->get_data(); }
}; // class common_frame;

/* typedefs end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

// singleton instance of the radio driver
RH_RF95 radio_rf(pin_RFM95_CS, pin_RFM95_INT);

// packet counter, we increment per transmission
int16_t m_packetnum = 0; 

// fixme save_memory
weather_array<uint8_t> m_common_array;

/* globals end ----------------------------------------------- */
/* functions begin ----------------------------------------------- */

common_frame frame_weewx_out(F("WEEWX"),F("OUTPUT"),F("WEEWX_OUTPUT_FRAME"),
                             F("SIZE_PACK_WEEWX_FRAME"),F("WEEWX_FRAME_MAX"),
                             SIZE_PACK_WEEWX_FRAME,     WEEWX_FRAME_MAX);
common_frame frame_radio_out(F("RADIO"),F("OUTPUT"),F("RADIO_OUTPUT_FRAME"),
                             F("SIZE_PACK_RADIO_FRAME"),F("RADIO_FRAME_MAX"),
                             SIZE_PACK_RADIO_FRAME,     RADIO_FRAME_MAX);
common_frame frame_radio_in (F("RADIO"),F("INPUT" ),F("RADIO_INPUT_FRAME"),
                             F("SIZE_PACK_RADIO_FRAME"),F("RADIO_FRAME_MAX"),
                             SIZE_PACK_RADIO_FRAME,     RADIO_FRAME_MAX);

// use default assignment operator
common_frame *m_cframe = &frame_weewx_out;

void select_frame_weewx_out(void) { m_cframe = &frame_weewx_out; }
void select_frame_radio_out(void) { m_cframe = &frame_radio_out; }
void select_frame_radio_in (void) { m_cframe = &frame_radio_in ; }

void print_frame_to_serial(format_string title);
void print_ascii_input_char(char c);

/* functions end ----------------------------------------------- */

char hex_to_ascii(byte d) {
  FUNC_BEGIN;
  static const char table[17] = "0123456789ABCDEF";
  ERROR_CONDITION(((0 <= d) && (d < 16)), F("bad hex digit"));
  return table[d];
 catch_block:
  return 'x';        // not a valid hex ascii representation;
}

byte ascii_to_hex(char c) {
  FUNC_BEGIN;
  switch (c) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': return 10;
  case 'b': return 11;
  case 'c': return 12;
  case 'd': return 13;
  case 'e': return 14;
  case 'f': return 15;
  case 'A': return 10;
  case 'B': return 11;
  case 'C': return 12;
  case 'D': return 13;
  case 'E': return 14;
  case 'F': return 15;
  };
  ERROR_CONDITION(false, F("bad hex digit"));
  // should never reach this
  return 0;
 catch_block:
  return -1;    // not a valid hex digit;
}

void weather_radio::pack_word(int &idx, bool done, uint16_t w) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  idx += SIZE_PACK_WORD;
  int j = idx;
  // leave space for trailing '\0'
  ERROR_CONDITION((idx < m_cframe->m_max), F("pack_word overflow m_cframe->m_max"));
  if (!done) {
    m_cframe->at(j--) = '\0';
    m_cframe->at(j--) = '-';
    m_cframe->at(j--) = '-';
    m_cframe->at(j--) = '-';
    m_cframe->at(j--) = '-';
    return;
  }
  // fixme check endian
  m_cframe->at(j--) = '\0';
  m_cframe->at(j--) = hex_to_ascii(w % 16);  // low order byte in high j
  w >>= 4;
  m_cframe->at(j--) = hex_to_ascii(w % 16);
  w >>= 4;
  m_cframe->at(j--) = hex_to_ascii(w % 16);
  w >>= 4;
  m_cframe->at(j--) = hex_to_ascii(w % 16);  // high order byte in low j
  w >>= 4;
  return;
 catch_block:
  wexception.print();
}

void weather_radio::unpack_word(int &idx, bool done, uint16_t &w) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  int j = idx;
  idx += SIZE_PACK_WORD;
  bool hex0;
  bool hex1;
  bool hex2;
  bool hex3;
  // default in case of error
  w = -1;
  // leave space for trailing '\0'
  ERROR_CONDITION((idx < m_cframe->m_max), F("pack_word overflow m_cframe->m_max"));
  if (!done) {
    return;
  }
  hex0 = isHexadecimalDigit(m_cframe->at(j + 0));
  hex1 = isHexadecimalDigit(m_cframe->at(j + 1));
  hex2 = isHexadecimalDigit(m_cframe->at(j + 2));
  hex3 = isHexadecimalDigit(m_cframe->at(j + 3));
  // warning not error;
  ERROR_CONDITION((hex0 && hex1 && hex2 && hex3), F("chars are not all digits"));
  w = 0;
  w |= ascii_to_hex(m_cframe->at(j++));  // high order byte in low j
  w <<= 4;
  w |= ascii_to_hex(m_cframe->at(j++));
  w <<= 4;
  w |= ascii_to_hex(m_cframe->at(j++));
  w <<= 4;
  w |= ascii_to_hex(m_cframe->at(j++));  // low order byte in high j
  return;
 catch_block:
  wexception.print();
}

// does not matter if little or big endian; only used in pairs with this unpack_float;
void weather_radio::pack_float(int &idx, bool done, float fff) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  uint16_t H1 = 0;
  uint16_t L1 = 0;
  uint32_t H  = 0;
  uint32_t L  = 0;
  uint32_t dw = 0;
  int j = idx + SIZE_PACK_FLOAT;
  ERROR_CONDITION((j < m_cframe->m_max), F("pack float overflow m_cframe->m_max"));
  memcpy(&dw, &fff, sizeof(float));
  L = dw & 0x0000FFFF;
  H = (dw >> 16);
  // must be in range etc.;
  // fixme check
  L1 = ((uint16_t) L);
  H1 = ((uint16_t) H);
  pack_word(idx, done, H1);
  pack_word(idx, done, L1);
  return;
 catch_block:
  wexception.print();
}

void weather_radio::unpack_float(int &idx, bool done, float &fff) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  uint16_t H1 = 0;
  uint16_t L1 = 0;
  uint32_t H  = 0;
  uint32_t L  = 0;
  uint32_t dw = 0;
  int j = idx + SIZE_PACK_FLOAT;
  // default in case of error
  fff = -1.0;
  ERROR_CONDITION((j < m_cframe->m_max), F("pack float overflow m_cframe->m_max"));
  unpack_word(idx, done, H1);
  unpack_word(idx, done, L1);
  // this definitely works;
  H = ((uint32_t) H1);
  L = ((uint32_t) L1);
  dw = ((H << 16) | L);
  memcpy(&fff, &dw, sizeof(float));
  return;
 catch_block:
  wexception.print();
}

void weather_radio::pack_header(int &idx) {
  FUNC_BEGIN;
  for (int j = 0; RADIO_FRAME_HEADER[j]; j++) {
    char c = RADIO_FRAME_HEADER[j];
    m_cframe->at(idx++) = c;
  }
  m_cframe->at(idx) = '\0';
}

void weather_radio::pack_term(int &idx) {
  FUNC_BEGIN;
  for (int j = 0; RADIO_FRAME_TERM[j]; j++) {
    char c = RADIO_FRAME_TERM[j];
    m_cframe->at(idx++) = c;
  }
  m_cframe->at(idx) = '\0';
}

void weather_radio::unpack_header(int &idx) {
  FUNC_BEGIN;
  for (int j = 0; RADIO_FRAME_HEADER[j]; j++) {
    char c = RADIO_FRAME_HEADER[j];
    char d = m_cframe->at(idx++);
    ERROR_CONDITION((c == d), F("header does not match"));
  }
  return;
 catch_block:
  ;
}

void weather_radio::unpack_term(int &idx) {
  FUNC_BEGIN;
  for (int j = 0; RADIO_FRAME_TERM[j]; j++) {
    char c = RADIO_FRAME_TERM[j];
    char d = m_cframe->at(idx++);
    ERROR_CONDITION((c == d), F("term does not match"));
  }
  return;
 catch_block:
  ;
}

void weather_radio::pack_outside_done(int &idx, loop_counter_outside &loopc_out) {
  FUNC_BEGIN;
  uint16_t w = 0;
  w |= ((loopc_out.m_done[ws_TSYS] ? 1 : 0) << ws_TSYS);
  w |= ((loopc_out.m_done[ws_WIND] ? 1 : 0) << ws_WIND);
  w |= ((loopc_out.m_done[ws_TEMP] ? 1 : 0) << ws_TEMP);
  w |= ((loopc_out.m_done[ws_LIGH] ? 1 : 0) << ws_LIGHT);
  w |= ((loopc_out.m_done[ws_RAIN] ? 1 : 0) << ws_RAIN);
  pack_word(idx, true, w);
}

void weather_radio::unpack_outside_done(int &idx, loop_counter_outside &loopc_out) {
  FUNC_BEGIN;
  uint16_t w = 0;
  unpack_word(idx, true, w);
  loopc_out.m_done[ws_TSYS] = ((w >> ws_TSYS ) & 1);
  loopc_out.m_done[ws_WIND] = ((w >> ws_WIND ) & 1);
  loopc_out.m_done[ws_TEMP] = ((w >> ws_TEMP ) & 1);
  loopc_out.m_done[ws_LIGH] = ((w >> ws_LIGHT) & 1);
  loopc_out.m_done[ws_RAIN] = ((w >> ws_RAIN ) & 1);
}

// WIS = weather data entry index set;
void weather_radio::pack_WIS(int &idx, weather_data &wdat, bool done, WIS &idx_set) {
  FUNC_BEGIN;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int iii = idx_set.m_idx[jjj];
    float val;
    wdat.get(iii, val);
    pack_float(idx, done, val);
  }
  return;
 catch_block:
  wexception.print();
}

// WIS = weather data entry index set;
void weather_radio::unpack_WIS(int &idx, weather_data &wdat, bool done, WIS &idx_set) {
  FUNC_BEGIN;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int iii = idx_set.m_idx[jjj];
    float val;
    unpack_float(idx, done, val);
    bool convert = false;
    wdat.set(iii, val, convert);
  }
  return;
 catch_block:
  wexception.print();
}

// changes m_cframe->m_length;
// does not change wdat;
// require done defined;
void weather_radio::pack_outside_frame(int &idx,
                                       weather_data &wdat,
                                       loop_counter_outside &loopc_out)
{
  FUNC_BEGIN;
  //  e.clear_all();
  bool done_wind = loopc_out.m_done[ws_WIND];
  bool done_temp = loopc_out.m_done[ws_TEMP];
  bool done_ligh = loopc_out.m_done[ws_LIGH];
  bool done_rain = loopc_out.m_done[ws_RAIN];
  bool done_tsys = loopc_out.m_done[ws_TSYS];
  pack_WIS(idx, wdat, done_wind, WIS_outside_wind0);
  pack_WIS(idx, wdat, done_temp, WIS_outside_temp0);
  pack_WIS(idx, wdat, done_ligh, WIS_outside_light0);
  pack_WIS(idx, wdat, done_rain, WIS_outside_rain0);
  pack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_client);
  pack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_light);
  pack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_rain);
}

// same format used for radio tx/rx and weewx_wmod driver rx
void weather_radio::pack_inside_frame(int &idx,
                                      weather_data &wdat,
                                      loop_counter_inside &loopc_in)
{
  FUNC_BEGIN;
  // e.clear_all();
  bool done_temp = loopc_in.m_done[ws_TEMP];
  pack_WIS(idx, wdat, done_temp, WIS_inside_radio);
}

// does not change m_common_array;
// does not change m_cframe->m_length;
// does     change wdat;
// does not change weather_radio class members;
// require done defined e.g. call unpack_outside_done() first;
void weather_radio::unpack_outside_frame(int &idx,
                                         weather_data &wdat,
                                         loop_counter_outside &loopc_out)
{
  FUNC_BEGIN;
  // e.clear_all();
  bool done_wind = loopc_out.m_done[ws_WIND];
  bool done_temp = loopc_out.m_done[ws_TEMP];
  bool done_ligh = loopc_out.m_done[ws_LIGH];
  bool done_rain = loopc_out.m_done[ws_RAIN];
  // fixme check only one of these
  bool done_tsys = loopc_out.m_done[ws_TSYS];
  unpack_WIS(idx, wdat, done_wind, WIS_outside_wind0);
  unpack_WIS(idx, wdat, done_temp, WIS_outside_temp0);
  unpack_WIS(idx, wdat, done_ligh, WIS_outside_light0);
  unpack_WIS(idx, wdat, done_rain, WIS_outside_rain0);
  unpack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_client);
  unpack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_light);
  unpack_WIS(idx, wdat, done_tsys, WIS_outside_tsys_rain);
}

// idx is address of trailing '\0'; this should exactly fill m_cframe->m_size_pack;
int weather_radio::check_term(int idx) {
  char c = m_cframe->at(idx);
  ERROR_CONDITION((c == '\0'), F("term trailing '\\0' does not match"));
  ERROR_CONDITION((m_cframe->m_length == idx), F("m_cframe->m_length == idx"));
  ERROR_CONDITION((idx == m_cframe->m_size_pack), F("idx == m_cframe->m_size_pack"));
  return 0;
 catch_block:
  ser_print(F("current frame         = ")); ser_println(m_cframe->m_name_full);
  ser_print(F("idx                   = ")); ser_println(idx);
  ser_print(m_cframe->m_name_size_pack); ser_print(F(" = ")); ser_println(m_cframe->m_size_pack);
  wexception.print();
  return 1;
}

// used for weather_radio_rx for transmitting by ttyUSB to weewx_wmod driver;
// weather_radio_rx transmits wdat from weather_client to weewx;
// the equivalent of unpack_weewx_frame is the python weewx_wmod driver;
void weather_radio::pack_weewx_frame(weather_data &wdat,
                                     loop_counter_outside &loopc_out,
                                     loop_counter_inside &loopc_in)
{
  FUNC_BEGIN;
  bool debug = false;
  select_frame_weewx_out();
  int idx;
  idx = 0;
  pack_header         (idx);
  pack_outside_frame  (idx, wdat, loopc_out);
  pack_inside_frame   (idx, wdat, loopc_in);
  pack_term           (idx);
  m_cframe->m_length = idx;
  if (debug) {
    print_frame_to_serial(m_cframe->m_name_full);
  }
  ERROR_CONDITION(!check_term(idx), F("improperly terminated"));
  return;
 catch_block:
  ;
}

// used for weather_client (modbus client and radio server)
// for transmitting by radio to weather_radio_rx;
void weather_radio::pack_radio_frame(weather_data &wdat, loop_counter_outside &loopc_out) {
  FUNC_BEGIN;
  bool debug = false;
  select_frame_radio_out();
  int idx;
  idx = 0;
  pack_header       (idx);
  pack_outside_done (idx, loopc_out);
  pack_outside_frame(idx, wdat, loopc_out);
  pack_term         (idx);
  m_cframe->m_length = idx;
  if (debug) {
    print_frame_to_serial(m_cframe->m_name_full);
  }
  ERROR_CONDITION(!check_term(idx), F("improperly terminated"));
  return;
 catch_block:
  ;
}

// the equivalent of unpack_weewx_frame is the python weewx_wmod driver;

// used for weather_radio_rx (radio client)
// for receiving by radio from weather_client (modbus client and radio server);
void weather_radio::unpack_radio_frame(weather_data &wdat, loop_counter_outside &loopc_out) {
  FUNC_BEGIN;
  bool debug = false;
  int idx;
  select_frame_radio_in();
  idx = 0;
  unpack_header       (idx);
  unpack_outside_done (idx, loopc_out);
  unpack_outside_frame(idx, wdat, loopc_out);
  unpack_term         (idx);

  // fixme
  // after radio input m_cframe->m_length (which is the number of chars read)
  // may be different from perfect value m_cframe->m_size_pack;
  // this always works
  if (debug) {
    print_frame_to_serial(m_cframe->m_name_full);
  }
  // input is not terminated with '\0' by radiohead;
  // see comment in receive_then_ack;
  //        leave space for trailing 0
  //        radio_frame_i_len = RADIO_FRAME_MAX - 1;
  // there should always be one more byte for trailing '\0' due to this "- 1";
  m_cframe->at(m_cframe->m_length) = '\0';
  // this rejects input with wrong length
  ERROR_CONDITION(!check_term(idx), F("improperly terminated"));
  return;
 catch_block:
  ;
}

// for Pete's sake how can I screw this up ????;
int weather_radio::check_wacky_macros(void) {
  ser_println(F("CHECK WACKY MACROS ==========================="));
  ser_println(F("expected values versus actual values"));
  int sw  = WIS_outside_wind0.m_MAX;
  int st  = WIS_outside_temp0.m_MAX;
  int sl  = WIS_outside_light0.m_MAX;
  int sr  = WIS_outside_rain0.m_MAX;
  int stc = WIS_outside_tsys_client.m_MAX;
  int stl = WIS_outside_tsys_light.m_MAX;
  int str = WIS_outside_tsys_rain.m_MAX;
  int so  = sw + st + sl + sr + stc + stl + str;
  int si  = WIS_inside_radio.m_MAX;
  ser_print(F("SIZE_PACK_OUT_WIND      = 8 * 4 = "));
  ser_print(SIZE_PACK_OUT_WIND); ser_print(F(" = 8 * ")); ser_println(sw);
  ser_print(F("SIZE_PACK_OUT_TEMP      = 8 * 2 = "));
  ser_print(SIZE_PACK_OUT_TEMP); ser_print(F(" = 8 * ")); ser_println(st);
  ser_print(F("SIZE_PACK_OUT_LIGH      = 8 * 7 = "));
  ser_print(SIZE_PACK_OUT_LIGH); ser_print(F(" = 8 * ")); ser_println(sl);
  ser_print(F("SIZE_PACK_OUT_RAIN      = 8 * 2 = "));
  ser_print(SIZE_PACK_OUT_RAIN); ser_print(F(" = 8 * ")); ser_println(sr);
  ser_print(F("SIZE_PACK_OUT_TSYSC     = 8 * 3 = "));
  ser_print(SIZE_PACK_OUT_TSYSC); ser_print(F(" = 8 * ")); ser_println(stc);
  ser_print(F("SIZE_PACK_OUT_TSYSL     = 8 * 3 = "));
  ser_print(SIZE_PACK_OUT_TSYSL); ser_print(F(" = 8 * ")); ser_println(stl);
  ser_print(F("SIZE_PACK_OUT_TSYSR     = 8 * 3 = "));
  ser_print(SIZE_PACK_OUT_TSYSR); ser_print(F(" = 8 * ")); ser_println(str);
  ser_print(F("SIZE_PACK_IN_RADIO      = 8 * 8 = "));
  ser_print(SIZE_PACK_IN_RADIO); ser_print(F(" = 8 * ")); ser_println(si);
  ser_print(F("SIZE_PACK_OUTSIDE_FRAME = 8 * 24   = 192 = "));
  ser_println(SIZE_PACK_OUTSIDE_FRAME); ser_print(F(" = 8 * ")); ser_println(so);
  ser_print(F("SIZE_PACK_INSIDE_FRAME  = 8 *  8   =  64 = "));
  ser_println(SIZE_PACK_INSIDE_FRAME);
  ser_print(F("SIZE_PACK_WEEWX_FRAME   = 3+192+64 = 259 = "));
  ser_println(SIZE_PACK_WEEWX_FRAME);
  ser_print(F("SIZE_PACK_RADIO_FRAME   = 7+192    = 199 = "));
  ser_println(SIZE_PACK_RADIO_FRAME);
  ser_print(F("RADIO_FRAME_MAX   (from RADIOHEAD) = 251 = ")); ser_println(RADIO_FRAME_MAX);
  ser_print(F("WEEWX_FRAME_MAX   (anyth that fits)= 260 = ")); ser_println(WEEWX_FRAME_MAX);
  ser_print(F("GENERAL_FRAME_MAX (anyth that fits)= 260 = ")); ser_println(GENERAL_FRAME_MAX);
  ERROR_CONDITION(RADIO_FRAME_MAX <= GENERAL_FRAME_MAX, F("GENERAL_FRAME_MAX >= RADIO_FRAME_MAX"));
  ERROR_CONDITION(WEEWX_FRAME_MAX <= GENERAL_FRAME_MAX, F("GENERAL_FRAME_MAX >= WEEWX_FRAME_MAX"));
  // + 1 for trailing '\0'
  ser_println(F("require (A) <= (B)"));
  ser_print(F("(A) SIZE_PACK_RADIO_FRAME + 1      = 200 = "));
  ser_println(SIZE_PACK_RADIO_FRAME + 1);
  ser_print(F("(B) RADIO_FRAME_MAX                = 251 = ")); ser_println(RADIO_FRAME_MAX);
  ERROR_CONDITION((SIZE_PACK_RADIO_FRAME + 1 <= RADIO_FRAME_MAX), F("radio frame overflow"));
  ser_print(F("SIZE_PACK_RADIO_FRAME = ")); ser_print(SIZE_PACK_RADIO_FRAME);
  ser_println(F(" <= 255"));
  ERROR_CONDITION((SIZE_PACK_RADIO_FRAME <= 255), F("idx <= SIZE_PACK_RADIO_FRAME must fit in byte"));
  ERROR_CONDITION((SIZE_PACK_WEEWX_FRAME + 1 <= WEEWX_FRAME_MAX), F("weewx frame overflow"));
  ser_println(F("CHECK WACKY MACROS pass ... ==========================="));
  return 0;
 catch_block:
  return 1;
}

// require Serial aready set up
void weather_radio::setup(void) {
  FUNC_BEGIN;
  bool res_init;
  bool res_freq;
  bool debug_macros = true;
  ERROR_CONDITION(!(debug_macros && check_wacky_macros()), F("check wacky macros"));
  m_common_array.setup(F("radio m_common_array"), GENERAL_FRAME_MAX);
  select_frame_radio_in();
  // select_frame_radio_out();
  // usually only RX blinks led
  // if (m_blink_led) {
  //    pinMode(pin_LED, OUTPUT);
  //  }
  pinMode(pin_RFM95_RST, OUTPUT);
  digitalWrite(pin_RFM95_RST, HIGH);
  pause_on_start();
  delay(100);
  ser_println(F("LoRa TX Test"));
  // manual reset
  digitalWrite(pin_RFM95_RST, LOW);
  delay(10);
  digitalWrite(pin_RFM95_RST, HIGH);
  delay(10);
  res_init = radio_rf.init();
  ERROR_CONDITION((res_init), F("LoRa radio init failed"));
  ser_println(F("LoRa radio init OKAY"));
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  res_freq = radio_rf.setFrequency(RF95_FREQ);
  ERROR_CONDITION((res_freq), F("setFrequency failed"));
  ser_print(F("Set Freq to: ")); ser_println(RF95_FREQ);
  // Defaults after init are
  // 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which use the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  radio_rf.setTxPower(23, false);
  return;
 catch_block:
  ser_println(F("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info"));
  wexception.print();
}

void print_ascii_input_char(char c) {
  switch (c) {
  case '\n':  ser_print(F("N")); break;
  case '\r':  ser_print(F("R")); break;
  case '\0':  ser_print(F("Z")); break;
  default:
    ser_print(c);
    break;
  }
}

void print_frame_to_serial(format_string title) {
  FUNC_BEGIN;
  ser_println(title);
  ser_println(F("got:  frame = "));
  int nsix = (m_cframe->m_length + 15) / 16;
  for (int six = 0; six < nsix; six++) {
    snprintf(snprintf_buf, SNPRINTF_BUF_MAX, "%5d > ", six * 16);
    ser_print(snprintf_buf);
    for (int j = 0; j < 16; j++) {
      int i = six * 16 + j;
      if (i >= m_cframe->m_length) break;
      char c = ((char) m_cframe->at(i));
      print_ascii_input_char(c);
      ser_print(F(" "));
    }
    ser_println();
  }
  return;
 catch_block:
  ;
}

void weather_radio::print_tx_frame_to_serial(void) {
  print_frame_to_serial(F("TX FRAME"));
}

void weather_radio::print_rx_frame_to_serial(void) {
  print_frame_to_serial(F("RX FRAME"));
}

// over ttyUSB
void weather_radio::transmit_weewx_frame(void) {
  FUNC_BEGIN;
  select_frame_weewx_out();
  for (int idx = 0; idx < m_cframe->m_length; idx++) {
    char c = (char) m_cframe->at(idx);
    Serial.print(c);
  }
  return;
 catch_block:
  ;
}

// require TX frame already packed in m_cframe and select_frame_radio_out();
// require m_cframe->m_length already set;
int weather_radio::transmit_then_ack(void) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  bool debug = false;
  uint8_t radio_frame_i_len;
  uint8_t radio_frame_o_len;
  uint8_t frame_ack[4];
  int res_strcmp;
  bool res_wait;
  bool res_recv;
  select_frame_radio_out();
  // e.clear_all();
  // wait 1 second between transmits; could also sleep() here;
  delay(1000);

  ser_println(F("pre-packed tx frame ..."));
  if (debug) {
    print_tx_frame_to_serial();
  }
  
  // send length is uint8_t not int
  ERROR_CONDITION(m_cframe->m_length <= 255, F("m_cframe->m_length **byte** overflow"));
  radio_frame_o_len = ((uint8_t) m_cframe->m_length);
  
  // transmit sensors to rf95_server;
  ser_println(F("transmit ..."));
  delay(10);
  radio_rf.send((uint8_t *)(m_cframe->get_data()), radio_frame_o_len);
  ser_println(F("transmit; waiting to complete ..."));
  delay(10);
  radio_rf.waitPacketSent();

  // receive ACK; wait;
  ser_println(F("receive ACK; wait ..."));
  res_wait = radio_rf.waitAvailableTimeout(1000);
  // NOT ERROR_CONDITION
  ERROR_RETURN((res_wait), F("receive ACK timeout; check the RX is listening"));

  // receive ACK; should be a reply message for us now;
  // resets radio_frame_i;
  radio_frame_i_len = 4;
  res_recv = radio_rf.recv(frame_ack, &radio_frame_i_len);
  
  // NOT ERROR_CONDITION
  ERROR_RETURN((res_recv), F("receive ACK failed"));

  // print ACK;
  ser_print(F("got reply: "));
  ser_println(((char *) frame_ack));

  ser_print(F("RSSI: "));
  ser_println(radio_rf.lastRssi(), DEC);
  
  // check it is an ACK;
  res_strcmp = strcmp(((char *) frame_ack), "ACK");
  // NOT ERROR_CONDITION
  ERROR_RETURN((!res_strcmp), F("message other than ACK"));
  return 0;
 catch_block:
  return -1;
}

// input result in m_cframe after successful return of receive_then_ack();
// outside program should unpack m_cframe;
int weather_radio::receive_then_ack(void) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  bool debug = false;
  uint8_t radio_frame_i_len;
  uint8_t radio_frame_o_len;
  uint8_t frame_ack[4];
  bool res_recv;
  select_frame_radio_in();
  // e.clear_all();
  // not an error; not a receive;
  bool res_avail = radio_rf.available();
  // NOT ERROR_CONDITION
  ERROR_RETURN((res_avail), F("res_avail"));

  // receive sensors;
  // should be a message for us now;
  // leave space for trailing 0
  radio_frame_i_len = RADIO_FRAME_MAX - 1;
  res_recv = radio_rf.recv(m_cframe->get_data(), &radio_frame_i_len);
  m_cframe->m_length = radio_frame_i_len;
  // NOT ERROR_CONDITION
  ERROR_RETURN((res_recv), F("receive failed"));

  if (false) {
    int i = radio_frame_i_len;
    if (i + 1 >= 0) {
      ser_print("c[len+1] = "); ser_println(m_cframe->at(i+1));
    }
    if (i >= 0) {
      ser_print("c[len-0] = "); ser_println(m_cframe->at(i--));
    }
    if (i >= 0) {
      ser_print("c[len-1] = "); ser_println(m_cframe->at(i--));
    }
    if (i >= 0) {
      ser_print("c[len-2] = "); ser_println(m_cframe->at(i--));
    }
    if (i >= 0) {
      ser_print("c[len-3] = "); ser_println(m_cframe->at(i--));
    }
  }
  
  // print sensors;
  if (debug) {
    RH_RF95::printBuffer("received: ", m_cframe->get_data(), radio_frame_i_len);
    print_rx_frame_to_serial();
  }
  ser_print(F("RSSI: "));
  ser_println(radio_rf.lastRssi(), DEC);
  
  // transmit ACK;
  // if (m_blink_led) {
  //   digitalWrite(pin_LED, HIGH);
  // }

  strcpy(((char *) frame_ack), "ACK");
  radio_frame_o_len = 4;
  radio_rf.send(frame_ack, radio_frame_o_len);
  radio_rf.waitPacketSent();
  ser_println(F("Sent a reply"));

  // if (m_blink_led) {
  //   digitalWrite(LED, LOW);
  // }
  return 0;
 catch_block:
  return -1;
}

// eee eof
