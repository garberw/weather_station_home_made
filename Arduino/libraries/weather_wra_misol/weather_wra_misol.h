#ifndef weather_wra_misol_h
#define weather_wra_misol_h
/*
  misol rain gauge driver
  It reads in inches:
  rain_daily
  rain_hourly
  rain_daily_until_last_hour
  and outputs them to the serial monitor.
*/

#include <weather_lib_data.h>

/* typedefs begin ----------------------------------------------- */

class weather_misol {
public:
  static void count_bucket_tip_ISR(void);
  void setup_pins(void);
  void setup_data(void);
  bool new_tips(void) { return (m_rain_bucket_tips > m_rain_bucket_prev); }
  double rain_mm(void) { return ((float) m_rain_bucket_tips) * RAIN_HEIGHT_PER_BUCKET_TIP_MM; }
  double rain_inch(void) { return ((float) m_rain_bucket_tips) * RAIN_HEIGHT_PER_BUCKET_TIP_INCH; }
  void copy_to_wdat(weather_data &wdat);
  void advanced_read(weather_data &wdat);
  static volatile uint32_t m_last_debounce_time_msec;
  static volatile uint16_t m_rain_bucket_prev;
  static volatile uint16_t m_rain_bucket_tips;
  static volatile uint16_t m_rain_reset;
}; // class weather_misol;

/* typedefs end ----------------------------------------------- */
#endif
// eee eof
