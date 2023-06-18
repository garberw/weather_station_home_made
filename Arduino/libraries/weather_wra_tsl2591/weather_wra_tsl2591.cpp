// This library is a driver for the Adafruit TSL2591 digital luminosity and IR sensor.
// Dynamic Range: 600M:1
// Maximum Lux: 88K

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#include <weather_lib_util.h>
#include <weather_wra_tsl2591.h>

/* macros begin ----------------------------------------------- */

// LLF == LUX_LINEAR_FULL
// LLV == LUX_LINEAR_VISIBLE
// LLA == LUX_LINEAR_ADVANCED_READ
#define MY_GAIN             (1.0)           // same as in configure_sensor
#define MY_INTEGRATION_TIME (100.0)         // same as in configure_sensor
#define LLF_FACTOR1         ( 7.808657607)  // (W/m2) * msec corrected for filter (c.f.f.)
#define LLV_FACTOR1         (11.798251352)  // (W/m2) * msec corrected for filter (c.f.f.)
#define LLA_FACTOR1         ( 4.3691367  )  // (W/m2) * msec corrected for filter (c.f.f.)
#define LLF_FACTOR          ((LLF_FACTOR1)/((MY_GAIN)*(MY_INTEGRATION_TIME))) // (W/m2) (c.f.f.)
#define LLV_FACTOR          ((LLV_FACTOR1)/((MY_GAIN)*(MY_INTEGRATION_TIME))) // (W/m2) (c.f.f.)
#define LLA_FACTOR          ((LLA_FACTOR1)/((MY_GAIN)*(MY_INTEGRATION_TIME))) // (W/m2) (c.f.f.)

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

// pass in a number for the sensor identifier (for your use later)
Adafruit_TSL2591 sensor_tsl(2591);

/* globals end ----------------------------------------------- */

void weather_tsl2591::setup(void) {
  FUNC_BEGIN;
  ser_println(F("Light Sensor"));
  ser_println(F("Starting Adafruit TSL2591 Test!"));
  bool res = sensor_tsl.begin();
  if (res) {
    ser_println(F("Found a TSL2591 sensor"));
  } else {
    ERROR_CONDITION(false, F("No sensor found ... check your wiring?"));
  }
  print_details();
  configure_sensor();

  // can check the macro LLF_FACTOR is computed correctly
  lux_linear_full_factor();
  // can check the macro LLV_FACTOR is computed correctly
  lux_linear_visible_factor();
  // can check the macro LLA_FACTOR is computed correctly
  lux_linear_advanced_factor();

  return;
 catch_block:
  ;
}

// Displays some basic information on this sensor from the unified
// sensor API sensor_t type (see Adafruit_Sensor for more information)
void weather_tsl2591::print_details(void) {
  FUNC_BEGIN;
  sensor_t sensor;
  sensor_tsl.getSensor(&sensor);
  divider_line(F("="), 30); ser_println();
  ser_print  (F("Sensor:       ")); ser_println(sensor.name);
  ser_print  (F("Driver Ver:   ")); ser_println(sensor.version);
  ser_print  (F("Unique ID:    ")); ser_println(sensor.sensor_id);
  ser_print  (F("Max Value:    ")); ser_print(sensor.max_value); ser_println(F(" lux"));
  ser_print  (F("Min Value:    ")); ser_print(sensor.min_value); ser_println(F(" lux"));
  ser_print  (F("Resolution:   ")); ser_print(sensor.resolution, 4); ser_println(F(" lux"));  
  divider_line(F("="), 30); ser_println();
}

