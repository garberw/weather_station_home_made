// see README_bme688.txt
#include <Adafruit_BME680.h>
#include <Adafruit_EEPROM_I2C.h>
#include <Adafruit_FRAM_I2C.h>
#include <bsec.h>
#include <weather_lib_util.h>
#include <weather_lib_data.h>
#include <weather_wra_bme688.h>

// macros begin --------------------------------------------------------------

#define SEALEVELPRESSURE_HPA (1013.25)

// Adafruit defaults
#define BME_SCK  13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS   10

// #define BME_SCK   9
// #define BME_MISO  8
// #define BME_MOSI  7
// #define BME_CS    6

// it takes about an hour to calibrate; worse in pure air;
#define STATE_SAVE_PERIOD	UINT32_C(720 * 60 * 1000) // 360 minutes -  4 times a day
// #define STATE_SAVE_PERIOD	UINT32_C(360 * 60 * 1000) // 360 minutes -  4 times a day
// #define STATE_SAVE_PERIOD	UINT32_C(120 * 60 * 1000) // 120 minutes - 12 times a day

// the default I2C address
#define EEPROM_ADDRESS 0x50

// macros end --------------------------------------------------------------
// globals begin --------------------------------------------------------------

/*
  Configure the BSEC library with information about the sensor
  18v/33v = Voltage at Vdd. 1.8V or 3.3V
  3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
  4d/28d = Operating age of the sensor in days
  generic_18v_3s_4d
  generic_18v_3s_28d
  generic_18v_300s_4d
  generic_18v_300s_28d
  generic_33v_3s_4d
  generic_33v_3s_28d
  generic_33v_300s_4d
  generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
 #include "config/generic_33v_3s_4d/bsec_iaq.txt"
  // #include "config/generic_33v_3s_28d/bsec_iaq.txt"
};

uint8_t bsec_state[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t state_update_counter = 0;

// adafruit part #5046; Temperature Humidity Pressure Gas Sensor BME688;
// I2C ADDRESS 0x77;
// I2C ADDRESS 0x76 when jumper from SDO to GND;

Bsec iaq_sensor;

Adafruit_EEPROM_I2C eeprom_i2c;
//Adafruit_FRAM_I2C eeprom_i2c;

// globals end --------------------------------------------------------------

void eeprom_test(void);
void print_version(void);
bool check_iaq_sensor_status(void);
float altitude(float pressure, float sea_level);
String iaq_text(float iaq_score);
void load_state(void);
void update_state(void);

void eeprom_test(void) {
  FUNC_BEGIN;
  // Serial.begin(115200);
  
  float f = 3.141592;
  uint8_t buffer[4];  // floats are 4 bytes!
  memcpy(buffer, (void *)&f, 4);
  
  ser_println("Writing float to address 0x00");
  eeprom_i2c.write(0x00, buffer, 4);

  eeprom_i2c.read(0x00, buffer, 4);
  memcpy((void *)&f, buffer, 4);
  ser_print("Read back float value: ");
  ser_println(f, 8);
}

void print_version(void) {
  FUNC_BEGIN;
  ser_println(F("BSEC library version iaq_sensor.version.xxx"));
  ser_print(F("major        = ")); ser_println(iaq_sensor.version.major);
  ser_print(F("minor        = ")); ser_println(iaq_sensor.version.minor);
  ser_print(F("major_bugfix = ")); ser_println(iaq_sensor.version.major_bugfix);
  ser_print(F("minor_bugfix = ")); ser_println(iaq_sensor.version.minor_bugfix);
}

// require Serial.begin() called first; then recommend delay(1000) or more;
void weather_bme688::setup(void) {
  FUNC_BEGIN;
  ser_println(F("weather_bme688::setup() begin"));
  // we are using goto label so variable declarations have to come first in order to compile;
  bool res = false;
  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  watchdog_reset();
  delay(1500);
  watchdog_reset();

  ser_println(F("initializing eeprom_i2c"));
  Serial.flush();
  res = eeprom_i2c.begin(EEPROM_ADDRESS);
  ERROR_CONDITION((res), F("eeprom_i2c not identified; Check your connections;"));

  ser_println(F("eeprom_i2c found; success;"));

  ser_println(F("BME688 async test"));
  Serial.flush();

  // I2C
  ser_println(F("iaq_sensor.begin()"));
  Serial.flush();
  iaq_sensor.begin(BME68X_I2C_ADDR_HIGH, Wire);
  watchdog_reset();
  print_version();
  res = check_iaq_sensor_status();
  ERROR_CONDITION((res), F("Could not find a valid BME680 sensor, check wiring!"));

  ser_println(F("Found a sensor"));
  Serial.flush();

  ser_println(F("set config"));
  Serial.flush();
  iaq_sensor.setConfig(bsec_config_iaq);
  res = check_iaq_sensor_status();
  ERROR_CONDITION((res), F("Could not set config"));

  ser_println(F("load_state"));

  watchdog_reset();
  load_state();
  watchdog_reset();
  
  ser_println(F("updateSubscription"));
  iaq_sensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  res = check_iaq_sensor_status();
  ERROR_CONDITION((res), F("Could not updateSubscription"));
  
  // bme68x_dev iaq_sensor._bme68x
  // is private so we can not set these; that is probably on purpose;
  // iaq_sensor._bme68x.setTemperatureOversampling(BME680_OS_8X);
  // iaq_sensor._bme68x.setHumidityOversampling(BME680_OS_2X);
  // iaq_sensor._bme68x.setPressureOversampling(BME680_OS_4X);
  // iaq_sensor._bme68x.setIIRFilterSize(BME680_FILTER_SIZE_3);
  // iaq_sensor._bme68x.setGasHeater(320, 150);   // 320 deg C for 150 msec

  ser_println(F("weather_bme688::setup() end pass"));
  Serial.flush();
  // weather_halt();
  // alternatively
  // delay(120000);
  return;
 catch_block:
  ser_println(F("weather_bme688::setup() end fail"));
  wexception.print();
  // fixme
  Serial.flush();
  weather_halt();
}

void weather_bme688::copy_to_wdat(weather_data &wdat) {
  // bug in BSEC example; this pressure is Pa not hPa
  // fixme non-floats
  bool convert = true;
  float alt = altitude(iaq_sensor.pressure, SEALEVELPRESSURE_HPA);
  wdat.set(WDAT_temperature_in     , iaq_sensor.temperature           , convert);
  wdat.set(WDAT_humidity_in        , iaq_sensor.humidity              , convert);
  wdat.set(WDAT_altitude           , alt                              , convert);
  wdat.set(WDAT_pressure           , iaq_sensor.pressure              , convert);  
  wdat.set(WDAT_gas_resistance     , iaq_sensor.gasResistance         , convert);
  wdat.set(WDAT_iaq_score          , iaq_sensor.iaq                   , convert);
  wdat.set(WDAT_co2_equiv          , iaq_sensor.co2Equivalent         , convert);
  wdat.set_iaq_text(iaq_text(iaq_sensor.iaq));

  wdat.set(WDAT_raw_temperature    , iaq_sensor.rawTemperature        , convert);
  wdat.set(WDAT_raw_humidity       , iaq_sensor.rawHumidity           , convert);
  wdat.set(WDAT_static_iaq         , iaq_sensor.staticIaq             , convert);
  wdat.set(WDAT_breath_voc_equiv   , iaq_sensor.breathVocEquivalent   , convert);
  wdat.set(WDAT_comp_gas_value     , iaq_sensor.compGasValue          , convert);
  wdat.set(WDAT_gas_percentage     , iaq_sensor.gasPercentage         , convert);
  
  wdat.set(WDAT_stab_status        , iaq_sensor.stabStatus            , convert);
  wdat.set(WDAT_run_in_status      , iaq_sensor.runInStatus           , convert);
  
  wdat.set(WDAT_acc_iaq            , iaq_sensor.iaqAccuracy           , convert);
  wdat.set(WDAT_acc_static_iaq     , iaq_sensor.staticIaqAccuracy     , convert);
  wdat.set(WDAT_acc_co2            , iaq_sensor.co2Accuracy           , convert);
  wdat.set(WDAT_acc_breath_voc     , iaq_sensor.breathVocAccuracy     , convert);
  wdat.set(WDAT_acc_comp_gas       , iaq_sensor.compGasAccuracy       , convert);
  wdat.set(WDAT_acc_gas_percentage , iaq_sensor.gasPercentageAccuracy , convert);

  wdat.set(WDAT_time_trigger     , millis()                           , convert);
  wdat.set(WDAT_output_timestamp        , iaq_sensor.outputTimestamp  , convert);
}

// fixme read timeout
int weather_bme688::advanced_read(weather_data &wdat) {
  FUNC_BEGIN;
  bool res = false;
  int64_t meas_end_ms = iaq_sensor.nextCall;       // measurement end;
  int64_t call_time_ms = iaq_sensor.getTimeMs();   // now;
  int64_t rem_time_ms = meas_end_ms - call_time_ms; // time left

  if (rem_time_ms > 0) {

    ser_print(F("delay rem_time_ms = ")); ser_println(rem_time_ms);
    Serial.flush();

    watchdog_reset();
    delay(static_cast<unsigned int>(rem_time_ms * 1.1));
    watchdog_reset();
  }
  // true if new data is available;
  // this should be true since we delayed for rem_time_ms until iaq_sensor.nextCall;
  res = iaq_sensor.run();  
  if (!res) {
    check_iaq_sensor_status();
  }
  ERROR_CONDITION((res), F("run too early"));

  copy_to_wdat(wdat);

  ser_println(F("update_state"));

  update_state();

  // wdat.print_inside_all();

  // fixme
  // Serial.flush();
  // delay(2000);
  return 0;
 catch_block:
  wexception.print();
  // fixme
  Serial.flush();
  // weather_halt();
  return -1;
}

/*
  Calculates the altitude (in meters).
  Reads the current atmostpheric pressure (in hPa) from the sensor and
  calculates via the provided sea-level pressure (in hPa).
  seaLevel     Sea-level pressure in hPa
  return       Altitude in meters

  Equation taken from BMP180 datasheet (page 16):
  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  Note that using the equation from wikipedia can give bad results
  at high altitude. See this thread for more information:
  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
*/
float altitude(float pressure, float sea_level) {
  FUNC_BEGIN;
  float atmospheric = pressure / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / sea_level, 0.1903));
}

