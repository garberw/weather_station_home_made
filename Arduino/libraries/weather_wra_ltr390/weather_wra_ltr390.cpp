// This library is a driver for the Adafruit LTR390 UV light sensor;
// also measures luminosity and infrared (IR);

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LTR390.h>
#include <weather_wra_ltr390.h>

/* macros begin ----------------------------------------------- */

#define UVI_FACTOR1 (18.0 / 2300.0)    // uvi corrected for filter (c.f.f.)
#define UVI_RES     (20)               // uvs mode default
//#define UVI_GAIN    (18.0)             // uvs mode default
#define UVI_GAIN    ( 1.0)             // uvs mode default
#define UVI_FACTOR  ((UVI_FACTOR1)*(1 << (20 - UVI_RES))/(UVI_GAIN))  // uvi (c.f.f.)

#define LUX_FACTOR1 (1.266922833)      // (W/m2) * msec  (c.f.f.)
#define LUX_GAIN    (3.0)              // als mode default
#define LUX_TIME    (16.0)             // als mode default
#define LUX_FACTOR  ((LUX_FACTOR1)/((LUX_GAIN)*(LUX_TIME))) // (W/m2) (c.f.f.)

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

Adafruit_LTR390 sensor_ltr;

/* globals end ----------------------------------------------- */

bool weather_ltr390::new_data_available(void) {
  return sensor_ltr.newDataAvailable();
}

byte weather_ltr390::gain_factor(void) {
  FUNC_BEGIN;
  switch (sensor_ltr.getGain()) {
  case LTR390_GAIN_1: return 1; break;
  case LTR390_GAIN_3: return 3; break;
  case LTR390_GAIN_6: return 6; break;
  case LTR390_GAIN_9: return 9; break;
  case LTR390_GAIN_18: return 18; break;
  default:
    ERROR_CONDITION(false, F("gain_factor"));
  }
  // fixme
  return 1;
 catch_block:
  // fixme
  return 1; 
}

byte weather_ltr390::resolution_factor(void) {
  FUNC_BEGIN;
  switch (sensor_ltr.getResolution()) {
  case LTR390_RESOLUTION_13BIT: return 13; break;
  case LTR390_RESOLUTION_16BIT: return 16; break;
  case LTR390_RESOLUTION_17BIT: return 17; break;
  case LTR390_RESOLUTION_18BIT: return 18; break;
  case LTR390_RESOLUTION_19BIT: return 19; break;
  case LTR390_RESOLUTION_20BIT: return 20; break;
  default:
    ERROR_CONDITION(false, F("resolution_factor"));
  }
  // fixme
  return 0;
 catch_block:
  return 0;
}

// pow(2.0, #BIT - 18);
// e.g. for 13 bit return pow(2.0, 13 - 18) = 1.0/32.0 = 0.03125;
// e.g. for 17 bit return pow(2.0, 17 - 18) = 0.5;
float weather_ltr390::resolution_integration(void) {
  FUNC_BEGIN;
  switch (sensor_ltr.getResolution()) {
  case LTR390_RESOLUTION_13BIT: return 0.03125; break; // conversion time =  12.5 msec ????
  case LTR390_RESOLUTION_16BIT: return 0.25;    break; // conversion time =  25 msec
  case LTR390_RESOLUTION_17BIT: return 0.5;     break; // conversion time =  50 msec
  case LTR390_RESOLUTION_18BIT: return 1.0;     break; // conversion time = 100 msec
  case LTR390_RESOLUTION_19BIT: return 2.0;     break; // conversion time = 200 msec
  case LTR390_RESOLUTION_20BIT: return 4.0;     break; // conversion time = 400 msec
  default:
    ERROR_CONDITION(false, F("resolution_integration"));
  }
  // fixme
  return 0.0;
 catch_block:
  return 0.0;
}

// default value
// sensor_ltr.setGain(LTR390_GAIN_3);
// sensor_ltr.setResolution(LTR390_RESOLUTION_16BIT);
// slower more accurate value 20 bit integration_time() == 400.0 msec;
void weather_ltr390::set_mode_uvs(void) {
  FUNC_BEGIN;
  sensor_ltr.setMode(LTR390_MODE_UVS);
  // fixme original until 5/28/23
  //  sensor_ltr.setGain(LTR390_GAIN_18);
  sensor_ltr.setGain(LTR390_GAIN_1);
  sensor_ltr.setResolution(LTR390_RESOLUTION_20BIT);
  sensor_ltr.setThresholds(100, 1000);
  sensor_ltr.configInterrupt(true, LTR390_MODE_UVS);
}

// fixme sensible gain
// fixme sensible resolution
// fixme sensible thresholds
void weather_ltr390::set_mode_als(void) {
  FUNC_BEGIN;
  sensor_ltr.setMode(LTR390_MODE_ALS);
  sensor_ltr.setGain(LTR390_GAIN_3);
  sensor_ltr.setResolution(LTR390_RESOLUTION_16BIT);
  sensor_ltr.setThresholds(100, 1000);
  sensor_ltr.configInterrupt(true, LTR390_MODE_ALS);
}

