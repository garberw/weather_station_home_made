#ifndef weather_wra_tsl2591_h
#define weather_wra_tsl2591_h
// This library is a driver for the Adafruit TSL2591 digital luminosity and IR sensor.
// Dynamic Range: 600M:1
// Maximum Lux: 88K

#include <weather_lib_data.h>

/* macros begin ----------------------------------------------- */

#define LUX_PER_WPM2     (126.7)           // (126.7 lux )/(  1.0 W/m2)
#define WPM2_PER_LUX     (0.007892660)     // (  1.0 W/m2)/(126.7 lux )
#define LIGHT_OVERFLOW   (4294966000.0)
#define LIGHT_UNDERFLOW  (-LIGHT_OVERFLOW)

/* macros end ----------------------------------------------- */
/* typedefs begin ----------------------------------------------- */

class weather_tsl2591 {
public:
  void     setup              (void);
  void     print_details      (void);
  void     configure_sensor   (void);
  uint16_t gain_factor        (void);
  float    integration_time   (void);
  void     print_gain         (void);
  void     print_timing       (void);
  float    lux_linear_visible (uint16_t visible);
  float    lux_linear_full    (uint16_t full);
  float    lux_linear_full_factor    (void);
  float    lux_linear_visible_factor (void);
  float    lux_linear_advanced_factor(void);
  void     advanced_read      (weather_data &wdat);
}; // class weather_tsl2591;

/* typedefs end ----------------------------------------------- */

#endif
// eee eof
