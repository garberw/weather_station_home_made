// see README_pas_c02.txt
#include <Arduino.h>
#include <pas-co2-ino.hpp>

#include <weather_lib_util.h>
#include <weather_lib_data.h>
#include <weather_wra_pas_co2.h>

// macros begin --------------------------------------------------------------

#define SEALEVELPRESSURE_HPA (1013.25)

#define SAMPLE_PERIOD_MIN  UINT32_C(60 * 1000) // 60 sec

/* 
   The sensor supports 100KHz and 400KHz. 
   You hardware setup and pull-ups value will
   also influence the i2c operation. You can 
   change this value to 100000 in case of 
   communication issues.
*/
#define I2C_FREQ_HZ 400000  

// macros end --------------------------------------------------------------
// globals begin --------------------------------------------------------------

/*
  Create CO2 object. Unless otherwise specified,
  using the Wire interface
*/
PASCO2Ino cotwo;
Error_t err;

// globals end --------------------------------------------------------------

// require setup_communication_ser_0 called; recommended delay at least 500 msec;
void weather_pas_co2::setup(void)
{
  FUNC_BEGIN;
  int res;
  /*
  Serial.begin(9600);
  Serial.println("serial initialized");
  */
  ser_println(F("weather_wra_pas_co2::setup() begin"));
  watchdog_reset();
  delay(500);
  watchdog_reset();
  
  /* Initialize the i2c interface used by the sensor */
  // fixme called twice
  Wire.begin();
  Wire.setClock(I2C_FREQ_HZ);

  /* Initialize the sensor */
  err = cotwo.begin();
  ERROR_CONDITION((err == XENSIV_PASCO2_OK), F("cotwo initialization error: "));
  ser_println(F("Found a sensor"));

  res = start_next_reading();
  ERROR_RETURN((!res), F("start_next_reading"));

  // minimum wait is 5000 msec
  for (int jjj = 0; jjj < 10; jjj++) {
    watchdog_reset();
    delay(500);
  }
  res = wait_for_reading();
  ERROR_RETURN((!res), F("wait_for_reading"));

  res = start_next_reading();
  ERROR_RETURN((!res), F("start_next_reading"));
  
  ser_println(F("weather_wra_pas_co2::setup() end pass"));
  watchdog_reset();
  return;
 catch_block:
  ser_println(F("weather_wra_pas_co2::setup() end fail"));
  Serial.println(err);
  wexception.print();
}

int weather_pas_co2::start_next_reading(void) {
  FUNC_BEGIN;
  // trigger a one shot measurement
  ser_println("watchdog_reset()");
  watchdog_reset();
  ser_println("cotwo.startMeasure()");
  err = cotwo.startMeasure();
  ser_println("watchdog_reset()");
  watchdog_reset();
  ERROR_CONDITION((err == XENSIV_PASCO2_OK), F("startMeasure"));
  ser_println(F("startMeasure passed"));
  return 0;
 catch_block:
  Serial.println(err);
  wexception.print();
  return 1;
}

int weather_pas_co2::wait_for_reading(void) {
  FUNC_BEGIN;
  // wait for the value to be ready
  // minimum measurement time is 5000 msec;
  // fixme assume watchdog is more than 500 msec;
  /*
    getCO2() is called until the value is 
    available.  
    getCO2() returns 0 when no measurement 
    result is yet available or an error has
    occurred.
  */
  do {
    ser_println("watchdog_reset()");
    watchdog_reset();
    // ser_println("delay(500)");
    // delay(500);
    ser_println("watchdog_reset()");
    watchdog_reset();
    ser_println("cotwo.getCO2()");
    err = cotwo.getCO2(m_last_co2);
    ser_println("watchdog_reset()");
    watchdog_reset();
    ERROR_CONDITION((err == XENSIV_PASCO2_OK), F("waiting for first measurement"));
  } while (m_last_co2 == 0);

  m_last_time_msec = millis();

  ser_println("wait_for_reading passed -------------------------------------");
  ser_print("m_last_co2 = "); ser_println(m_last_co2);
  Serial.flush();
  return 0;
 catch_block:
  Serial.println("wait_for_reading failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  Serial.println(err);
  wexception.print();
  Serial.flush();
  return 1;
}

void weather_pas_co2::copy_to_wdat(weather_data &wdat) {
  FUNC_BEGIN;
  // bug in BSEC example; this pressure is Pa not hPa
  // fixme non-floats
  bool convert = true;
  float co2_equiv = ((float) m_last_co2);
  wdat.set(WDAT_co2_equiv           , co2_equiv                        , convert);
  // wdat.set(WDAT_iaq_score          , iaq_sensor.iaq                   , convert);
  // wdat.set_iaq_text(iaq_text(iaq_sensor.iaq));
}

// require call iaq.advanced_read() first to get pressure in wdat;
int weather_pas_co2::advanced_read(weather_data &wdat) {
  FUNC_BEGIN;
  int res1;
  int res2;
  int32_t call_time_msec = millis();
  int32_t gap_time_msec = call_time_msec - m_last_time_msec;

  ser_print("m_last_time_msec = "); ser_println(m_last_time_msec);
  ser_print("call_time_msec   = "); ser_println(call_time_msec);
  ser_print("gap_time_msec    = "); ser_println(gap_time_msec);
  // counter overflowed
  if (gap_time_msec < 0) {
    m_last_time_msec = 0;
    gap_time_msec = call_time_msec;
    // fall through
  }
  if (gap_time_msec > SAMPLE_PERIOD_MIN) {
    ser_println("...................................... attempt co2 reading");
    res1 = wait_for_reading();

    if (res1) Serial.println(err);
    
    res2 = start_next_reading();

    if (res2) Serial.println(err);
    
    ERROR_RETURN((!res1), F("wait_for_reading"));
    ERROR_RETURN((!res2), F("start_next_reading"));

  }
  // in Hg;
  float pressure;
  wdat.get(WDAT_pressure, pressure);
  // hPa; about 900.0;
  pressure *= CONVERT_IN_HG_TO_PA * 0.01;
  // fixme
  /*
  cotwo.setPressRef(pressure);
  */
  copy_to_wdat(wdat);

  return 0;
 catch_block:
  wexception.print();
  ser_println("attempt co2 reading failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  ser_print("res1 = "); ser_println(res1);
  ser_print("res2 = "); ser_println(res2);
  Serial.println(err);
  wexception.print();
  // fixme
  Serial.flush();
  // weather_halt();
  return -1;
}


// eee eof
