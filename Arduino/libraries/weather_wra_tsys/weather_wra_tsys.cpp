// Adafruit aht20;  tsys case_temp case_humid;
// TemperatureZero; tsys cpu_temp;

#include <Adafruit_AHTX0.h>
#include <TemperatureZero.h>
#include <weather_lib_util.h>
#include <weather_wra_tsys.h>

/* globals begin ----------------------------------------------- */

Adafruit_AHTX0 aht_sensor;
Adafruit_Sensor *aht_temp;
Adafruit_Sensor *aht_humid;
TemperatureZero t0_cpu_temp = TemperatureZero();

/* globals end ----------------------------------------------- */

// common to modbus client and all modbus servers;
void weather_tsys::setup(void) {
  FUNC_BEGIN;
  bool res = false;
  ser_println(F("Adafruit AHT10/AHT20 test!"));
  res = aht_sensor.begin();
  ERROR_CONDITION((res), F("Failed to find AHT10/AHT20 chip"));
  ser_println(F("AHT10/AHT20 Found!"));
  aht_temp  = aht_sensor.getTemperatureSensor();
  aht_temp->printSensorDetails();
  aht_humid = aht_sensor.getHumiditySensor();
  aht_humid->printSensorDetails();
  // TemperatureZero library for Adafruit itsy bitsy ATSAMD51 cpu;
  t0_cpu_temp.init();
  return;
 catch_block:
  wexception.print();
  weather_halt();
}

// common to modbus client and all modbus servers;
void weather_tsys::advanced_read(float &case_temp,
                                 float &case_humid,
                                 float &cpu_temp)
{
  FUNC_BEGIN;
  // Adafruit aht20; Get a new normalized sensor event;
  sensors_event_t event_temp;
  sensors_event_t event_humid;
  aht_temp->getEvent(&event_temp);
  aht_humid->getEvent(&event_humid);
  case_temp  = event_temp.temperature;
  case_humid = event_humid.relative_humidity;
  // TemperatureZero library for Adafruit itsy bitsy ATSAMD51 cpu;
  cpu_temp = t0_cpu_temp.readInternalTemperature();
}

void weather_tsys::advanced_read_client(weather_data &wdat) {
  FUNC_BEGIN;
  float case_temp;
  float case_humid;
  float cpu_temp;
  bool convert = true;
  advanced_read(case_temp        , case_humid, cpu_temp);
  wdat.set(WDAT_client_case_temp , case_temp , convert);
  wdat.set(WDAT_client_case_humid, case_humid, convert);
  wdat.set(WDAT_client_cpu_temp  , cpu_temp  , convert);
}

void weather_tsys::advanced_read_light(weather_data &wdat) {
  FUNC_BEGIN;
  float case_temp;
  float case_humid;
  float cpu_temp;
  bool convert = true;
  advanced_read(case_temp       , case_humid, cpu_temp);
  wdat.set(WDAT_light_case_temp , case_temp,  convert);
  wdat.set(WDAT_light_case_humid, case_humid, convert);
  wdat.set(WDAT_light_cpu_temp  , cpu_temp ,  convert);
}

void weather_tsys::advanced_read_rain(weather_data &wdat) {
  FUNC_BEGIN;
  float case_temp;
  float case_humid;
  float cpu_temp;
  bool convert = true;
  advanced_read(case_temp      , case_humid, cpu_temp);
  wdat.set(WDAT_rain_case_temp , case_temp , convert);
  wdat.set(WDAT_rain_case_humid, case_humid, convert);
  wdat.set(WDAT_rain_cpu_temp  , cpu_temp  , convert);
}

// eee eof
