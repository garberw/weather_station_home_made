/*
  misol rain gauge driver
  It reads in inches:
  rain_daily
  rain_hourly
  rain_daily_until_last_hour
  and outputs them to the serial monitor.
*/
/*
  // timestamp in defunct print_rain();
  #include <RTClib.h>
  RTC_Millis rtc;
  rtc.begin(DateTime(__DATE__, __TIME__));
  DateTime now = rtc.now();
  ser_print(now.hour());
  ser_print(F(":"));
  ser_print(now.minute());
  ser_print(F(" since last change "));
*/

#include <Wire.h>
#include <weather_lib_util.h>
#include <weather_wra_misol.h>

/* macros begin ----------------------------------------------- */

// const int pin_RAIN = 2;
const int pin_RAIN = 9;

const int BUCKET_TIPS_RESET = 10000;    // must fit in uint16_t
const int CHECK_TIME_SEC = 7;
// const uint32_t DEBOUNCE_DELAY_MSEC = 50;   // fixme check msec
const uint32_t DEBOUNCE_DELAY_MSEC = 300;   // fixme check msec

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

volatile uint32_t weather_misol::m_last_debounce_time_msec;
volatile uint16_t weather_misol::m_rain_bucket_prev;
volatile uint16_t weather_misol::m_rain_bucket_tips;
volatile uint16_t weather_misol::m_rain_reset;

/* globals end ----------------------------------------------- */

// reading millis() once is OK, you get the value that was set when the interrupt occurs.
// But as millis() requires interrupts as well to get updated,
// you can't rely on millis() changing whilst executing the ISR.

// interrupt service routine (ISR) 
// assigned to the interrupt for pin_RAIN by rain_setup_pins()
void weather_misol::count_bucket_tip_ISR(void) {
  FUNC_BEGIN;
  uint32_t time_msec = millis();
  // fixme if time wraps around (overflows) uint32_t  
  // it should still work
  if (time_msec - m_last_debounce_time_msec > DEBOUNCE_DELAY_MSEC) {
    m_rain_bucket_tips++;
    m_last_debounce_time_msec = time_msec;
    if (m_rain_bucket_tips >= BUCKET_TIPS_RESET) {
      // set m_rain_reset to false after reporting to modbus client
      m_rain_reset = true;
      m_rain_bucket_tips = 0;
      m_rain_bucket_prev = 0;
    }
  }
}

void weather_misol::setup_pins(void) {
  FUNC_BEGIN;
  // pinMode(pin_RAIN, INPUT);
  pinMode(pin_RAIN, INPUT_PULLUP);
  m_rain_bucket_tips = 0;
  m_rain_bucket_prev = 0;
  m_rain_reset = true;
  int interrupt = digitalPinToInterrupt(pin_RAIN);
  attachInterrupt(interrupt, count_bucket_tip_ISR, CHANGE);
}

void weather_misol::setup_data(void) {
  FUNC_BEGIN;
  m_last_debounce_time_msec = 0;
  m_rain_bucket_prev = 0;
  m_rain_bucket_tips = 0;
  m_rain_reset = true;
}

void weather_misol::copy_to_wdat(weather_data &wdat) {
  // wdat.set() converts units before storing in wdat;
  // total = tips with unit conversion to inches;
  // rest are stored as integers;
  // change is only defined now;
  float tips   = static_cast<float>(m_rain_bucket_tips);
  float change = static_cast<float>(m_rain_bucket_tips - m_rain_bucket_prev);
  float prev   = static_cast<float>(m_rain_bucket_tips);
  float reset  = static_cast<float>(m_rain_reset);
  bool convert = true;
  wdat.set(WDAT_rain_bucket_tips  , tips  , convert);
  wdat.set(WDAT_rain_bucket_change, change, convert);
  wdat.set(WDAT_rain_bucket_prev  , prev  , convert);
  wdat.set(WDAT_rain_total        , tips  , convert);
  wdat.set(WDAT_rain_reset        , reset , convert);
}

void weather_misol::advanced_read(weather_data &wdat) {
  // want to spend as much time checking for rain as possible;
  // do not miss bucket tip;
  // only print on update; only save on update;
  if (!new_tips() && !m_rain_reset) return;
  // update occurs;
  // only print on update; fixme was in process_upstream_sensor;
  divider_line(F("="), 30); ser_println(F(" advanced_read_rain begin"));
  if (m_rain_reset) {
    ser_println(F("rain reset"));
  } else {
    ser_println(F("new tips"));
  }
  // m_rain_bucket_change is only defined and used in copy_to_wdat;
  // call this before rest of m_rain_bucket_prev !!!!;
  copy_to_wdat(wdat);

  wdat.print_outside_sensor(ws_RAIN);

  // start a new measurement; reset m_rain_bucket_prev;
  m_rain_bucket_prev = m_rain_bucket_tips;
}

// eee eof
