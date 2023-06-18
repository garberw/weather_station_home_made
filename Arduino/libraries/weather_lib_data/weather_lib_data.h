#ifndef weather_lib_data_h
#define weather_lib_data_h

// fixme 
#include <weather_lib_util.h>
#include <weather_lib_counter.h>
#include <weather_lib_modbus_macros.h>

/* macros begin ----------------------------------------------- */

// see README.rain_units.txt

#define CONVERT_MPS_TO_MPH              (2.23694)
#define CONVERT_C_TO_F                  (9.0 / 5.0)
// inches rain per bucket tip                        
#define RAIN_HEIGHT_PER_BUCKET_TIP_INCH (0.014776181)
#define RAIN_HEIGHT_PER_BUCKET_TIP_MM   (0.375315)
#define CONVERT_PA_TO_IN_HG             (0.000295333727)
#define CONVERT_IN_HG_TO_PA             (1.0 / CONVERT_PA_TO_IN_HG)
#define CONVERT_M_TO_FT                 (3.280839895)
#define CONVERT_RAIN                    (RAIN_HEIGHT_PER_BUCKET_TIP_INCH)
#define CONVERT_OHM_TO_KOHM             (0.001)


// outside
#define WDAT_wind_speed         ( 0)
#define WDAT_wind_direction     ( 1)
#define WDAT_wind_speed_max     ( 2)
#define WDAT_wind_rating        ( 3)
#define WDAT_temperature_out    ( 4)
#define WDAT_humidity_out       ( 5)
#define WDAT_client_case_temp   ( 6)
#define WDAT_client_case_humid  ( 7)
#define WDAT_client_cpu_temp    ( 8)
#define WDAT_light_full         ( 9)
#define WDAT_light_ir           (10)
#define WDAT_light_visible      (11)
#define WDAT_light_lux          (12)
#define WDAT_light_lux2         (13)
#define WDAT_light_uv           (14)
#define WDAT_light_rho          (15)
#define WDAT_light_case_temp    (16)
#define WDAT_light_case_humid   (17)
#define WDAT_light_cpu_temp     (18)
#define WDAT_rain_bucket_tips   (19)
#define WDAT_rain_bucket_change (20)
#define WDAT_rain_bucket_prev   (21)
#define WDAT_rain_total         (22)
#define WDAT_rain_reset         (23)
#define WDAT_rain_case_temp     (24)
#define WDAT_rain_case_humid    (25)
#define WDAT_rain_cpu_temp      (26)
// inside
#define WDAT_temperature_in     (27)
#define WDAT_humidity_in        (28)
#define WDAT_pressure           (29)
#define WDAT_gas_resistance     (30)
#define WDAT_altitude           (31)
#define WDAT_iaq_score          (32)
#define WDAT_co2_equiv          (33)
#define WDAT_raw_temperature    (34)
#define WDAT_raw_humidity       (35)
#define WDAT_static_iaq         (36)
#define WDAT_breath_voc_equiv   (37)
#define WDAT_comp_gas_value     (38)
#define WDAT_gas_percentage     (39)
#define WDAT_stab_status        (40)
#define WDAT_run_in_status      (41)
#define WDAT_acc_iaq            (42)
#define WDAT_acc_static_iaq     (43)
#define WDAT_acc_co2            (44)
#define WDAT_acc_breath_voc     (45)
#define WDAT_acc_comp_gas       (46)
#define WDAT_acc_gas_percentage (47)
#define WDAT_time_trigger       (48)
#define WDAT_output_timestamp   (49)
#define WDAT_ENTRY_MAX          (50)

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

/*
extern const int iset_outside_all[];
extern const int iset_outside_wind[];
extern const int iset_outside_temp[];
extern const int iset_outside_light[];
extern const int iset_outside_rain[];
extern const int iset_outside_tsys_client[];
extern const int iset_inside_all[];
extern const int iset_inside_temp[];
extern const int iset_outside_light_without_tsys[];
extern const int iset_outside_rain_without_tsys[];
extern const int iset_outside_tsys_light[];
extern const int iset_outside_tsys_rain[];
*/

// weather data entry index set; just an array of int indices;
class WIS;

