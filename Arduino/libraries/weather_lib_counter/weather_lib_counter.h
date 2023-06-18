#ifndef weather_lib_counter_h
#define weather_lib_counter_h

#include <weather_lib_util.h>
#include <weather_lib_modbus_macros.h>

template <int MAX>
class loop_counter {
public:
  byte m_count_max;
  byte m_count[MAX];
  bool m_done[MAX];
  format_string m_title[MAX];
  void set_all_count_zero(void) {
    for (byte c = 0; c < MAX; c++) {
      m_count[c] = 0;
    }
  }
  void set_all_done_false(void) {
    for (byte c = 0; c < MAX; c++) {
      m_done[c] = false;
    }
  }
  bool finished(byte c) {
    FUNC_BEGIN;
    BOUNDS(c, 0, MAX, F("finished"));
    return (m_done[c] || (m_count[c]  >= m_count_max));
  catch_block:
    wexception.print();
    return false;
  }
  bool finished_all(void) {
    for (byte c = 0; c < MAX; c++) {
      if (!finished(c)) return false;
    }
    return true;
  }
  bool give_up(byte c) {
    BOUNDS(c, 0, MAX, F("give_up"));
    return (m_count[c] >= m_count_max);
  catch_block:
    wexception.print();
    return true;
  }
  void increment(byte c, bool res) {
    FUNC_BEGIN;
    BOUNDS(c, 0, MAX, F("increment"));
    m_count[c]++;
    ser_print(F("attempt ")); ser_print(m_count[c]); ser_print(F(" "));
    ser_print(m_title[c]); ser_print(F(" "));
    if (res == WDAT_PASS) {
      m_done[c] = true;
      ser_println(F(" success"));
    } else if (m_count[c] >= m_count_max) {
      ser_println(F(" failure; giving up ......................;"));
    } else {
      ser_println(F(" failure; retry;"));
    }
    return;
  catch_block:
    ser_print(F("c   = ")); ser_println(c);
    ser_print(F("MAX = ")); ser_println(MAX);
  }
  void print_done(void) {
    for (byte c = 0; c < MAX; c++) {
      ser_print(F("done_")); ser_print(m_title[c]); ser_print(F("        = "));
      ser_println(m_done[c]);
    }
  }
}; // class loop_counter;

class loop_counter_single : public loop_counter<ws_LOOPC_MAX> {
public:
  typedef loop_counter<ws_LOOPC_MAX> BASE;
  loop_counter_single(void) {
    m_title[ws_LOOPC_COUNT] = F("sngl");
  }
  // special case loop counter with only one element MAX == 1; bad programming;
  void count_loopc(bool res) {
    increment(ws_LOOPC_COUNT, res);
  }
  bool give_up(void) {
    return BASE::give_up(ws_LOOPC_COUNT);
  }
}; // class loop_counter_single;

class loop_counter_outside : public loop_counter<ws_MAX> {
public:
  loop_counter_outside(void) {
    m_title[ws_TSYS] = F("tsys");
    m_title[ws_WIND] = F("wind");
    m_title[ws_TEMP] = F("temp");
    m_title[ws_LIGH] = F("ligh");
    m_title[ws_RAIN] = F("rain");
  }
  void count_wind (bool res) { increment(ws_WIND, res); }
  void count_temp (bool res) { increment(ws_TEMP, res); }
  void count_light(bool res) { increment(ws_LIGH, res); }
  void count_rain (bool res) { increment(ws_RAIN, res); }
  void count_tsys (bool res) { increment(ws_TSYS, res); }
  bool finished_wind (void) { return finished(ws_WIND); }
  bool finished_temp (void) { return finished(ws_TEMP); }
  bool finished_light(void) { return finished(ws_LIGH); }
  bool finished_rain (void) { return finished(ws_RAIN); }
  bool finished_tsys (void) { return finished(ws_TSYS); }
}; // class loop_counter_outside;

class loop_counter_inside : public loop_counter<ws_INSIDE_MAX> {
public:
  loop_counter_inside(void) {
    m_title[ws_INSIDE_TEMP] = F("temp");
  }
  void count_temp (bool res) { increment(ws_INSIDE_TEMP, res); }
  bool finished_temp (void) { return finished(ws_INSIDE_TEMP); }

}; // class loop_counter_inside;

/* functions begin ----------------------------------------------- */

void label_done_wind (void);
void label_done_temp (void);
void label_done_light(void);
void label_done_rain (void);

/* functions end ----------------------------------------------- */

#endif
// eee eof