// Display some basic information on this sensor
// require call Serial.begin() first;
void weather_ltr390::setup(void) {
  FUNC_BEGIN;
  ser_println(F("Adafruit LTR-390 test"));
  bool res = sensor_ltr.begin();
  ERROR_CONDITION((res), F("Couldn't find LTR sensor!"));
ser_println(F("Found LTR sensor!"));

  // can check the macro UVI_FACTOR is computed correctly
  UVI_factor();
  // can check the macro LUX_FACTOR is computed correctly
  LUX_factor();
  
  set_mode_uvs();

  if (sensor_ltr.getMode() == LTR390_MODE_ALS) {
    ser_println(F("In ALS mode"));
  } else {
    ser_println(F("In UVS mode"));
  }
  ser_print(F("Gain : "));
  ser_println(gain_factor());
  ser_print(F("Resolution : "));
  ser_println(resolution_factor());
  return;
 catch_block:
  ;
}

// UVI based upon the rated sensitivity of
// 1 UVI per 2300 counts at 18X gain factor and 20-bit resolution.
// min(res) = 13; max(num) = 1 << 7; min(num) = 1;
float weather_ltr390::UVI_factor(void) {
  FUNC_BEGIN;
  set_mode_uvs();
  byte     res    = resolution_factor();
  byte     gain   = gain_factor();
  uint16_t num    = ((float)(1 << (20 - res)));
  
  float    res_f  = ((float) res);
  float    gain_f = ((float) gain);
  float    num_f  = ((float) num);
  
  float    FACTOR = UVI_FACTOR1 * num_f / gain_f;

  ser_print(F("FACTOR = UVI_FACTOR1 * num_f / gain_f;"));
  ser_print(F("FACTOR == UVI_FACTOR macro if correct;"));
  ser_print(F("UVI_FACTOR1      = ")); ser_println(UVI_FACTOR1, 5);
  ser_print(F("res              = ")); ser_println(res);
  ser_print(F("gain             = ")); ser_println(gain);
  ser_print(F("num              = ")); ser_println(num);
  ser_print(F("FACTOR           = ")); ser_println(FACTOR, 5);
  ser_print(F("UVI_FACTOR macro = ")); ser_println(UVI_FACTOR, 5);
  return FACTOR;
}

float weather_ltr390::LUX_factor(void) {
  FUNC_BEGIN;
  set_mode_als();
  float    gain   = ((float) gain_factor());
  float    time   = ((float) integration_time());
  float    FACTOR = LUX_FACTOR1 / ( time * gain );

  ser_print(F("FACTOR = LUX_FACTOR1 / ( time * gain );"));
  ser_print(F("FACTOR = LUX_FACTOR macro if correct;"));
  ser_print(F("LUX_FACTOR1      = ")); ser_println(LUX_FACTOR1, 5);
  ser_print(F("gain             = ")); ser_println(gain, 5);
  ser_print(F("time             = ")); ser_println(time, 5);
  ser_print(F("FACTOR           = ")); ser_println(FACTOR, 5);
  ser_print(F("LUX_FACTOR macro = ")); ser_println(LUX_FACTOR, 5);
  return FACTOR;
}

// Read UV count and return calculated UV Index (UVI) value
float weather_ltr390::UV_index(uint32_t &uvs) {
  FUNC_BEGIN;
  set_mode_uvs();
  // average to squeeze a little more data out of it
  while (!new_data_available()) delay(10);
  uint32_t uvs1 = sensor_ltr.readUVS();
  while (!new_data_available()) delay(10);
  uint32_t uvs2 = sensor_ltr.readUVS();
  uvs = (uvs1 + uvs2) / 2;
  float uvi_f  = UVI_FACTOR * ((float) (uvs1 + uvs2)/2.0);
  return uvi_f;
}

// return total count in uint32_t &light (OUTPUT argument);
float weather_ltr390::lux_linear(uint32_t &light1) {
  FUNC_BEGIN;
  set_mode_als();
  while (!new_data_available()) delay(10);
  light1   = sensor_ltr.readALS();
  float lux2_f   = LUX_FACTOR * ((float) light1);
  return lux2_f;
}

void weather_ltr390::advanced_read(weather_data &wdat) {
  FUNC_BEGIN;
  while (!new_data_available());
  uint32_t uvs;
  float    uvi = UV_index(uvs);
  uint32_t light;
  float    lux2 = lux_linear(light);
  bool convert = true;
  wdat.set(WDAT_light_uv  , uvi , convert);
  wdat.set(WDAT_light_lux2, lux2, convert);

  // fixme
  // non-UV light is not necessarily done yet
  // just print UV light
  // this prints both
  // wdat.print_outside_sensor(ws_LIGHT);

  // raw measurements
  ser_println("raw measurements");
  ser_print(F("UV               = ")); ser_println(uvs);
  ser_print(F("light            = ")); ser_println(light);
}

// eee eof