// Configures the gain and integration time;
// we are going to use a fixed gain and integration time;
// TSL2591_INTEGRATIONTIME_100MS = shortest integration time    (bright light);
// TSL2591_INTEGRATIONTIME_300MS = ( 300 msec )  = DEFAULT
// TSL2591_INTEGRATIONTIME_600MS = longest  integration time    (dim    light);
// You can change the gain on the fly, to adapt to brighter/dimmer light situations;
// Changing the integration time gives you a longer time over which to sense light;
// longer timelines are slower, but are good in very low light situtations;
// our problem is usually that there is broad daylight and it can saturate the sensor
// at high gain;
void weather_tsl2591::configure_sensor(void) {
  FUNC_BEGIN;
  sensor_tsl.setGain(TSL2591_GAIN_LOW);
  // sensor_tsl.setGain(TSL2591_GAIN_MED);
  // sensor_tsl.setGain(TSL2591_GAIN_HIGH);
  sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS); 
  // sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  // sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // sensor_tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
  divider_line(F("="), 30); ser_println();
  print_gain();
  print_timing();
  divider_line(F("="), 30); ser_println();
}

uint16_t weather_tsl2591::gain_factor(void) {
  FUNC_BEGIN;
  tsl2591Gain_t gain = sensor_tsl.getGain();
  switch(gain) {
  case TSL2591_GAIN_LOW:   return    1; break; // (   1x )      = lowest gain  (bright light)
  case TSL2591_GAIN_MED:   return   25; break; // (  25x )      = DEFAULT
  case TSL2591_GAIN_HIGH:  return  428; break; // ( 428x )      = highest gain (dim light)
  case TSL2591_GAIN_MAX:   return 9876; break;
  default:
    ERROR_CONDITION(false, F("gain_factor; unknown gain;"));
    break;
  }
  // fixme
  return 1;
 catch_block:
  // fixme
  return 1;
}

void weather_tsl2591::print_gain(void) {
  FUNC_BEGIN;
  ser_print  (F("Gain:         "));
  tsl2591Gain_t gain = sensor_tsl.getGain();
  switch(gain) {
  case TSL2591_GAIN_LOW:   ser_println(F("   1x (Low   )")); break;
  case TSL2591_GAIN_MED:   ser_println(F("  25x (Medium)")); break;
  case TSL2591_GAIN_HIGH:  ser_println(F(" 428x (High  )")); break;
  case TSL2591_GAIN_MAX:   ser_println(F("9876x (Max   )")); break;
  default:
    ERROR_CONDITION(false, F("print_gain; unknown gain;"));
  }
  return;
 catch_block:
  ;
}

void weather_tsl2591::print_timing(void) {
  FUNC_BEGIN;
  ser_print  (F("Timing:       "));
  ser_print((sensor_tsl.getTiming() + 1) * 100, DEC); 
  ser_println(F(" ms"));
}

float weather_tsl2591::integration_time(void) {
  FUNC_BEGIN;
  tsl2591IntegrationTime_t timing = sensor_tsl.getTiming();
  switch (timing) {
  case TSL2591_INTEGRATIONTIME_100MS:  return 100.0F; break;
  case TSL2591_INTEGRATIONTIME_200MS:  return 200.0F; break;
  case TSL2591_INTEGRATIONTIME_300MS:  return 300.0F; break;
  case TSL2591_INTEGRATIONTIME_400MS:  return 400.0F; break;
  case TSL2591_INTEGRATIONTIME_500MS:  return 500.0F; break;
  case TSL2591_INTEGRATIONTIME_600MS:  return 600.0F; break;
  default:
    ERROR_CONDITION(false, F("error:  integration_time; unknown timing;"));
  }
  // fixme
  return 0.0F;
 catch_block:
  // fixme
  return 0.0F;
}

// factor used in lux_linear_full;
// constant assuming gain and integration time are fixed;
// calculate or replace with macro;
float weather_tsl2591::lux_linear_full_factor(void) {
  float gain = ((float) gain_factor());
  float time = ((float) integration_time());
  float LLFF = LLF_FACTOR1 / (time * gain);

  ser_print(F("LLFF = LLF_FACTOR1 / ( time * gain );"));
  ser_print(F("LLFF = LLF_FACTOR macro if correct;"));
  ser_print(F("LLF_FACTOR1      = ")); ser_println(LLF_FACTOR1, 5);
  ser_print(F("gain             = ")); ser_println(gain, 5);
  ser_print(F("time             = ")); ser_println(time, 5);
  ser_print(F("LLFF             = ")); ser_println(LLFF, 5);
  ser_print(F("LLF_FACTOR macro = ")); ser_println(LLF_FACTOR, 5);
  return LLFF;
}

