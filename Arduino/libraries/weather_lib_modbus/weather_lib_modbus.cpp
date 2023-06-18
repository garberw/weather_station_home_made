#include <weather_lib_util.h>
#include <weather_lib_array.h>
#include <weather_lib_modbus.h>
#include <weather_lib_data.h>

/* typedefs begin ----------------------------------------------- */

typedef weather_array<byte> *BBUF;

/* typedefs end ----------------------------------------------- */
/* functions begin ----------------------------------------------- */

void pack_byte(weather_array<uint8_t> &buf, int addr, byte b);
void unpack_byte (weather_array<uint8_t> &buf, int addr, byte &b);
void pack_word(weather_array<uint8_t> *buf, int addr, uint16_t w);
void unpack_word(weather_array<uint8_t> *buf, int addr, uint16_t &w);
void pack_register(weather_array<byte> *buf, int addr, uint16_t w);
void unpack_register(weather_array<byte> *buf, int addr, uint16_t &w);
void pack_register_float(weather_array<byte> *buf, int addr, float fff);
void unpack_register_float(weather_array<byte> *buf, int addr, float &fff);

BBUF modbus_registers (void);
void pack_weather_data(BBUF buf, weather_data &wdat, WIS &idx_set);
void unpack_weather_data(BBUF buf, weather_data &wdat, WIS &idx_set, bool convert);
void unpack_weather_data_uint16_t(BBUF buf, weather_data &wdat, WIS &idx_set, bool convert);

/* functions end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

// SoftwareSerial weather_sensor::RS485Serial(pin_RX, pin_TX);

frame_idx weather_sensor::m_frame_idx;

weather_array<weather_sensor> m_weather_buf(F("weather_modbus.m_weather_buf"),ws_MAX);
weather_array<byte> m_frame_array[frame_idx_MAX];
weather_array<byte> *m_frame = NULL;

/* globals end ----------------------------------------------- */
/* array pack and unpack begin ---------------------------------------------------------------*/
// these are for modbus
// other stuff should use little endian packing
// fixme radio uses big endian but it is transparent (self consistent);

