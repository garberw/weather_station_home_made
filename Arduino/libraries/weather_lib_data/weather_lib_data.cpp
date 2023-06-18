// weather sensor data; inside and out;
// see README.example_json_packet.txt for print_inside_all_weewx();

#include <weather_lib_util.h>
#include <weather_lib_data.h>
#include <weather_wra_misol.h>
#include <weather_wra_tsys.h>

/* macros begin ----------------------------------------------- */

// fixme other conversion factors
#define Acf CONVERT_C_TO_F
#define Bcf (32.0)
#define Cve CONVERT_MPS_TO_MPH
#define Cra CONVERT_RAIN
#define Cpi CONVERT_PA_TO_IN_HG
#define Cok CONVERT_OHM_TO_KOHM
#define Cmf CONVERT_M_TO_FT
#define Ctt (0.001)

#define Kve (Cve * 0.01)
#define Kcf (Acf * 0.01)
#define Kho (1.0 * 0.01)

// it only has 7 digits accuracy anyway
#define FORMAT_FLOAT_MAX 18

// fixme just plain "rain" is variable in weewx
// fixme check signal; rho;
// fixme check ex_lux; ex_rain_total; ex_altitude;
// fixme ex_acc_iaq; ex_time_trigger;

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

weather_data_entry1 weather_declaration[] = {
  // IDX                    LABEL                     WEEWX SCHEMA               UNIT       ORIG UNIT
  // outside --------------------------------------
  // original type bool --------------------------------------
  // rain counter was reset
  { WDAT_rain_reset        ,F("rain_reset"         ) ,F("ex_rain_reset"        ),F("bool" ),F("bool"  ),1.0,0.0 },
  // original type uint16_t --------------------------------------
  // meter/sec * 100;
  { WDAT_wind_speed        ,F("wind_speed"         ) ,F("windSpeed"            ),F("mi/hr"),F("m/s"   ),Kve,0.0 },
  // degrees; 0 = North; 90 = East;
  { WDAT_wind_direction    ,F("wind_direction"     ) ,F("windDir"              ),F("deg"  ),F("deg"   ),1.0,0.0 },
  // meter/sec * 100; after power on;
  { WDAT_wind_speed_max    ,F("wind_speed_max"     ) ,F("ex_windSpeedMax"      ),F("mi/hr"),F("m/s"   ),Kve,0.0 },
  // 0 to 17 ????;
  { WDAT_wind_rating       ,F("wind_rating"        ) ,F("ex_windRating"        ),F("level"),F("level" ),1.0,0.0 },
  // C * 100; 
  { WDAT_temperature_out   ,F("temperature_out"    ) ,F("outTemp"              ),F("F"    ),F("C"     ),Kcf,Bcf },
  // % RH * 100;  relative humidity;
  { WDAT_humidity_out      ,F("humidity_out"       ) ,F("outHumidity"          ),F("% RH" ),F("% RH"  ),Kho,0.0 },
  { WDAT_rain_bucket_tips  ,F("rain_bucket_tips"   ) ,F("ex_rain_bucket_tips"  ),F("#"    ),F("#"     ),1.0,0.0 },
  { WDAT_rain_bucket_change,F("rain_bucket_change" ) ,F("ex_rain_bucket_change"),F("#"    ),F("#"     ),1.0,0.0 },
  { WDAT_rain_bucket_prev  ,F("rain_bucket_prev"   ) ,F("ex_rain_bucket_prev"  ),F("#"    ),F("#"     ),1.0,0.0 },
  // 10 entries
  // original type float --------------------------------------
  // see RAIN_HEIGHT_PER_BUCKET_TIP; mm or inch;
  { WDAT_rain_total        ,F("rain_total"         ) ,F("ex_rain_total"        ),F("in"   ),F("#"     ),Cra,0.0 },
  { WDAT_client_case_temp  ,F("client_case_temp"   ) ,F("caseTemp1"            ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_client_case_humid ,F("client_case_humid"  ) ,F("caseHumid1"           ),F("% RH" ),F("% RH"  ),1.0,0.0 },
  { WDAT_client_cpu_temp   ,F("client_cpu_temp"    ) ,F("cpuTemp1"             ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_light_full        ,F("light_full"         ) ,F("signal3"              ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  { WDAT_light_ir          ,F("light_ir"           ) ,F("signal5"              ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  { WDAT_light_visible     ,F("light_visible"      ) ,F("signal4"              ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  { WDAT_light_lux         ,F("light_lux"          ) ,F("ex_lux"               ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  { WDAT_light_lux2        ,F("light_lux2"         ) ,F("signal2"              ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  { WDAT_light_rho         ,F("light_rho"          ) ,F("signal6"              ),F("W/m2" ),F("W/m2"  ),1.0,0.0 },
  // UV index
  { WDAT_light_uv          ,F("light_uv"           ) ,F("UV"                   ),F("uvi"  ),F("uvi"   ),1.0,0.0 },
  { WDAT_light_case_temp   ,F("light_case_temp"    ) ,F("caseTemp2"            ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_light_case_humid  ,F("light_case_humid"   ) ,F("caseHumid2"           ),F("% RH" ),F("% RH"  ),1.0,0.0 },
  { WDAT_light_cpu_temp    ,F("light_cpu_temp"     ) ,F("cpuTemp2"             ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_rain_case_temp    ,F("rain_case_temp"     ) ,F("caseTemp3"            ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_rain_case_humid   ,F("rain_case_humid"    ) ,F("caseHumid3"           ),F("% RH" ),F("% RH"  ),1.0,0.0 },
  { WDAT_rain_cpu_temp     ,F("rain_cpu_temp"      ) ,F("cpuTemp3"             ),F("F"    ),F("C"     ),Acf,Bcf },
  // 17 entries
  // inside --------------------------------------
  // original type float --------------------------------------
  { WDAT_temperature_in    ,F("temperature_in"     ) ,F("inTemp"               ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_humidity_in       ,F("humidity_in"        ) ,F("inHumidity"           ),F("% RH" ),F("% RH"  ),1.0,0.0 },
  // sure float is good enough ????
  // not hPa
  { WDAT_pressure          ,F("pressure"           ) ,F("pressure"             ),F("inHg" ),F("Pa"    ),Cpi,0.0 },
  // not kOhm
  { WDAT_gas_resistance    ,F("gas_resistance"     ) ,F("gas_resistance"       ),F("kOhm" ),F("Ohm"   ),Cok,0.0 },
  { WDAT_altitude          ,F("altitude"           ) ,F("ex_altitude"          ),F("ft"   ),F("meter" ),Cmf,0.0 },
  // index of air quality 0 to 500
  { WDAT_iaq_score         ,F("iaq_score"          ) ,F("iaq"                  ),F("iaq"  ),F("iaq"   ),1.0,0.0 },
  // parts per million
  { WDAT_co2_equiv         ,F("co2_equiv"          ) ,F("co2"                  ),F("ppm"  ),F("ppm"   ),1.0,0.0 },
  // 7 entries
  // original type float --------------------------------------
  { WDAT_raw_temperature   ,F("raw_temperature"    ) ,F("ex_raw_temperature"   ),F("F"    ),F("C"     ),Acf,Bcf },
  { WDAT_raw_humidity      ,F("raw_humidity"       ) ,F("ex_raw_humidity"      ),F("% RH" ),F("% RH"  ),1.0,0.0 },
  { WDAT_static_iaq        ,F("static_iaq"         ) ,F("ex_static_iaq"        ),F("iaq"  ),F("iaq"   ),1.0,0.0 },
  { WDAT_breath_voc_equiv  ,F("breath_voc_equiv"   ) ,F("ex_breath_voc_equiv"  ),F("ppm"  ),F("ppm"   ),1.0,0.0 },
  { WDAT_comp_gas_value    ,F("comp_gas_value"     ) ,F("ex_comp_gas_value"    ),F("kOhm" ),F("Ohm"   ),Cok,0.0 },
  { WDAT_gas_percentage    ,F("gas_percentage"     ) ,F("ex_gas_percentage"    ),F("%"    ),F("%"     ),1.0,0.0 },
  // 6 entries
  // original type bool  --------------------------------------
  { WDAT_stab_status       ,F("stab_status"        ) ,F("ex_stab_status"       ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_run_in_status     ,F("run_in_status"      ) ,F("ex_run_in_status"     ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_iaq           ,F("acc_iaq"            ) ,F("ex_acc_iaq"           ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_static_iaq    ,F("acc_static_iaq"     ) ,F("ex_acc_static_iaq"    ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_co2           ,F("acc_co2"            ) ,F("ex_acc_co2"           ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_breath_voc    ,F("acc_breath_voc"     ) ,F("ex_acc_breath_voc"    ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_comp_gas      ,F("acc_comp_gas"       ) ,F("ex_acc_comp_gas"      ),F("bool"),F("bool"   ),1.0,0.0 },
  { WDAT_acc_gas_percentage,F("acc_gas_percentage" ) ,F("ex_acc_gas_percentage"),F("bool"),F("bool"   ),1.0,0.0 },
  // 8 entries
  // original type long  --------------------------------------
  { WDAT_time_trigger      , F("time_trigger"      ) ,F("ex_time_trigger"      ),F("s"   ),F("ms"     ),Ctt,0.0 },
  { WDAT_output_timestamp  , F("output_timestamp"  ) ,F("ex_output_timestamp"  ),F("s"   ),F("ms"     ),Ctt,0.0 },
  // entries = 10 + 17 + 7 + 6 + 8 + 2 = 50 total
  
};

const int iset_outside_all1[] = {
  WDAT_wind_speed,
  WDAT_wind_direction,
  WDAT_wind_speed_max,
  WDAT_wind_rating,
  WDAT_temperature_out,
  WDAT_humidity_out,
  WDAT_client_case_temp,
  WDAT_client_case_humid,
  WDAT_client_cpu_temp,
  WDAT_light_full,
  WDAT_light_ir,
  WDAT_light_visible,
  WDAT_light_lux,
  WDAT_light_lux2,
  WDAT_light_rho,
  WDAT_light_uv,
  WDAT_light_case_temp,
  WDAT_light_case_humid,
  WDAT_light_cpu_temp,
  WDAT_rain_reset,
  WDAT_rain_bucket_tips,
  WDAT_rain_bucket_change,
  WDAT_rain_bucket_prev,
  WDAT_rain_total,
  WDAT_rain_case_temp,
  WDAT_rain_case_humid,
  WDAT_rain_cpu_temp,
};

const int iset_outside_wind0[] = {
  WDAT_wind_speed,
  WDAT_wind_direction,
  WDAT_wind_speed_max,
  WDAT_wind_rating,
};

const int iset_outside_temp0[] = {
  WDAT_temperature_out,
  WDAT_humidity_out,
};

const int iset_outside_light0[] = {
  WDAT_light_full,
  WDAT_light_ir,
  WDAT_light_visible,
  WDAT_light_lux,
  WDAT_light_lux2,
  WDAT_light_rho,
  WDAT_light_uv,
};

const int iset_outside_rain0[] = {
  WDAT_rain_total,
  WDAT_rain_bucket_tips,
  WDAT_rain_bucket_change,
  WDAT_rain_bucket_prev,
  WDAT_rain_reset,
};

const int iset_outside_light1[] = {
  WDAT_light_full,
  WDAT_light_ir,
  WDAT_light_visible,
  WDAT_light_lux,
  WDAT_light_lux2,
  WDAT_light_rho,
  WDAT_light_uv,
  WDAT_light_case_temp,
  WDAT_light_case_humid,
  WDAT_light_cpu_temp,
};

const int iset_outside_rain1[] = {
  WDAT_rain_total,
  WDAT_rain_bucket_tips,
  WDAT_rain_bucket_change,
  WDAT_rain_bucket_prev,
  WDAT_rain_reset,
  WDAT_rain_case_temp,
  WDAT_rain_case_humid,
  WDAT_rain_cpu_temp,
};

const int iset_outside_tsys_client[] = {
  WDAT_client_case_temp,
  WDAT_client_case_humid,
  WDAT_client_cpu_temp,
};

const int iset_outside_tsys_light[] = {
  WDAT_light_case_temp,
  WDAT_light_case_humid,
  WDAT_light_cpu_temp,
};

const int iset_outside_tsys_rain[] = {
  WDAT_rain_case_temp,
  WDAT_rain_case_humid,
  WDAT_rain_cpu_temp,
};

const int iset_inside_all[] = {
  WDAT_temperature_in,
  WDAT_humidity_in,
  WDAT_pressure,
  WDAT_gas_resistance,
  WDAT_altitude,
  WDAT_iaq_score,
  WDAT_co2_equiv,
  // special float
  WDAT_raw_temperature,
  WDAT_raw_humidity,
  WDAT_static_iaq,
  WDAT_breath_voc_equiv,
  WDAT_comp_gas_value,
  WDAT_gas_percentage,
  // bool
  WDAT_stab_status,
  WDAT_run_in_status,
  WDAT_acc_iaq,
  WDAT_acc_static_iaq,
  WDAT_acc_co2,
  WDAT_acc_breath_voc,
  WDAT_acc_comp_gas,
  WDAT_acc_gas_percentage,
  // long time
  WDAT_time_trigger,
  WDAT_output_timestamp,
};

const int iset_inside_radio[] = {
  WDAT_temperature_in,
  WDAT_humidity_in,
  WDAT_pressure,
  WDAT_gas_resistance,
  WDAT_altitude,
  WDAT_iaq_score,
  WDAT_co2_equiv,
  // bool
  WDAT_acc_iaq,
};

// 0 = without tsys_light; tsys_rain;
// 1 = with    tsys_light; tsys_rain;
WIS WIS_outside_all1       (iset_outside_all1       , sizeof(iset_outside_all1       ));
WIS WIS_outside_wind0      (iset_outside_wind0      , sizeof(iset_outside_wind0      ));
WIS WIS_outside_temp0      (iset_outside_temp0      , sizeof(iset_outside_temp0      ));
WIS WIS_outside_light0     (iset_outside_light0     , sizeof(iset_outside_light0     ));
WIS WIS_outside_rain0      (iset_outside_rain0      , sizeof(iset_outside_rain0      ));
WIS WIS_outside_light1     (iset_outside_light1     , sizeof(iset_outside_light1     ));
WIS WIS_outside_rain1      (iset_outside_rain1      , sizeof(iset_outside_rain1      ));
WIS WIS_outside_tsys_client(iset_outside_tsys_client, sizeof(iset_outside_tsys_client));
WIS WIS_outside_tsys_light (iset_outside_tsys_light , sizeof(iset_outside_tsys_light ));
WIS WIS_outside_tsys_rain  (iset_outside_tsys_rain  , sizeof(iset_outside_tsys_rain  ));
WIS WIS_inside_all         (iset_inside_all         , sizeof(iset_inside_all         ));
WIS WIS_inside_radio       (iset_inside_radio       , sizeof(iset_inside_radio       ));

/* globals end ----------------------------------------------- */

template <>
void weather_data::print_item<float>(float item) {
  static char buf[FORMAT_FLOAT_MAX];
  sprintf(buf, "%.7f", item);
  Serial.print(buf);
}

template <>
void weather_data::print_item<format_string>(format_string item) {
  Serial.print(F("\""));
  Serial.print(item);
  Serial.print(F("\""));
}

int weather_data::get(int idx, float &val) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("wdat index"));
  {
    weather_data_entry &entry = m_data[idx];
    val = entry.m_val;
  }
  return 0;
 catch_block:
  wexception.print();
  return 1;
}

int weather_data::get_name_mine(int idx, format_string &name_mine) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("wdat index"));
  {
    weather_data_entry &entry = m_data[idx];
    name_mine = entry.m_name_mine;
  }
  return 0;
 catch_block:
  wexception.print();
  return 1;
}

int weather_data::get_name_weewx(int idx, format_string &name_weewx) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("wdat index"));
  {
    weather_data_entry &entry = m_data[idx];
    name_weewx = entry.m_name_weewx;
  }
  return 0;
 catch_block:
  wexception.print();
  return 1;
}

int weather_data::set(int idx, float val, bool convert) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("wdat index"));
  {
    weather_data_entry &entry = m_data[idx];
    if (convert) {
      entry.m_val = val * entry.m_convert_a + entry.m_convert_b;
    } else {
      entry.m_val = val;
    }
  }
  return 0;
 catch_block:
  wexception.print();
  return 1;
}

// fixme just use original table
// assign the fixed part of m_data[idx] during setup
void weather_data::store(weather_data_entry1 &entry1) {
  FUNC_BEGIN;
  int idx = entry1.m_idx;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("wdat index"));
  // fixme check for duplicate idx;
  ERROR_CONDITION((m_data[idx].m_idx == -1), F("m_data index duplicate (reuse)"));
  {
    weather_data_entry &entry = m_data[idx];
    entry.m_idx         = entry1.m_idx;
    entry.m_name_mine   = entry1.m_name_mine;
    entry.m_name_weewx  = entry1.m_name_weewx;
    entry.m_unit        = entry1.m_unit;
    entry.m_unit_orig   = entry1.m_unit_orig;
    entry.m_convert_a   = entry1.m_convert_a;
    entry.m_convert_b   = entry1.m_convert_b;
    entry.m_strlen_name_mine  = strlen((const char *) entry1.m_name_mine); // fixme
    entry.m_strlen_name_weewx = strlen((const char *) entry1.m_name_weewx); // fixme
    m_strlen_name_mine_max    = max(m_strlen_name_mine_max , entry.m_strlen_name_mine );
    m_strlen_name_weewx_max   = max(m_strlen_name_weewx_max, entry.m_strlen_name_weewx);
  }
  return;
 catch_block:
  wexception.print();
}

// copy weather_declaration to m_data
void weather_data::setup(void) {
  FUNC_BEGIN;
  ser_println(F("wdat::setup() begin"));
  int sz1 = sizeof(weather_declaration);
  int sz2 = sizeof(weather_data_entry1);
  int len = sz1 / sz2;
  ERROR_CONDITION(((sz1 % sz2) == 0), F("impossible weather_declaration"));
  ERROR_CONDITION((len == WDAT_ENTRY_MAX), F("len == DATA_ENTRY_MAX"));
  // clear fixed data
  m_strlen_name_mine_max = 0;
  m_strlen_name_weewx_max = 0;
  for (int idx = 0; idx < WDAT_ENTRY_MAX; idx++) {
    weather_data_entry &entry = m_data[idx];
    entry.m_idx = -1;
  }
  // clear not fixed data
  set_all_data_zero();
  // copy weather_declaration fixed data to m_data
  for (int idx = 0; idx < len; idx++) {
    // store entry1 at idx1 and print (echo)
    weather_data_entry1 &entry1 = weather_declaration[idx];
    int idx1 = entry1.m_idx;
    store(entry1);
    print_idx_fixed(idx1);
  }
  ser_println(F("wdat::setup() end pass"));
  Serial.flush();
  watchdog_reset();
  return;
 catch_block:
  ser_println(F("wdat::setup() end fail"));
  wexception.print();
  Serial.flush();
}

// not fixed data
void weather_data::set_all_data_zero(void) {
  for (int idx = 0; idx < WDAT_ENTRY_MAX; idx++) {
    weather_data_entry &entry = m_data[idx];
    entry.m_val = 0.0;
  }
}

void weather_data::print_entry(weather_data_entry &entry) {
  FUNC_BEGIN;
  // %s with m_strlen_max chars
  snprintf(snprintf_buf, SNPRINTF_BUF_MAX, "%-*s = %15.7f %s",
           m_strlen_name_mine_max,
           ((const char *) (entry.m_name_mine)),
           entry.m_val,
           ((const char *) (entry.m_unit)));
  ser_println(snprintf_buf);
  return;
 catch_block:
  wexception.print();
}

void weather_data::print_entry_fixed(weather_data_entry &entry) {
  FUNC_BEGIN;
  char buf1[23];
  char buf2[8];
  char buf3[8];
  snprintf(buf1, 23, "\"%s\"", ((const char *) (entry.m_name_mine)));
  snprintf(buf2,  8, "\"%s\"", ((const char *) (entry.m_unit)));
  snprintf(buf3,  8, "\"%s\"", ((const char *) (entry.m_unit_orig)));
  snprintf(snprintf_buf, SNPRINTF_BUF_MAX, "<< %2d, %-22s, %-7s, %-5s, %f, %f >>",
           entry.m_idx,
           buf1,
           buf2,
           buf3,
           entry.m_convert_a,
           entry.m_convert_b);
  ser_println(snprintf_buf);
  return;
 catch_block:
  wexception.print();
}

void weather_data::print_idx(int idx) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("print idx bounds"));
  {
    weather_data_entry &entry = m_data[idx];
    print_entry(entry);
  }
  return;
 catch_block:
  wexception.print();
}

void weather_data::print_idx_fixed(int idx) {
  FUNC_BEGIN;
  BOUNDS(idx, 0, WDAT_ENTRY_MAX, F("print idx bounds"));
  {
    weather_data_entry &entry = m_data[idx];
    print_entry_fixed(entry);
  }
  return;
 catch_block:
  wexception.print();
}

void weather_data::print(WIS &idx_set) {
  FUNC_BEGIN;
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int idx = idx_set.m_idx[jjj];
    print_idx(idx);
  }
  return;
 catch_block:
  wexception.print();
}

void weather_data::print_outside_sensor(device_idx ws) {
  FUNC_BEGIN;
  switch (ws) {
  case ws_WIND:    print_outside_wind0();  break;
  case ws_TEMP:    print_outside_temp0();  break;
  case ws_LIGHT:   print_outside_light0(); break;
  case ws_RAIN:    print_outside_rain0();  break;
  default:
    ERROR_CONDITION(false, F("print_outside_sensor; bad ws;"));
    break;
  }
  return;
 catch_block:
  ;
}

void weather_data::print_outside_tsys(device_idx ws) {
  FUNC_BEGIN;
  switch (ws) {
  case ws_WIND:    break;
  case ws_TEMP:    break;
  case ws_LIGHT:   print_outside_tsys_light();  break;
  case ws_RAIN:    print_outside_tsys_rain();   break;
  case ws_TSYS:    print_outside_tsys_client(); break;
  default:
    ERROR_CONDITION(false, F("print_outside_tsys; bad ws;"));
    break;
  }
  return;
 catch_block:
  ;
}

// print TTT as entry in json dictionary e.g.
// { "key0" : val0, "key1" : val1, "key2" : val2, etc. }
template <typename TTT>
void weather_data::print_weewx_val(format_string name_weewx,
                                   bool done,
                                   TTT val,
                                   format_string separator)
{
  Serial.print(F("\""));
  Serial.print(name_weewx);
  Serial.print(F("\": "));
  if (done) {
    print_item(val);
  } else {
    Serial.print(F("None"));
  }
  Serial.print(separator);
}

// from get(iii, val); val is in final converted units; e.g. inTemp is Fahrenheit;
void weather_data::print_weewx_VAL(format_string name_weewx,
                                   bool done,
                                   int idx,
                                   format_string separator)
{
  float val;
  get(idx, val);
  print_weewx_val(name_weewx, done, val, separator);
}


void weather_data::print_weewx_WIS(bool done, WIS &idx_set, bool final_separator) {
  static format_string separator = F("");
  for (int jjj = 0; jjj < idx_set.m_MAX; jjj++) {
    int iii = idx_set.m_idx[jjj];
    float val;
    get(iii, val);
    format_string name_weewx;
    get_name_weewx(iii, name_weewx);
    if (final_separator && (jjj == idx_set.m_MAX - 1)) {
      print_weewx_val(name_weewx, done, val, separator);
    } else {
      print_weewx_val(name_weewx, done, val);
    }
  }
}

void weather_data::print_inside_all_weewx(uint32_t date_time) {
  Serial.print(F("{"));
  bool done = true;
  // this is a quick timestamp for comparison; delete in weewx driver;
  print_weewx_val(F("aaa_millis"    ), done,  millis());
  /*
  // printt(F("time"          ), done,  date_time);
  print_weewx_val(F("time"          ), done,  date_time);
  print_weewx_val(F("protocol"      ), done,  40);
  print_weewx_val(F("model"         ), done,  F("Acurite-Atlas"));
  // fixme; not important; sensor_map uses wildcards
  print_weewx_val(F("id"            ), done,  0x229);  
  print_weewx_val(F("channel"       ), done,  F("A"));  // fixme check this
  print_weewx_val(F("sequence_num"  ), done,  3);       // fixme check this
  print_weewx_val(F("message_type"  ), done,  8);       // fixme check this
  // print_weewx_VAL same as print_weewx_val but get val at m_data[idx];
  print_weewx_VAL(F("inTemp"        ), done,  WDAT_temperature_in);
  print_weewx_VAL(F("inHumidity"    ), done,  WDAT_humidity_in);
  print_weewx_VAL(F("pressure"      ), done,  WDAT_pressure);
  print_weewx_VAL(F("altitude"      ), done,  WDAT_altitude);
  print_weewx_VAL(F("gas_resistance"), done,  WDAT_gas_resistance);
  print_weewx_VAL(F("iaq"           ), done,  WDAT_iaq_score);
  print_weewx_VAL(F("co2"           ), done,  WDAT_co2_equiv, F(""));
  */
  bool final_separator = true;
  // note also has acc_iaq in addition to above original fields;
  print_weewx_WIS(done, WIS_inside_radio, final_separator);
  Serial.println(F("}"));
}

// so far only wmod uses this
void weather_data::print_inside_outside_all_weewx(uint32_t date_time,
                                                  loop_counter_outside &loopc_out,
                                                  loop_counter_inside &loopc_in)
{
  Serial.print(F("{"));
  {
    bool done_wind = loopc_out.m_done[ws_WIND];
    bool done_temp = loopc_out.m_done[ws_TEMP];
    bool done_ligh = loopc_out.m_done[ws_LIGH];
    bool done_rain = loopc_out.m_done[ws_RAIN];
    bool done_tsys = loopc_out.m_done[ws_TSYS];    // fixme check this
    print_weewx_WIS(done_wind, WIS_outside_wind0);
    print_weewx_WIS(done_temp, WIS_outside_temp0);
    print_weewx_WIS(done_ligh, WIS_outside_light0);
    print_weewx_WIS(done_rain, WIS_outside_rain0);
    print_weewx_WIS(done_tsys, WIS_outside_tsys_client);
    print_weewx_WIS(done_tsys, WIS_outside_tsys_light);
    print_weewx_WIS(done_tsys, WIS_outside_tsys_rain);
  }
  {
    bool final_separator = true;
    bool done_temp = loopc_in.m_done[ws_TEMP];
    print_weewx_WIS(done_temp, WIS_inside_radio, final_separator);
  }
  Serial.println(F("}"));
}

// eee eof
