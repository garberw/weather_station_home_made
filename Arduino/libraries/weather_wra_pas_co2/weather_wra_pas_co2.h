#ifndef weather_wra_pas_co2_h
#define weather_wra_pas_co2_h

class weather_pas_co2 {
public:
  int32_t m_last_time_msec;
  int16_t m_last_co2;
  void setup(void);
  int start_next_reading(void);
  int wait_for_reading(void);
  int  advanced_read(weather_data &wdat);
  void copy_to_wdat(weather_data &wdat);
  void print(void);
};

#endif
// eee eof