void pack_byte(weather_array<uint8_t> &buf, int addr, byte b) {
  buf.at(addr) = b;
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

void unpack_byte (weather_array<uint8_t> &buf, int addr, byte &b) {
  b = buf.at(addr);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// big endian for modbus
void pack_word(weather_array<uint8_t> *buf, int addr, uint16_t w) {
  buf->at(addr + 0) = highByte(w);
  buf->at(addr + 1) = lowByte(w);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// big endian for modbus
void unpack_word(weather_array<uint8_t> *buf, int addr, uint16_t &w) {
  uint8_t H = buf->at(addr + 0);
  uint8_t L = buf->at(addr + 1);
  w = word(H, L);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// fixme big endian floats do not matter
void pack_double_word(weather_array<uint8_t> *buf, int addr, uint32_t dw) {
  bool debug = false;
  if (debug) {
    ser_print("pack_double_word dw = "); ser_println(dw, HEX);
  }
  buf->at(addr + 3) = dw & 0x000000FF;    // low order byte
  dw >>= 8;
  buf->at(addr + 2) = dw & 0x000000FF;
  dw >>= 8;
  buf->at(addr + 1) = dw & 0x000000FF;
  dw >>= 8;
  buf->at(addr + 0) = dw & 0x000000FF;    // high order byte
  if (debug) {
    ser_print("pack_double_word a0 = "); ser_println(buf->at(addr+0), HEX);
    ser_print("pack_double_word a1 = "); ser_println(buf->at(addr+1), HEX);
    ser_print("pack_double_word a2 = "); ser_println(buf->at(addr+2), HEX);
    ser_print("pack_double_word a3 = "); ser_println(buf->at(addr+3), HEX);
  }  
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// fixme big endian floats do not matter
void unpack_double_word(weather_array<uint8_t> *buf, int addr, uint32_t &dw) {
  bool debug = false;
  if (debug) {
    ser_print("unpack_double_word a0 = "); ser_println(buf->at(addr+0), HEX);
    ser_print("unpack_double_word a1 = "); ser_println(buf->at(addr+1), HEX);
    ser_print("unpack_double_word a2 = "); ser_println(buf->at(addr+2), HEX);
    ser_print("unpack_double_word a3 = "); ser_println(buf->at(addr+3), HEX);
  }  
  dw = 0;
  dw |= buf->at(addr + 0);    // high order byte
  dw <<= 8;
  dw |= buf->at(addr + 1);
  dw <<= 8;
  dw |= buf->at(addr + 2);
  dw <<= 8;
  dw |= buf->at(addr + 3);    // low order byte
  if (debug) {
    ser_print("unpack_double_word dw = "); ser_println(dw, HEX);
  }
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// big endian for modbus
void pack_register(weather_array<byte> *buf, int addr, uint16_t w) {
  pack_word(buf, PACK_OUTPUT_OFFSET + addr * 2, w);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// big endian for modbus
void unpack_register(weather_array<byte> *buf, int addr, uint16_t &w) {
  unpack_word(buf, PACK_OUTPUT_OFFSET + addr * 2, w);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// memcpy(dest, src, num_bytes);
void pack_register_float(weather_array<byte> *buf, int addr, float fff) {
  uint32_t dw;
  bool debug = false;
  if (debug) {
    ser_print(F("pack_register_float fff = ")); ser_println(fff, 12);
  }
  memcpy(&dw, &fff, sizeof(uint32_t));
  if (debug) {
    ser_print(F("pack_register_float dw  = ")); ser_println(dw, HEX);
  }
  pack_double_word(buf, PACK_OUTPUT_OFFSET + addr * 4, dw);
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

// memcpy(dest, src, num_bytes);
void unpack_register_float(weather_array<byte> *buf, int addr, float &fff) {
  uint32_t dw;
  bool debug = false;
  if (debug) {
    ser_println(F("unpack_register_float"));
  }
  unpack_double_word(buf, PACK_OUTPUT_OFFSET + addr * 4, dw);
  memcpy(&fff, &dw, sizeof(uint32_t));
  if (debug) {
    ser_print(F("unpack_register_float dw  = ")); ser_println(dw, HEX);
    ser_print(F("unpack_register_float fff = ")); ser_println(fff, 12);
  }
  ERROR_CATCH;
  return;
 catch_block:
  wexception.print();
}

/* array pack and unpack end ---------------------------------------------------------------*/

int weather_sensor::static_setup_frame_array(void) {
  FUNC_BEGIN;
  m_frame_array[frame_idx_I].setup(F("m_frame_array_I"), DATA_MAX);
  m_frame_array[frame_idx_O].setup(F("m_frame_array_O"), DATA_MAX);
  m_frame_array[frame_idx_T].setup(F("m_frame_array_T"), DATA_MAX);
  return 0;
}

// Modbus states that a baud rate higher than 19200 must use a fixed 750 us
// for inter character time out and 1.75 ms for a frame delay.
// For baud rates below 19200 the timing is more critical and has to be calculated.
// e.g. 9600 baud in a 10 bit packet is 960 characters per second
// In milliseconds this will be 960 characters per 1000ms. So for 1 character
// 1000 ms / 960 characters is 1.04167ms per character and finaly modbus states an
// intercharacter must be 1.5T or 1.5 times longer than a normal character and thus
// 1.5T = 1.04167ms * 1.5 = 1.5625ms. A frame delay is 3.5T.
// Added experimental low latency delays. This makes the implementation
// non-standard but practically it works with all major modbus master implementations.
void weather_sensor::calc_timeout(void) {
  FUNC_BEGIN;
  int32_t baud = RS485Serial_BAUD;
  int bits_per_packet = m_MAX;
  bool low_latency = false;
  if (baud == 1000000 && low_latency) {
    m_timeout_T15_usec = 1;
    m_timeout_T35_usec = 10;
  } else if (baud >= 115200 && low_latency){
    m_timeout_T15_usec = 75;
    m_timeout_T35_usec = 175;
  } else if (baud > 19200) {
    m_timeout_T15_usec = 750;
    m_timeout_T35_usec = 1750;
  } else {
    /*
      m_timeout_T15_usec = 15000000/baud; // 1T * 1.5 = T1.5
      m_timeout_T35_usec = 35000000/baud; // 1T * 3.5 = T3.5
    */
    // fixme these are minimums; try slightly larger;
    static const float nudge = 1.0;
    float char_per_sec = ((float) baud) / ((float) bits_per_packet);
    float T_usec = 1.0e6 / char_per_sec;
    m_timeout_T15_usec = ((uint32_t) (1.5 * nudge * T_usec));
    m_timeout_T35_usec = ((uint32_t) (3.5 * nudge * T_usec));
  }
  ser_print(F("baud               = ")); ser_println(baud);
  ser_print(F("m_timeout_T15_usec = ")); ser_println(m_timeout_T15_usec);
  ser_print(F("m_timeout_T35_usec = ")); ser_println(m_timeout_T35_usec);
}

void weather_sensor::pack_CRC(uint16_t cHL) { pack_word(m_frame, m_MAX - 2, cHL); }

// require select_sensor() called;
// Compute the MODBUS RTU CRC of
// input(0) ... input(m_MAX-3)    (omit last two CRC bytes)
void weather_sensor::ModRTU_CRC(uint16_t &cHL) {
  FUNC_BEGIN;
  // some little aliases ..........
  BBUF input = m_frame;
  const int   len   = m_MAX - 2;
  // ..............................
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)(input->at(pos));      // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else {                        // Else LSB is not set
        crc >>= 1;                    // Just shift right
      }
    }
  }
  // the above calculation has the bytes swapped;
  // m_cHL equals the crc in the standard order;
  cHL =  word(lowByte(crc), highByte(crc));
}

// require select_sensor() called;
// compare mhl to chl;
// chl = crc computed from input in m_frame read from serial port (computed);
// mhl = crc input in m_frame read from serial port (measured)
int weather_sensor::compare_CRC(int cHL) {
  FUNC_BEGIN;
  uint8_t H = m_frame->at(m_MAX - 2);
  uint8_t L = m_frame->at(m_MAX - 1);
  uint16_t mHL = word(H, L);
  ERROR_CONDITION((cHL == mHL), F("crc error"));
  return 0;
 catch_block:
  ser_println(F("crc error"));
  ser_print(F("cHL = ")); print_hex_16(cHL); ser_println();
  ser_print(F("mHL = ")); print_hex_16(mHL); ser_println();
  print(F("compare_CRC"));
  return -1;
}

// client transmit CTX
// (address)(function code)(starting address  )(data length L words        )(checksum)
//  0x01     0x03           0x00 0x00           0x00 0x02                    0xC4 0x0B
// server transmit CRX
// (address)(function code)(length bytes N=L*2)(data word1)...(data word(L))(checksum)
//  0x01     0x03           0x04                0x00 0x79      0x00 0x7A     0xAA 0x09
void weather_sensor::setup(device_idx ws, byte len) {
  FUNC_BEGIN;
  m_device_idx = ws;
  m_data_length_words = len;
  m_MAX_CTX = 8;
  m_MAX_CRX = 3 + m_data_length_words * 2 + 2;
  ERROR_CONDITION(((m_MAX_CTX <= DATA_MAX) && (m_MAX_CRX <= DATA_MAX)),
               F("MAX > DATA_MAX; buffer overflow"));
  // fixme init whether this is client or server
  select_sensor(wr_CRX, frame_idx_I);
  calc_timeout();
  return;
 catch_block:
  ;
}

void weather_sensor::select_sensor(weather_ctx wr, frame_idx fr) {
  FUNC_BEGIN;
  m_weather_ctx = wr;
  m_MAX = (wr == wr_CTX) ? m_MAX_CTX : m_MAX_CRX;
  // manual bounds check on frame_idx
  BOUNDS(fr, 0, frame_idx_MAX, F("frame_idx"));
  m_frame_idx = fr;
  m_frame = &m_frame_array[fr];
  return;
 catch_block:
  ser_print(F("error:  frame index ")); ser_println(fr);
}


void weather_sensor::zero(void) {
  FUNC_BEGIN;
  for (int i = 0; i < m_MAX; i++) {
    m_frame->at(i) = 0;
  }
}

void weather_sensor::print(format_string title) {
  FUNC_BEGIN;
  ser_println(title);
  ser_print(F("index = "));
  for(int i = 0; i < m_MAX; i++) {
    print_dec_8(i);
    ser_print(F(" "));
  }
  ser_println();
  ser_print(F("buf   ="));
  for(int i = 0; i < m_MAX; i++) {
    ser_print(F(" ")); print_hex_8(m_frame->at(i));
  }
  ser_println();
}

// include checksum
int weather_sensor::compare_data(frame_idx fr2) {
  FUNC_BEGIN;
  // manual bounds check on frame_idx
  BOUNDS(fr2, 0, frame_idx_MAX, F("frame_idx"));
  for (int i = 0; i < m_MAX; i++) {
    if (m_frame->at(i) != m_frame_array[fr2].at(i)) {
      return -1;
    }
  }
  return 0;
 catch_block:
  ser_print(F("error:  frame index")); ser_println(fr2);
  return -1;
}

// require select_sensor() called;
void weather_sensor::pack_client_request_frame(void) {
  FUNC_BEGIN;
  // CRC high order byte comes first in output
  uint16_t cHL = -1;
  static format_string msg1 = F("pack_client_request_frame when m_weather_ctx != wr_CTX");
  static format_string msg2 = F("pack_client request_frame when m_frame_idx != frame_idx_O or T");
  ERROR_CONDITION((m_weather_ctx == wr_CTX), msg1);
  ERROR_CONDITION((m_frame_idx != frame_idx_I), msg2);
  m_frame->at(0) = m_device_idx;
  m_frame->at(1) = 3;
  m_frame->at(2) = 0;
  m_frame->at(3) = 0;
  m_frame->at(4) = 0;
  m_frame->at(5) = m_data_length_words;
  ModRTU_CRC(cHL);
  pack_CRC(cHL);
  return;
 catch_block:
  ;
}

// require select_sensor() called;
// require data packed separately;
void weather_sensor::pack_server_reply_frame(void) {
  FUNC_BEGIN;
  // CRC high order byte comes first in output
  uint16_t cHL = -1;
  static format_string msg1 = F("pack_server_reply_frame when m_weather_ctx != wr_CRX");
  static format_string msg2 = F("pack_server_reply_frame when m_frame_idx != frame_idx_O");
  ERROR_CONDITION((m_weather_ctx == wr_CRX),    msg1);
  ERROR_CONDITION((m_frame_idx == frame_idx_O), msg2);
  m_frame->at(0) = m_device_idx;
  m_frame->at(1) = 3;
  m_frame->at(2) = m_data_length_words * 2;
  ModRTU_CRC(cHL);
  pack_CRC(cHL);
  return;
 catch_block:
  ;
}

// require select_sensor() called;
/*
  inline byte *weather_modbus::modbus_registers(void) {
  return &weather_sensor::m_frame(PACK_OUTPUT_OFFSET);
  }
*/

// require select_sensor() called;
inline BBUF modbus_registers(void) {
  // return weather_sensor::m_frame;
  return m_frame;
}

// pack buf with values from wdat ____IN_ORDER_OF____ index in idx_set;
// word index in register = 2 * float_idx; 16 byte word; 32 byte float;
void pack_weather_data(BBUF buf, weather_data &wdat, WIS &idx_set) {
  FUNC_BEGIN;
  bool debug = false;
  // if float_idx = 3 this is stored as 3rd float == 12th byte in buffer;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int entry_idx = idx_set.m_idx[jjj];
    int modbus_float_address = jjj;
    float valf;
    wdat.get(entry_idx, valf);
    pack_register_float(buf, modbus_float_address, valf);
    if (debug) {
      float echo_valf = valf;
      format_string echo_name;
      wdat.get_name_mine(entry_idx, echo_name);
      ser_print(F("packed modbus valf = ")); ser_println(valf);
      ser_print(F("get wdat echo_name = ")); ser_println(echo_name);
      ser_print(F("get wdat echo_valf = ")); ser_println(echo_valf);
    }
  }
 catch_block:
  wexception.print();
}


void unpack_weather_data(BBUF buf, weather_data &wdat, WIS &idx_set, bool convert) {
  FUNC_BEGIN;
  bool debug = false;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int entry_idx = idx_set.m_idx[jjj];
    // we invented a modbus float register which is simply two modbus registers = 4 bytes;
    int modbus_float_address = jjj;
    float valf;
    unpack_register_float(buf, modbus_float_address, valf);
    // all conversions are done as soon as the data enters the c++ program;
    // my modbus servers/sensors have already converted the data;
    // the modbus servers wind (renke) and temp/humidity (sht30) have not converted it yet;
    // unit conversion also divides some of these by 100.0 and so on;
    wdat.set(entry_idx, valf, convert);
    if (debug) {
      float echo_valf;
      format_string echo_name;
      wdat.get(entry_idx, echo_valf);
      wdat.get_name_mine(entry_idx, echo_name);
      ser_print(F("unpacked modbus valf = ")); ser_println(valf);
      ser_print(F("saved wdat echo_name = ")); ser_println(echo_name);
      ser_print(F("saved wdat echo_valf = ")); ser_println(echo_valf);
    }
  }
 catch_block:
  wexception.print();
}

// when the order of entries in sensor output is determined by the manufacturer (e.g. wind; temp;)
// WIS idx_set must list entries in same order they occur in sensor output;
// and we assume sensor output registers are in consecutive positions;
// e.g.
// idx_set.m_idx[jjj=0] = WDAT_wind_speed    ; modbus_register_address = 0;
// idx_set.m_idx[jjj=1] = WDAT_wind_direction; modbus_register_address = 1;
// idx_set.m_idx[jjj=2] = WDAT_wind_speed_max; modbus_register_address = 2;
// idx_set.m_idx[jjj=3] = WDAT_wind_rating   ; modbus_register_address = 3;
// modbus register address is in register words = 2 bytes;
void unpack_weather_data_uint16_t(BBUF buf, weather_data &wdat, WIS &idx_set, bool convert) {
  FUNC_BEGIN;
  bool debug = false;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int entry_idx = idx_set.m_idx[jjj];
    int modbus_register_address = jjj;
    uint16_t valu;
    unpack_register(buf, modbus_register_address, valu);
    // fixme  necessary ????;
    // handles twos complement negative temperatures stored in uint16_t;
    int16_t vali = ((int  ) valu);
    float   valf = ((float) vali);
    // all conversions are done as soon as the data enters the c++ program;
    // my modbus servers/sensors have already converted the data;
    // the modbus servers wind (renke) and temp/humidity (sht30) have not converted it yet;
    // unit conversion also divides some of these by 100.0 and so on;
    wdat.set(entry_idx, valf, convert);
    if (debug) {
      float echo_valf;
      format_string echo_name;
      wdat.get(entry_idx, echo_valf);
      wdat.get_name_mine(entry_idx, echo_name);
      ser_print(F("unpacked modbus valf = ")); ser_println(valf);
      ser_print(F("saved wdat echo_name = ")); ser_println(echo_name);
      ser_print(F("saved wdat echo_valf = ")); ser_println(echo_valf);
    }
  }
}

void weather_modbus::pack_sensor(device_idx ws, weather_data &wdat) {
  FUNC_BEGIN;
  bool debug = false;
  uint16_t cHL = -1;
  select_server_output(ws);
  BBUF buf = modbus_registers();
  switch (ws) {
//case ws_WIND:
//    ::pack_weather_data(buf, wdat, WIS_outside_wind );
//    break;
//case ws_TEMP:
// ::pack_weather_data(buf, wdat, WIS_outside_temp ); break;
  case ws_LIGH:
    if (debug) ser_println(F("pack_sensor light"));
    ::pack_weather_data(buf, wdat, WIS_outside_light1);
    break;
  case ws_RAIN:
    if (debug) ser_println(F("pack_sensor rain"));
    ::pack_weather_data(buf, wdat, WIS_outside_rain1);
    break;
  default:
    ERROR_CONDITION(false, F("pack_sensor; bad ws; fatal"));
    break;
  }
  buf_ModRTU_CRC(cHL);
  buf_pack_CRC(cHL);
  return;
 catch_block:
  ;
}

void weather_modbus::unpack_sensor(device_idx ws, weather_data &wdat) {
  FUNC_BEGIN;
  bool debug = false;
  select_server_input(ws);
  BBUF buf = modbus_registers();
  bool convert_not_yet_commercial = true;
  bool convert_already_my_sensor = false;
  switch (ws) {
  case ws_WIND:
    if (debug) ser_println(F("unpack_sensor wind"));
    ::unpack_weather_data_uint16_t(buf, wdat, WIS_outside_wind0, convert_not_yet_commercial);
    break;
  case ws_TEMP:
    if (debug) ser_println(F("unpack_sensor temp"));
    ::unpack_weather_data_uint16_t(buf, wdat, WIS_outside_temp0, convert_not_yet_commercial);
    break;
  case ws_LIGH:
    if (debug) ser_println(F("unpack_sensor light"));
    ::unpack_weather_data(buf, wdat, WIS_outside_light1, convert_already_my_sensor);
    break;
  case ws_RAIN:
    if (debug) ser_println(F("unpack_sensor rain"));
    ::unpack_weather_data(buf, wdat, WIS_outside_rain1, convert_already_my_sensor);
    break;
  default:
    ERROR_CONDITION(false, F("unpack_sensor; bad ws; fatal"));
    break;
  }
  return;
 catch_block:
  ;
}

////////////////////////////////////////////////////////////////////////////////////

// client tx (ctx) or server tx (crx)
// require m_frame_array[frame_idx_O] has data;
// require select_*_output() called for print();
#define DEBUG_WRITE false
int weather_sensor::write(int &nwritten) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  nwritten = 0;
  // write is always frame_idx_O
  ERROR_CONDITION((m_frame_idx == frame_idx_O), F("write not using output frame"));
  // should stay in receive mode whenever not writing
  // digitalWrite(pin_RTS, RS485Transmit);
  divider_line_error_lead();
  ser_print(F("write = "));
  for (int b = 0; b < m_MAX; b++) {
    ser_print(F(" "));
    print_hex_8(m_frame->at(b));
  }
  ser_println();
  nwritten = RS485Serial.write(m_frame->get_data(), m_MAX);
  RS485Serial.flush();
  // allow a modbus frame delay to indicate end of transmission
  delayMicroseconds(m_timeout_T35_usec);
  // should stay in receive mode whenever not writing
  // digitalWrite(pin_RTS, RS485Receive);
  ERROR_CONDITION((nwritten == m_MAX), F("nwritten != m_MAX"));
  return 0;
 catch_block:
  if (nwritten != m_MAX) {
    divider_line_error_lead();
    ser_print(F("nwritten = ")); ser_print(nwritten);
    ser_print(F(" m_MAX = ")); ser_println(m_MAX);
    wexception.print();
  }

  print(F("OUTPUT"));

  return 1;
}

void weather_sensor::peek_byte(byte imodbus, byte &B) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  if (imodbus >= DATA_MAX) {
    m_overflow_occur = true;
    return;
  }
  while (!RS485Serial.available()) {
    delayMicroseconds(m_timeout_T15_usec);
    m_time_read_usec += m_timeout_T15_usec;
    if (((int32_t) m_time_read_usec) > ((int32_t) m_timeout_read_usec)) {
      m_timeout_occur = true;
      // terminate echo
      ser_println();
      ser_print(F("m_time_read_usec    = ")); ser_println(m_time_read_usec);
      ser_print(F("m_timeout_read_usec = ")); ser_println(m_timeout_read_usec);
      return;
    }
  }
  B = RS485Serial.peek();
}

