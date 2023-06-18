#ifndef weather_lib_rtc_h
#define weather_lib_rtc_h

// fixme
// #include <Wire.h>    // for I2C communication

class time_with_sync {
public:
  // times in msec;
  // (2023 - 1970) * seconds/year < 1.7e9 (decimal) < 2^31 so it should fit in an int
  // const int32_t RTC_PERIOD_RESET = 0;               // client reset each loop;
  // const int32_t RTC_PERIOD_RESET = 10;              // client reset each 20 sec;
  // const int32_t RTC_PERIOD_RESET = 3600 * 6;        // client reset every 6 hours;
  // const int32_t RTC_PERIOD_RESET = 60 * 60 * 1;     // client reset every 1 hours;
  // const int32_t RTC_PERIOD_RESET = 60 * 20;         // client reset every 20 min;
  // const int32_t RTC_PERIOD_RESET = 60 * 2;          // client reset every 2 min; 
  const int32_t RTC_PERIOD_RESET = 60 * 60 * 24;    // client reset every 2 min; 
  // const int32_t RTC_PERIOD_RESET = 60 * 60 * 1;    // client reset every  1 hours;
  static const int     RTC_SERIAL_DEFAULT_TIMEOUT = 1000; // msec
  void calc_timeout(void);
  int setup(void);
  void reset_timeout(void);
  int sync(void);
  uint32_t unix_epoch(void);
  int32_t m_last_set;
  uint32_t m_timeout_T15_usec;
  uint32_t m_timeout_T35_usec;
};

#endif
// eee eof
