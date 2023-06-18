#ifndef weather_lib_modbus_macros_h
#define weather_lib_modbus_macros_h

/*
  Modbus RTU Weather Library

  This library provides Modbus RTU communication for
  the client to request weather sensor data and for
  the server (weather sensor) to reply.
  Sensors include:

  wind
  temperature and humidity 
  light and UV
  rain
*/

/* macros begin ----------------------------------------------- */

// #define pin_RTS        5  // RE and DE on MAX485 chip
// #define pin_RX         6  // RO on MAX485
// #define pin_TX         7  // DI on MAX485
// #define RS485Transmit  HIGH
// #define RS485Receive   LOW

#define FRAME_MAX      128
// must be >= m_MAX for receive for sensor with largest m_data_length_words
// formula is DATA_MAX >= 5 + 2 * m_MAX
// largest m_MAX is 12 for light
// DATA_MAX >= 29
// #define DATA_MAX       29

#define ARDUINO_BUFFER_SIZE 64
#define DATA_MAX ARDUINO_BUFFER_SIZE

#define PACK_OUTPUT_OFFSET 3  // e.g. address(HUMIDITY)=PACK_OUTPUT_OFFSET+ 2 * TEMP_CRX_HUMIDITY

#define frame_idx_I 0
#define frame_idx_O 1
#define frame_idx_T 2
#define frame_idx_MAX 3

#define wr_CTX true      // client TX (server RX) true
#define wr_CRX false     // client TX (server RX) false
#define wr_INVALID 255

// single has these value for loop counter
#define ws_LOOPC_COUNT 0
#define ws_LOOPC_MAX   1

// inside has these values for loop counter
#define ws_INSIDE_TEMP 0
#define ws_INSIDE_MAX  1

// outside has these values for loop counter
#define ws_TSYS  0   // not a weather sensor server just local tsys sensors
#define ws_WIND  1
#define ws_TEMP  2
#define ws_LIGH  3
#define ws_LIGHT 3
#define ws_RAIN  4
#define ws_MAX   5
#define ws_INVALID 255

/*
  enum device_idx {
  ws_WIND = 1;
  ws_TEMP = 2;
  ws_LIGH = 3;
  ws_LIGHT = 2;
  ws_RAIN = 4;
  };
  enum device_idx1 {
  ws_WIND = 1;
  ws_TEMP = 2;
  ws_LIGH = 3;
  ws_LIGHT = 2;
  ws_RAIN = 4;
  ws_INVALID = -1;
  };
*/

// addresses of registers packed in input buffer relative to m_data[0];
#define WIND_CRX_SPEED        0
#define WIND_CRX_DIRECTION    1
#define WIND_CRX_SPEED_MAX    2
#define WIND_CRX_RATING       3
#define WIND_MAX              4

#define TEMP_CRX_TEMPERATURE  0
#define TEMP_CRX_HUMIDITY     1
#define TEMP_MAX              2

#define LIGHT_CRX_FULL_H              0
#define LIGHT_CRX_FULL_L              1
#define LIGHT_CRX_IR_H                2
#define LIGHT_CRX_IR_L                3
#define LIGHT_CRX_VISIBLE_H           4
#define LIGHT_CRX_VISIBLE_L           5
#define LIGHT_CRX_LUX_H               6
#define LIGHT_CRX_LUX_L               7
#define LIGHT_CRX_LUX2_H              8
#define LIGHT_CRX_LUX2_L              9
#define LIGHT_CRX_UV_H               10
#define LIGHT_CRX_UV_L               11
#define LIGHT_CRX_CASE_TEMPERATURE_H 12
#define LIGHT_CRX_CASE_TEMPERATURE_L 13
#define LIGHT_CRX_CASE_HUMIDITY_H    14
#define LIGHT_CRX_CASE_HUMIDITY_L    15
#define LIGHT_CRX_CPU_TEMPERATURE_H  16
#define LIGHT_CRX_CPU_TEMPERATURE_L  17
#define LIGHT_MAX                    18

#define RAIN_CRX_BUCKET_TIPS          0
#define RAIN_CRX_RESET                1
// fixme new vars
#define RAIN_CRX_CASE_TEMPERATURE_H   2
#define RAIN_CRX_CASE_TEMPERATURE_L   3
#define RAIN_CRX_CASE_HUMIDITY_H      4
#define RAIN_CRX_CASE_HUMIDITY_L      5
#define RAIN_CRX_CPU_TEMPERATURE_H    6
#define RAIN_CRX_CPU_TEMPERATURE_L    7
#define RAIN_MAX                      8

typedef byte device_idx;
typedef byte device_idx1;
typedef bool weather_ctx;
typedef byte weather_ctx1;
typedef byte frame_idx;

/* macros end ----------------------------------------------- */

#endif
// eee eof