// byte ngap; buffer is only 64 bytes on arduino uno;
// byte input/output
bool weather_sensor::match_gap(byte &ngap) {
  FUNC_BEGIN;
  byte B = 0;
  while (B == 0) {
    peek_byte(ngap, B);
    if (m_overflow_occur || m_timeout_occur) return false;
    if (B == 0) {
      RS485Serial.read();
      //      m_frame->at(ngap) = B;
      ngap++;
    }
  }
  return true;
}

bool weather_sensor::match_byte(byte &imodbus, bool specific, byte BT, byte &B) {
  FUNC_BEGIN;
  B = 0;
  peek_byte(imodbus, B);
  if (m_timeout_occur || m_overflow_occur) return false;
  B = RS485Serial.read();
  ser_print(F(" "));
  print_hex_8(B);
  m_frame->at(imodbus) = B;
  imodbus++;
  if (specific && (B != BT)) {
    return false;
  }
  return true;
}

void match_flush_input_buffer(byte &nflush) {
  FUNC_BEGIN;
  nflush = 0;
  divider_line_error_lead();
  ser_print(F("flush = "));
  while (RS485Serial.available()) {
    byte B = RS485Serial.read();
    nflush++;
    ser_print(F(" "));
    print_hex_8(B);
  }
  ser_println();
}

