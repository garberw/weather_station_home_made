#ifndef weather_wra_ltr390_h
#define weather_wra_ltr390_h
// This library is a driver for the Adafruit LTR390 UV sensor.  also luminosity and IR.

#include <weather_lib_data.h>

/* typedefs begin ----------------------------------------------- */

class weather_ltr390 {
public:
  void  setup                  (void);
  byte  gain_factor            (void);
  byte  resolution_factor      (void);
  float resolution_integration (void);
  // time in msec;
  float integration_time       (void) { return resolution_integration() * 100.0; }
  void  set_mode_uvs           (void);
  void  set_mode_als           (void);
  float UV_index               (uint32_t &uvs);
  float lux_linear             (uint32_t &light1);
  void  advanced_read          (weather_data &wdat);
  bool  new_data_available     (void);
  float UVI_factor             (void);
  float LUX_factor             (void);
  int stub;
}; // class weather_ltr390;

/* typedefs end ----------------------------------------------- */
#endif
// eee eof