// update m_iaq_score
// update m_iaq_text
String iaq_text(float iaq_score) {
  FUNC_BEGIN;
  if (iaq_score > 350.0) return "air quality is Extremely polluted; identify contamination; leave";
  if (iaq_score > 250.0) return "air quality is Severely polluted; identify contamination; leave";
  if (iaq_score > 200.0) return "air quality is Heavily polluted; maximum ventilation";
  if (iaq_score > 150.0) return "air quality is Moderately polluted; irritation possible; more ventilation";
  if (iaq_score > 100.0) return "air quality is Lightly polluted; problems possible; ventilate";
  if (iaq_score >  50.0) return "air quality is Good; no irritation";
  if (iaq_score >=  0.0) return "air quality is Excellent; pure;";
  return "air quality is error negative";
}

// Helper function definitions
bool check_iaq_sensor_status(void) {
  FUNC_BEGIN;
  if (iaq_sensor.bsecStatus != BSEC_OK) {
    if (iaq_sensor.bsecStatus < BSEC_OK) {
      ser_print(F("BSEC error code : ")); ser_println(iaq_sensor.bsecStatus);
      Serial.flush();
      weather_halt();
    } else {
      ser_print(F("BSEC warning code : ")); ser_println(iaq_sensor.bsecStatus);
    }
  }

  if (iaq_sensor.bme68xStatus != BME68X_OK) {
    if (iaq_sensor.bme68xStatus < BME68X_OK) {
      ser_print(F("BME68X error code : ")); ser_println(iaq_sensor.bme68xStatus);
      Serial.flush();
      weather_halt();
    } else {
      ser_print(F("BME68X warning code : ")); ser_println(iaq_sensor.bme68xStatus);
    }
  }

  if ((iaq_sensor.bsecStatus == BSEC_OK) && (iaq_sensor.bme68xStatus == BME68X_OK)) {
    return true;
  }
  return false;
}