// igap input/output
int weather_sensor::match_modbus(byte &ngap, byte &imodbus, device_idx1 &ws, weather_ctx1 &wr) {
  FUNC_BEGIN;
  int ierr = -1;
  byte B = 0;
  byte leader = 0;
  byte nbyte = 0;
  byte nword = 0;
  byte CH = 0;
  byte CL = 0;
  if (!match_gap(ngap)) return -1;
  ser_print(F("ngap = ")); ser_println(ngap);
  divider_line_error_lead();
  ser_print(F("echo  = "));
  // imodbus = igap;
  // server id ws
  if (!match_byte(imodbus, false, 0, B)) return -1;
  ws = B;
  // function code 3
  if (!match_byte(imodbus, true, 3, B)) return -1;
  // nbyte
  if (!match_byte(imodbus, false, 0, B)) return -1;
  leader = B;
  // now wr is known;
  // if (wr_target == wr) assemble frame
  if (leader == 0) {
    wr = wr_CTX;
    if (!match_byte(imodbus, true, 0, B)) return -1;
    if (!match_byte(imodbus, true, 0, B)) return -1;
    if (!match_byte(imodbus, false, 0, B)) return -1;
    nword = B;
    nbyte = B * 2;
    if (!match_byte(imodbus, false, 0, B)) return -1;
    CH = B;
    if (!match_byte(imodbus, false, 0, B)) return -1;
    CL = B;
  } else {
    wr = wr_CRX;
    nbyte = leader;
    if (nbyte % 2 != 0) {
      ser_print(" (error:  modbus nbyte odd) ");
      return -1;
    }
    nword = nbyte / 2;
    for (int w = 0; w < nword; w++) {
      if (!match_byte(imodbus, false, 0, B)) return -1;
      if (!match_byte(imodbus, false, 0, B)) return -1;
    }
    if (!match_byte(imodbus, false, 0, B)) return -1;
    CH = B;
    if (!match_byte(imodbus, false, 0, B)) return -1;
    CL = B;
  }
  return 0;
}

