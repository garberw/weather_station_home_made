// see header for license

// real time clock RTC DS3231;
// adafruit part #5188;
// I2C ADDRESS 0x68;
#include <RTClib.h> 
#include <weather_lib_util.h>
#include <weather_lib_rtc.h>

RTC_DS3231 clock_rtc;

uint32_t time_with_sync::unix_epoch(void) {
  FUNC_BEGIN;
  DateTime t = clock_rtc.now();
  return t.unixtime();
}

void time_with_sync::calc_timeout(void) {
  FUNC_BEGIN;
  // fixme these are minimums; try slightly larger;
  // unix epoch is about 9 digits
  static const int bits_per_packet = 10;
  float char_per_sec = ((float) SERIAL_PRINT_BAUD) / ((float) bits_per_packet);
  float T_usec = 1.0e6 / char_per_sec;
  m_timeout_T15_usec = ((uint32_t) (1.5 * T_usec));
  m_timeout_T35_usec = ((uint32_t) (3.5 * T_usec));
}

// require Serial set up;
// require wexception.clear_all();
int time_with_sync::setup(void) {
  FUNC_BEGIN;
  ser_println(F("starting time_with_sync::setup()"));
  calc_timeout();
  bool res = clock_rtc.begin();
  ERROR_CONDITION((res), F("time_with_sync error; fail to init clock"));
  ser_println(F("time_with_sync::setup() init rtc clock success"));
  // m_last_set = unix_epoch();
  m_last_set = SECONDS_FROM_1970_TO_2000;
  // this would cause halt if time_server not started when atlas wpa plugged in
  // sync();
  return 0;
 catch_block:
  // error echoed by driver
  wexception.print();
  Serial.flush();
  weather_halt();
  return 1;
}

// if (loopc.finished() && !loopc.done())
// i.e. it failed more than allowed number of attempts
// reset timeout
void time_with_sync::reset_timeout(void) {
  int32_t t0;
  ser_println(F("====================================="));
  ser_println(F("time_with_sync::sync() reset (give up)"));
  t0 = ((int32_t) unix_epoch());
  m_last_set = t0;
  ser_println(F("====================================="));
}

// atlas weewx driver acts as time server;
int time_with_sync::sync(void) {
  FUNC_BEGIN;
  int32_t t0, ts, m1, m2, gap;
  String bufa, bufs;
  DateTime TS;
  bool bad_last = false;
  int nline;
  ser_println(F("====================================="));
  ser_println(F("time_with_sync::sync() start"));
  t0 = ((int32_t) unix_epoch());
  if (!(t0 > m_last_set)) {
    ser_print(F("time_with_sync::sync(); error t0 < m_last_set"));
    bad_last = true;
  }
  // sync every RTC_PERIOD_RESET sec
  gap = (t0 - m_last_set);
  ser_print(F("time_with_sync::sync() gap since last set sec = "));
  ser_println(gap);
  ser_print(F("time_with_sync::sync() RTC_PERIOD_RESET       = "));
  ser_println(RTC_PERIOD_RESET);
  if ((gap < RTC_PERIOD_RESET) && !bad_last) {
    ser_println(F("time_with_sync::sync() (gap < RTC_PERIOD_RESET); SKIP RESET;"));
    return 0;
  }
  ser_println(F("time_with_sync::sync() attempting write(\"request_datetime\");"));
  Serial.write("request_datetime\n");
  ser_println(F("time_with_sync::sync() attempting read multiple input; drop leading times"));
  // should not do any output between first and final read();   fixme check;
  nline = 0;
  // fixme should be msec not usec
  //  Serial.setTimeout(m_timeout_T35_usec / 1000);
  bufa = String("");
  bufs = String("");
  do {
    bufs = Serial.readStringUntil('\n');
    bufa = bufa + "(" + String(nline) + ")" + bufs;
    if (Serial.peek() == '\n') {
      Serial.read();
    }
    nline++;
  } while (Serial.available());
  Serial.setTimeout(RTC_SERIAL_DEFAULT_TIMEOUT);
  ser_print(F("time_with_sync::sync() read bufs = {"));
  ser_print(bufs);
  ser_println(F("}"));
  ser_print(F("time_with_sync::sync() read bufa = {"));
  ser_print(bufa);
  ser_println(F("}"));
  ser_print(F("time_with_sync::sync() nline     = "));
  ser_println(nline);
  // parse input
  // uint32_t ts = unix_epoch from server (GMT timezone);
  ERROR_CONDITION((nline != 0), F("empty timestamp; check time server running;"));
  ERROR_CONDITION((bufs.length() == 10), F("wrong number of digits; read or parse error;"));
  ts = bufs.toInt();
  ERROR_CONDITION((ts != 0), F("timestamp not integer; read or parse error;"));
  // adjust only accepts DateTime
  TS = DateTime(ts);
  m1 = millis();
  // adjust sets clock;
  clock_rtc.adjust(TS);
  m2 = millis();
  // output echoed by driver
  ser_print(F("time_with_sync::sync() set unix_epoch ts= "));
  ser_print(ts);
  ser_print(F(" drift= "));
  ser_print(m2 - m1);
  ser_print(F(" msec"));
  ser_println(F(" success -------------------------"));
  m_last_set = ts;
  return 0;
 catch_block:
  // error echoed by driver
  ser_println(F("time_with_sync::sync() fail \\\\\\\\\\\\\\\\\\;"));
  wexception.print();
  return 1;
}

// eee eof
