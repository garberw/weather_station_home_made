#include <weather_lib_util.h>
#include <weather_lib_timer.h>

// n is just a check for the user that these occur in matched pairs
void weather_timer::start(byte n) {
  FUNC_BEGIN;
  ERROR_CONDITION(m_ntime >= 0, F("weather_timer m_ntime >= 0"));
  ERROR_CONDITION(m_ntime < WEATHER_TIMER_MAX, F("weather_timer m_ntime >= 0"));
  m_time_msec[m_ntime] = millis();
  ERROR_CONDITION((n == m_ntime), F("mismatched weather_timer pair;"));
  m_ntime++;
  ERROR_CONDITION(m_ntime < WEATHER_TIMER_MAX, F("weather_timer overflow stack;"));
  return;
 catch_block:
  if (m_ntime >= WEATHER_TIMER_MAX) {
    m_ntime = WEATHER_TIMER_MAX - 1;
  }
  wexception.print();
}
  
// n is just a check for the user that these occur in matched pairs
void weather_timer::stop(byte n, format_string title) {
  FUNC_BEGIN;
  uint32_t t1;
  uint32_t t2;
  m_ntime--;
  ERROR_CONDITION((n == m_ntime), F("mismatched weather_timer pair;"));
  ERROR_CONDITION(m_ntime >= 0, F("weather_timer m_ntime >= 0"));
  ERROR_CONDITION(m_ntime < WEATHER_TIMER_MAX, F("weather_timer m_ntime >= 0"));
  t1 = m_time_msec[m_ntime];
  t2 = millis();
  ser_print(F("time               = "));
  print_uint32((uint32_t) (t2 - t1));
  ser_print(F(" msec "));
  ser_println(title);
  return;
 catch_block:
  wexception.print();
}

// eee eof