void weather_sensor::print_ws(device_idx1 ws) {
  FUNC_BEGIN;
  switch (ws) {
  case ws_WIND:     ser_print(F("WIND  ")); break;
  case ws_TEMP:     ser_print(F("TEMP  ")); break;
  case ws_LIGH:     ser_print(F("LIGHT ")); break;
  case ws_RAIN:     ser_print(F("RAIN  ")); break;
  case ws_INVALID:  ser_print(F("CLIENT")); break;
  default:          ser_print(F("ERROR ")); break;
  }
}

void weather_sensor::print_wr(weather_ctx1 wr) {
  FUNC_BEGIN;
  switch (wr) {
  case wr_CTX:      ser_print(F("CTX REQUEST")); break;
  case wr_CRX:      ser_print(F("CRX DATA   ")); break;
  case wr_INVALID:  ser_print(F("NOT VALID  ")); break;
  default:          ser_print(F("ERROR      ")); break;
  }
}

void weather_sensor::print_ws_wr(format_string name, device_idx1 ws, weather_ctx1 wr) {
  FUNC_BEGIN;
  divider_line_error_lead();
  ser_print(name);
  ser_print(F(" "));
  print_ws(ws);
  ser_print(F(" "));
  print_wr(wr);
  ser_println();
}

// blocking read with timeout
// client rx (crx) or server rx (ctx)
// on exit m_frame_array[frame_idx_I] has data;
// require select_*_input() called for print();
int weather_sensor::read(int timeout_read_msec, weather_ctx wr, bool &match) {
  FUNC_BEGIN;
  CHECK_MEMORY_FREE_MIN;
  byte ngap1 = 0;
  byte ngap2 = 0;
  byte imodbus1 = 0;
  byte imodbus2 = 0;
  byte ws1 = ws_INVALID;
  byte ws2 = ws_INVALID;
  byte wr1 = wr_INVALID;
  byte wr2 = wr_INVALID;
  byte nflush = 0;
  bool res_match1;
  bool res_match2;
  bool complete;
  m_nread = 0;
  device_idx ws = m_device_idx;
  // read is always frame_idx_I
  ERROR_CONDITION((m_frame_idx == frame_idx_I), F("read not using input frame"));
  // fixme is there any harm in this redundant statement
  // fixme does this init Serial buffer ????;
  // digitalWrite(pin_RTS, RS485Receive);
  // put delay here since pin is set high here;
  // client:  wait for server to perform measurement or calculation;
  // server:  wait for client to process other servers;
  //  delay(timeout_read_msec);
  m_timeout_read_usec = timeout_read_msec * 1000;
  delayMicroseconds(m_timeout_T35_usec);   // added 10/5/22;
  // we can not afford FRAME_MAX so just fit in DATA_MAX; we only need that many chars
  match = false;
  // device_idx ws1 = 0;
  // device_idx ws2 = 0;
  // weather_ctx wr1 = -1;
  // weather_ctx wr2 = -1;
  // ws type is (device_idx  + ws_INVALID);
  // wr type is (weather_ctx + wr_INVALID);
  // ws not valid device_idx;
  // wr not valid weather_ctx;
  m_overflow_occur = false;
  m_timeout_occur = false;
  m_nread = 0;
  // m_nread = RS485Serial.readBytes(m_frame->get_data(), DATA_MAX);
  // for (int b = 0; b < m_nread; b++) {
  // ser_print(F(" "));
  // print_hex_8(m_frame->at(b));
  // }
  // ser_println();
  // special case for error catch
  m_time_read_usec = 0;
  res_match1 = !match_modbus(ngap1, imodbus1, ws1, wr1);
  // replace match2 with this:
  ngap2 = 0;
  imodbus2 = imodbus1;
  res_match2 = false;
  // bool res_match2 = !match_modbus(ngap2, imodbus2, ws2, wr2);
  // terminate echo printout;
  ser_println();
  ::match_flush_input_buffer(nflush);
  // remove string of zeros
  //  if (imodbus2 == 0) {
  //    m_nread = 0;
  //  }
  m_nread = imodbus2;
  complete = (m_nread > 0) && (res_match1 || res_match2);
  /*
    ser_print(F("igap1      = ")); ser_println(igap1);
    ser_print(F("imodbus1   = ")); ser_println(imodbus1);
    ser_print(F("igap2      = ")); ser_println(igap2);
    ser_print(F("imodbus2   = ")); ser_println(imodbus2);
    ser_print(F("nflush     = ")); ser_println(nflush);
    ser_print(F("nread      = ")); ser_println(m_nread);
    ser_print(F("incomplete = ")); ser_println(incomplete);
  */
  ERROR_CONDITION((!m_timeout_occur), F("timeout"));
  ERROR_CONDITION((!m_overflow_occur), F("overflow"));
  // could just skip match errors and catch them on compare_data();
  ERROR_CONDITION((complete), F("incomplete"));
  // remove gap
  if ((ws1 == ws) && (wr1 == wr)) {
    match = true;
    //  for (int b = igap1; b < imodbus1; b++) {
    //      m_frame->at(b - igap1) = m_frame->at(b);
    //    }
    // }
    //else if ((ws2 == ws) && (wr2 == wr)) {
    //  match = true;
    //  for (int b = igap2; b < imodbus2; b++) {
    //    m_frame->at(b - igap2) = m_frame->at(b);
    //  }
  } else {
    // ERROR_RETURN((m_nread != 0), F("underflow frame"));
    ERROR_SILENT((m_nread != 0), F("underflow frame"), (0));
    print_ws_wr(F("T"), ws,  wr ); 
    print_ws_wr(F("1"), ws1, wr1);
    print_ws_wr(F("2"), ws2, wr2);
    // ERROR_RETURN((false), F("ws and wr match not found"));
    ERROR_SILENT((false), F("nomatch"), (0));
  }
  // ================================================

  print_ws_wr(F("T"), ws,  wr ); 
  print_ws_wr(F("1"), ws1, wr1);
  print_ws_wr(F("2"), ws2, wr2);
  divider_line_result(RESULT_SUCCESS, F("read"));
  print(F("INPUT"));
  
  return 0;
 catch_block:
  //  if (m_nread <= 0) {
  //    divider_line_error_lead(); ser_println(F("... blank ..."));
  //  } else {
  if (m_nread > 0) {
    print_ws_wr(F("T"), ws,  wr ); 
    print_ws_wr(F("1"), ws1, wr1);
    print_ws_wr(F("2"), ws2, wr2);
    if (res_match1 || res_match2) {
      wexception.print();
    }
    divider_line_result(RESULT_FAIL, F("read"));
  }
  return 1;
}