// factor used in lux_linear_visible;
// constant assuming gain and integration time are fixed;
// calculate or replace with macro;
float weather_tsl2591::lux_linear_visible_factor(void) {
  float gain = ((float) gain_factor());
  float time = ((float) integration_time());
  float LLVF = LLV_FACTOR1 / (time * gain);

  ser_print(F("LLVF = LLV_FACTOR1 / ( time * gain );"));
  ser_print(F("LLVF = LLV_FACTOR macro if correct;"));
  ser_print(F("LLV_FACTOR1      = ")); ser_println(LLV_FACTOR1, 5);
  ser_print(F("gain             = ")); ser_println(gain, 5);
  ser_print(F("time             = ")); ser_println(time, 5);
  ser_print(F("LLVF             = ")); ser_println(LLVF, 5);
  ser_print(F("LLV_FACTOR macro = ")); ser_println(LLV_FACTOR, 5);
  return LLVF;
}

// factor used in lux_linear_advanced;
// constant assuming gain and integration time are fixed;
// calculate or replace with macro;
float weather_tsl2591::lux_linear_advanced_factor(void) {
  float gain = ((float) gain_factor());
  float time = ((float) integration_time());
  float LLAF = LLA_FACTOR1 / (time * gain);

  ser_print(F("LLAF = LLA_FACTOR1 / ( time * gain );"));
  ser_print(F("LLAF = LLA_FACTOR macro if correct;"));
  ser_print(F("LLA_FACTOR1      = ")); ser_println(LLA_FACTOR1, 5);
  ser_print(F("gain             = ")); ser_println(gain, 5);
  ser_print(F("time             = ")); ser_println(time, 5);
  ser_print(F("LLAF             = ")); ser_println(LLAF, 5);
  ser_print(F("LLA_FACTOR macro = ")); ser_println(LLA_FACTOR, 5);
  return LLAF;
}

float weather_tsl2591::lux_linear_full(uint16_t full) {
  float count = ((float) full);
  // float LLFF = lux_linear_full_factor();
  float LLFF = (LLF_FACTOR);
  float llf  = count * LLFF;
  return llf;
}

float weather_tsl2591::lux_linear_visible(uint16_t visible) {
  float count = ((float) visible);
  // float LLVF = lux_linear_visible_factor();
  float LLVF = (LLV_FACTOR);
  float llv  = count * LLVF;
  return llv;
}

void label_light_rho (void) { ser_print(F("light_rho        = ")); }


// Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
// That way you can do whatever math and comparisons you want!
void weather_tsl2591::advanced_read(weather_data &wdat) {
  FUNC_BEGIN;
  uint32_t lum     = sensor_tsl.getFullLuminosity();
  uint16_t full    = lum & 0xFFFF;
  uint16_t ir      = lum >> 16;
  uint16_t visible = full - ir;

  float LUM     = ((float) lum );
  float FULL    = ((float) full);
  float IR      = ((float) ir  );
  float VISIBLE = ((float) visible);
  float RHO     = (1.0 - IR / FULL) * (1.0 - IR / FULL);

  float lux1 = LLA_FACTOR * sensor_tsl.calculateLux(full, ir);
  float lux3 = lux_linear_full(full);
  float lux4 = lux_linear_full(visible);
  float lux5 = lux_linear_full(ir);
  float lux6 = RHO;

  bool convert = true;
  // copy_to_wdat()
  wdat.set(WDAT_light_lux,     lux1, convert);
  wdat.set(WDAT_light_full,    lux3, convert);
  wdat.set(WDAT_light_visible, lux4, convert);
  wdat.set(WDAT_light_ir,      lux5, convert);
  wdat.set(WDAT_light_lux2,    lux6, convert);

  // fixme
  // UV is not necessarily done yet
  // just print tsl2591 light
  // this prints both
  // wdat.print_outside_sensor(ws_LIGHT);
}

// eee eof
