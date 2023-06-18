#ifndef weather_wra_bme688_h
#define weather_wra_bme688_h

class weather_bme688 {
public:
  float   m_altitude;
  String  m_iaq_text;
  void setup(void);
  bool perform_reading(void);
  int  advanced_read(weather_data &wdat);
  void copy_to_wdat(weather_data &wdat);
  void print(void);
  int m_stub;
};

#endif
// eee eof