void weather_modbus::select_client_output(device_idx ws) {
  m_pattern = &m_weather_buf.at(ws);
  m_pattern->select_sensor(wr_CTX, frame_idx_O);
}

void weather_modbus::select_client_input(device_idx ws) {
  weather_ctx wr = wr_CRX;
  m_pattern = &m_weather_buf.at(ws);
  m_pattern->select_sensor(wr_CRX, frame_idx_I);
}

void weather_modbus::select_server_temp(device_idx ws) {
  m_pattern = &m_weather_buf.at(ws);
  m_pattern->select_sensor(wr_CTX, frame_idx_T);
}

void weather_modbus::select_server_output(device_idx ws) {
  m_pattern = &m_weather_buf.at(ws);
  m_pattern->select_sensor(wr_CRX, frame_idx_O);
}

void weather_modbus::select_server_input(device_idx ws) {
  m_pattern = &m_weather_buf.at(ws);
  m_pattern->select_sensor(wr_CTX, frame_idx_I);
}

// fixme require weather_ctx_wr() == wr_CRX on exit; not needed;
// reply server input data packed in m_frame_array[frame_idx_I] on exit;
int weather_modbus::client_tx_rx(device_idx ws, int timeout_read_msec) {
  FUNC_BEGIN;
  uint16_t cHL = -1;
  // wexception.clear_all();
  int res = -1;
  bool done_output = false;
  bool done_input = false;
  bool match = false;
  divider_line_error_lead(); ser_println(F("client_tx_rx begin"));
  // -----------------
  byte nflush = 0;
  // ::match_flush_input_buffer(nflush);
  // -----------------
  select_client_output(ws);
  buf_zero();
  buf_pack_client_request_frame();
  int nwritten;
  res = buf_write(nwritten);
  done_output = true;
  ERROR_RETURN((nwritten > 0), F("nwritten <= 0"));
  select_client_input(ws);
  buf_zero();
  res = buf_read(timeout_read_msec, wr_CRX, match);
  ERROR_RETURN((res == 0), F("read FAIL"));
  select_client_input(ws);
  // ERROR_RETURN((buf_nread() > 0), F("nread <= 0"));
  ERROR_SILENT((buf_nread() != 0), F(""), (-1));
  // ERROR_RETURN(match, F("read FAIL"));
  ERROR_SILENT((match), F("nomatch"), (-1));
  // on error do not print input unless buf_read passed
  done_input = true;
  buf_ModRTU_CRC(cHL);
  res = buf_compare_CRC(cHL);
  ERROR_RETURN((res == 0), F("crc FAIL"));

  select_client_output(ws);
  buf_print(F("OUTPUT"));
  select_client_input(ws);
  buf_print(F("INPUT"));

  divider_line_result(RESULT_SUCCESS, F("client_tx_rx"));
  return 0;
 catch_block:
  // if (buf_nread() <= 0) {
  // divider_line_error_lead(); ser_println(F("... blank ..."));
  // } else {
  if (buf_nread() > 0) {
    wexception.print();

    if (done_output) {
      select_client_output(ws);
      buf_print(F("OUTPUT"));
    }
    if (done_input) {
      select_client_input(ws);
      buf_print(F("INPUT"));
    }
  }
  divider_line_result(RESULT_FAIL, F("client_tx_rx"));
  select_client_input(ws);
  return -1;
}

