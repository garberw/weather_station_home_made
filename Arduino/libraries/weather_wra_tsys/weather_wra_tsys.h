#ifndef weather_wra_tsys_h
#define weather_wra_tsys_h
// cpu temp (TemperatureZero); case temp and humid (AHT20);
/*
  Modbus RTU Weather Library

  This library provides Modbus RTU communication for
  the client to request weather sensor data and for
  the server (weather sensor) to reply.
  Sensors include:

  wind
  temp and humid 
  light and UV
  rain
*/

#include <weather_lib_data.h>

/* typedefs begin ----------------------------------------------- */

class weather_tsys {
public:
  void setup(void);
  void advanced_read(float &case_temp, float &case_humid, float &cpu_temp);
  void advanced_read_client(weather_data &wdat);
  void advanced_read_light(weather_data &wdat);
  void advanced_read_rain(weather_data &wdat);
  int stub;
}; // class weather_tsys;

/* typedefs end ----------------------------------------------- */

#endif
// eee eof