void load_state(void) {
  FUNC_BEGIN;
  if (eeprom_i2c.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in eeprom_i2c
    ser_println("Reading state from eeprom_i2c ------------>>>>>>>>>>");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
      bsec_state[i] = eeprom_i2c.read(i + 1);
      ser_print(F("0x")); print_hex_8(bsec_state[i]); ser_print(F(", "));
    }
    ser_println();

    iaq_sensor.setState(bsec_state);
    check_iaq_sensor_status();
  } else {
    // Erase the eeprom_i2c with zeroes
    ser_println("Erasing eeprom_i2c");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
      eeprom_i2c.write(i, 0);

    // our eeprom does not have or require this;
    // eeprom_i2c.commit();
  }
}

void update_state(void) {
  FUNC_BEGIN;
  bool update = false;
  if (state_update_counter == 0) {
    // First state update when IAQ accuracy is >= 3 
    if (iaq_sensor.iaqAccuracy >= 3) {
      update = true;
      state_update_counter++;
    }
  } else {
    // Update every STATE_SAVE_PERIOD minutes 
    if ((state_update_counter * STATE_SAVE_PERIOD) < millis()) {
      update = true;
      state_update_counter++;
    }
  }

  if (update) {
    iaq_sensor.getState(bsec_state);
    check_iaq_sensor_status();

    ser_println("Writing state to eeprom_i2c");

    for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE ; i++) {
      eeprom_i2c.write(i + 1, bsec_state[i]);
      ser_print(F("0x")); print_hex_8(bsec_state[i]); ser_print(F(", "));
    }
    ser_println();

    eeprom_i2c.write(0, BSEC_MAX_STATE_BLOB_SIZE);

    // our eeprom does not have or require this;
    // eeprom_i2c.commit();
  }
}

// eee eof