#define DEBUG_SERVER_RX_TX false
// require reply server output data already packed in m_frame_array[frame_idx_O] on entry;
int weather_modbus::server_rx_tx(device_idx ws, int timeout_read_msec) {
  FUNC_BEGIN;
  // wexception.clear_all();
  int res = -1;
  bool done_input = false;
  bool done_output = false;
  divider_line_error_lead(); ser_println(F("server_rx_tx begin"));
  select_server_temp(ws);
  buf_zero();
  buf_pack_client_request_frame();
  select_server_input(ws);
  buf_zero();
  bool match = false;
  res = buf_read(timeout_read_msec, wr_CTX, match);
  ERROR_RETURN((res == 0), F("read FAIL"));
  // ERROR_RETURN((buf_nread() > 0), F("read FAIL"));
  ERROR_SILENT((buf_nread() != 0), F(""), (-1));
  // ERROR_RETURN(match, F("read FAIL"));
  ERROR_SILENT((match), F("nomatch"), (-1));
  // on error do not print input unless buf_read passed
  done_input = true;
  // compare only accepts exact copy of client_request_frame including checksum;
  // checksum alone would be good enough;
  res = buf_compare_data(frame_idx_T);
  ERROR_RETURN((res == 0), F("compare FAIL"));
  select_server_output(ws);
  // do not call buf_zero();
  // do not overwrite data;
  // pack with the template leaving register data unchanged;
  buf_pack_server_reply_frame();
  int nwritten;
  res = buf_write(nwritten);
  done_output = true;
  ERROR_RETURN((nwritten > 0), F("nwritten <= 0"));
  ERROR_RETURN((res == 0), F("write FAIL"));

  select_server_input(ws);
  buf_print(F("INPUT"));  // fixme added
  select_server_output(ws);
  buf_print(F("OUTPUT"));  // fixme added
  
  divider_line_result(RESULT_SUCCESS, F("server_rx_tx"));
  return 0;
 catch_block:
  // if (buf_nread() <= 0) {
  // divider_line_error_lead(); ser_println(F("... blank ..."));
  // } else {
  if (buf_nread() > 0) {
    wexception.print();
    if (done_input) {
      select_server_input(ws);
      buf_print(F("INPUT"));
    }
    if (done_output) {
      select_server_output(ws);
      buf_print(F("OUTPUT"));
    }
  }
  divider_line_result(RESULT_FAIL, F("server_rx_tx"));
  return -1;
}
////////////////////////////////////////////////////////////////////////////////////

