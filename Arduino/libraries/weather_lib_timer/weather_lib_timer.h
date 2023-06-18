#ifndef weather_lib_timer_h
#define weather_lib_timer_h

#include <weather_lib_util.h>

#define WEATHER_TIMER_MAX 5

class weather_timer {
public:
  weather_timer(void) : m_ntime(0) { }
  void setup() {
    FUNC_BEGIN;
    m_ntime = 0;
  }
  void start(byte n);
  void stop(byte n, format_string title);
private:
  byte m_ntime;
  uint32_t m_time_msec[WEATHER_TIMER_MAX];
}; // class weather_timer;

#endif
// eee eof
