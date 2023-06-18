#ifndef weather_lib_array_h
#define weather_lib_array_h

#include <weather_lib_util.h>

template <typename T>
class weather_array {
public:
  weather_array(void) : m_name(NULL), m_MAX(0), m_array(NULL) { }
  weather_array(format_string name, int MAX) { setup(name, MAX); }
  ~weather_array(void) { delete [] m_array; }
  void setup(format_string name, int MAX) {
    m_name = name;
    m_MAX = MAX;
    resize(MAX);
  }
  int            get_size    (void ) { return m_MAX; }
  int            get_MAX     (void ) { return m_MAX; }
  T *            get_data    (void ) { return m_array; }
  format_string &name        (void ) { return m_name; }
  void resize(int MAX) {
    delete [] m_array;
    m_MAX = MAX;
    // never access m_array[m_MAX] except to return gibberish on overflows;
    m_array = new T[MAX + 1];
    // new throws an exception on failure; arduino does not support this;
    // so far we just have to assume it succeeds;
    /*
    // halt if not allocated
    if (m_array == NULL) {
    // weather_halt();
      while (1) {
        Serial.println(F("halted")); Serial.println(millis()); delay(1000);
      }
    }
    */
  }
  int get(int i, T &val) {
    BOUNDS(i, 0, m_MAX, F("get"));
    val = m_array[i];
    return 0;
  catch_block:
    Serial.println(F("bounds check"));
    Serial.print(F("array    name = ")); Serial.println(m_name);
    Serial.print(F("i             = ")); Serial.println(i);
    Serial.print(F("MAX           = ")); Serial.println(m_MAX);
    // weather_halt();
    return 1;
  }
  int set(int i, T &val) {
    BOUNDS(i, 0, m_MAX, F("set"));
    m_array[i] = val;
    return 0;
  catch_block:
    Serial.println(F("bounds check"));
    Serial.print(F("array    name = ")); Serial.println(m_name);
    Serial.print(F("i             = ")); Serial.println(i);
    Serial.print(F("MAX           = ")); Serial.println(m_MAX);
    // weather_halt();
    return 1;
  }
  T & at(int i) {
    BOUNDS(i, 0, m_MAX, F("at()"));
    return m_array[i];
  catch_block:
    // reports error but does not halt or branch
    Serial.println(F("bounds check"));
    Serial.print(F("array    name = ")); Serial.println(m_name);
    Serial.print(F("i             = ")); Serial.println(i);
    Serial.print(F("MAX           = ")); Serial.println(m_MAX);
    /*
    // halt if not allocated
    if (m_array == NULL) {
      weather_halt();
    }
    */
    Serial.println(F("return gibberish when out of bounds"));
    return m_array[m_MAX];
  }
  format_string m_name;
  int           m_MAX;
  T *           m_array;
};

#endif
// eee eof