int weather_modbus::crc_debug_sensor(format_string msg, device_idx ws, uint16_t cHL) {
  FUNC_BEGIN;
  int res = -1;
  // share one with crc_debug
  ser_print(F("test CRC function "));
  ser_print(msg);
  divider_line(F("."), 12);
  ser_println();
  select_server_temp(ws);
  buf_pack_client_request_frame();
  res = buf_compare_CRC(cHL);
  return res;
}

int weather_modbus::crc_debug(void) {
  FUNC_BEGIN;
  //  weather_exception e;    // fixme
  int res = 0;   // zero this time
  res |= crc_debug_sensor(F("wind"),  ws_WIND, 0x4409);
  res |= crc_debug_sensor(F("temp"),  ws_TEMP, 0xC438);
  res |= crc_debug_sensor(F("light"), ws_LIGH, 0x4427);
  res |= crc_debug_sensor(F("rain"),  ws_RAIN, 0x4453);
  ERROR_RETURN((res == 0), F("halt"));
  ser_println(F("success !!!!!!!!"));
  return 0;
 catch_block:
  wexception.print();
  weather_halt();
  return -1;
}

void weather_modbus::setup_buffers(void) {
  FUNC_BEGIN;
  // length in bytes CTX = 8;
  // length in bytes CRX = 5 + 2 * data_length_words;

  // data sent is determined by
  // WIS_outside_wind0    4 uint16_t =  4 words =  8 bytes;
  // WIS_outside_temp0    2 uint16_t =  2 words =  4 bytes;
  // WIS_outside_light1  10 floats   = 20 words = 40 bytes;
  // WIS_outside_rain1    8 floats   = 16 words = 32 bytes;
  
  halt_state = false;

  // CTX (client transmit) messages; request sent to   sensor/server;
  //              0,   1,   2,   3,   4,   5,   6,   7,   8
  // wind_ctx = { 0x01,0x03,0x00,0x00,0x00,0x04,0x44,0x09 };
  // temp_ctx = { 0x02,0x03,0x00,0x00,0x00,0x02,0xC4,0x38 };
  // ligh_ctx = { 0x03,0x03,0x00,0x00,0x00,0x14,0x44,0x27 };
  // rain_ctx = { 0x04,0x03,0x00,0x00,0x00,0x10,0x44,0x53 };

  // CRX (client receive ) messages; reply   sent from sensor/server;
  //              0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,
  //              16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
  //              32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44
  // wind_crx = { 0x01,0x03,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF };
  // temp_crx = { 0x02,0x03,0x04,0x00,0x00,0x00,0x00,0xFF,0xFF };
  // ligh_crx = { 0x03,0x03,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  //              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  //              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF };
  // rain_crx = { 0x04,0x03,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  //              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF };

  m_weather_buf.at(ws_WIND).setup(ws_WIND,4);  // length = 5+2* 4=13 bytes;
  m_weather_buf.at(ws_TEMP).setup(ws_TEMP,2);  // length = 5+2* 2= 9 bytes;
  m_weather_buf.at(ws_LIGH).setup(ws_LIGH,20); // length = 5+2*20=45 bytes; 20 = 0x14; 40 = 0x28;
  m_weather_buf.at(ws_RAIN).setup(ws_RAIN,16); // length = 5+2*16=37 bytes; 16 = 0x10; 32 = 0x20;
  // fixme SELECT
  // select_ws_wr(ws_LIGH, wr_CTX);
  select_server_temp(ws_LIGH);
}

// eee eof