extern WIS WIS_outside_all1;
extern WIS WIS_outside_wind0;
extern WIS WIS_outside_temp0;
extern WIS WIS_outside_light0;
extern WIS WIS_outside_rain0;
extern WIS WIS_outside_light1;
extern WIS WIS_outside_rain1;
extern WIS WIS_outside_tsys_client;
extern WIS WIS_outside_tsys_light;
extern WIS WIS_outside_tsys_rain;
extern WIS WIS_inside_all;
extern WIS WIS_inside_radio;

/* globals end ----------------------------------------------- */
/* typedefs begin ----------------------------------------------- */

// weather data entry index set; just an array of int indices;
class WIS {
public:
  // m_idx must be globally allocated
  WIS(const int *idx, int max) : m_MAX(max / 4), m_idx(idx) {
    if ((max % 4) != 0) {
      Serial.println(F("impossible"));
      // weather_halt();
    }
  }
  int m_MAX;
  const int *m_idx;
}; // class WIS;

class weather_data_entry1 {
public:
  int           m_idx;
  format_string m_name_mine;
  format_string m_name_weewx;
  format_string m_unit;
  format_string m_unit_orig;
  float         m_convert_a;
  float         m_convert_b;
};

// e.g. temperature [F] = m_convert_a * temperature [C] + m_convert_b;
// m_unit = "F";
// m_unit_orig = "C";
// m_convert_a = 9/5;
// m_convert_b = 32;
class weather_data_entry {
public:
  // these change;
  float         m_val;
  // these are fixed after setup;
  int           m_idx;
  format_string m_name_mine;
  format_string m_name_weewx;
  format_string m_unit;
  format_string m_unit_orig;
  float         m_convert_a;
  float         m_convert_b;
  uint16_t      m_strlen_name_mine;
  uint16_t      m_strlen_name_weewx;
};

class weather_data {
public:
  void store(weather_data_entry1 &entry);
  int  set(int idx, float val, bool convert = false);
  int  set_iaq_text(String iaq_text) { m_iaq_text = iaq_text; return 0; }
  int  get(int idx, float &val);
  int  get_name_mine(int idx, format_string &name_mine);
  int  get_name_weewx(int idx, format_string &name_weewx);
  void setup(void);
  void set_all_data_zero(void);
  void print_entry_fixed(weather_data_entry &entry);
  void print_idx_fixed(int idx);
  void print_entry(weather_data_entry &entry);
  void print_idx(int idx);
  void print(WIS &idx_set);
  // outside; wmod;
  void print_outside_sensor     (device_idx ws);
  void print_outside_tsys       (device_idx ws);
  void print_outside_all1       (void) { print(WIS_outside_all1       ); }
  void print_outside_wind0      (void) { print(WIS_outside_wind0      ); }
  void print_outside_temp0      (void) { print(WIS_outside_temp0      ); }
  void print_outside_light0     (void) { print(WIS_outside_light0     ); }
  void print_outside_rain0      (void) { print(WIS_outside_rain0      ); }
  void print_outside_light1     (void) { print(WIS_outside_light1     ); }
  void print_outside_rain1      (void) { print(WIS_outside_rain1      ); }
  void print_outside_tsys_client(void) { print(WIS_outside_tsys_client); }
  void print_outside_tsys_light (void) { print(WIS_outside_tsys_light ); }
  void print_outside_tsys_rain  (void) { print(WIS_outside_tsys_rain  ); }
  // inside; wmod atlas and green;
  void print_inside_all         (void) { print(WIS_inside_all         ); }
  void print_inside_radio       (void) { print(WIS_inside_radio       ); }
  // weewx; atlas and green;
  void print_inside_all_weewx   (uint32_t date_time);
  void print_inside_outside_all_weewx(uint32_t date_time,
                                      loop_counter_outside &loopc_out,
                                      loop_counter_inside &loopc_in);
  weather_data_entry m_data[WDAT_ENTRY_MAX];
  String m_iaq_text;
  int m_strlen_name_mine_max;
  int m_strlen_name_weewx_max;
private:
  // void printt(format_string name_weewx, DateTime &date_time, format_string separator=F(", "));
  template <typename TTT>
  void print_item(TTT item) { Serial.print(item); }
  template <typename TTT>
  void print_weewx_val(format_string name_weewx, bool done, TTT val,
                       format_string separator=F(", "));
  void print_weewx_VAL(format_string name_weewx, bool done, int idx,
                       format_string separator=F(", "));
  void print_weewx_WIS(bool done, WIS &idx_set, bool final_separator=false);
}; // class weather_data;

/* typedefs end ----------------------------------------------- */

#endif
// eee eof
