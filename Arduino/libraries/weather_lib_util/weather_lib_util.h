#ifndef weather_lib_util_h
#define weather_lib_util_h

#include <Adafruit_SleepyDog.h>
#include <MemoryFree.h>
#include <Arduino.h>
// #include <SoftwareSerial.h>

/* macros error checking begin ----------------------------------------------- */

// options begin -----------------------------

// milliseconds
#define SETUP_DELAY           1000

// fixme set 0 or itsy bitsy arduinos will not start when disconnected from usb
#define PAUSE_ON_START        0

#define RS485Serial           Serial1

#define SERIAL_PRINT_BAUD     115200

#define RS485Serial_BAUD      9600

// options end -----------------------------

#define WDAT_PASS true
#define WDAT_FAIL false

// the longest combination of file + function
#define SERIAL_PRINT_BUF_MAX 64
#define SERIAL_PRINT_HASH_MAX 2309
#define SERIAL_PRINT_HASH_BASE 31
#define SERIAL_PRINT_STACK_MAX 1024

// exceptions
#define EXCEPTION_STACK_MAX 10
#define RESULT_SUCCESS true
#define RESULT_FAIL    false

// fixme remove
// #define ERROR_TYPE_FATAL    1
// #define ERROR_TYPE_RETURN   2
// #define ERROR_TYPE_CONTINUE 3
// #define ERROR_TYPE_CATCH    4

#define CHECK_MEMORY_FREE_MIN { memory_free_min = min(freeMemory(), memory_free_min); }

// fixme calls ser_print
#define PRINT_FREE_MEMORY(msg) { ser_print(msg); ser_print(F(" freeMemory = ")); ser_print(freeMemory()); CHECK_MEMORY_FREE_MIN; ser_print(F(" memory_free_min = ")); ser_println(memory_free_min); Serial.flush(); }

// the original error (throw/raise); not the return of a function with an error (re-raise);
// push error
// warning
// backtrace
// goto catch block
#define ERROR_CONDITION(condition, msg            ) \
  if (!(condition)) {                               \
    weather_error_condition((__func__), (msg));     \
    goto catch_block;                               \
  }

// the original error (throw/raise); not the return of a function with an error (re-raise);
// push error
// warning
// backtrace
// fixme halt (commented out)
// goto catch block
#define ERROR_FATAL(   condition, msg            ) \
  if (!(condition)) {                              \
    weather_error_fatal   ((__func__), (msg));     \
    goto catch_block;                              \
  }

// the original error (throw/raise); not the return of a function with an error (re-raise);
// do not push error
// no warning
// no backtrace
// return (return_val)
#define ERROR_SILENT(  condition, msg, return_val) \
  if (!(condition)) {                              \
    return (return_val);                           \
  }

// call after a function returns an error code; exit up the call tree;
// also call after error you do not want to push (push means report to calling function);
// if you are not sure just do ERROR_CONDITION (only difference is a redundant backtrace);
// do not push error
// warning
// no backtrace
// goto catch block
#define ERROR_RETURN(  condition, msg            ) \
  if (!(condition)) {                              \
    weather_error_return  ((__func__), (msg));     \
    goto catch_block;                              \
  }

// catch previously pushed errors
// wexception.any_set() true iff (no error exists)
// warning
// no backtrace
// goto catch block
#define ERROR_CATCH                                \
  if (wexception.any_set()) {                     \
    weather_error_catch(__func__);                 \
    goto catch_block;                              \
  }

#define BOUNDS(JJJ, IDX0, IDX1, MSG) ERROR_CONDITION(( ((IDX0)<=(JJJ)) && ((JJJ)<(IDX1)) ), (MSG) )

// order in arduino setup() function
// (1) setup_communication_ser0(); required (only once globally) for ...
// (2) ser_setup();                required (only once globally) for ...
// (3) FUNC_BEGIN;                 required (applies until func end
//                                    unless func called by this one overrides it)
// FUNC_BEGIN should always be first statement in function or absent;
// do not call FUNC_BEGIN more than once per function;

// use FUNC_BEGIN at the beginning of each function using ser_print();
// the LOCK constructor pushes m_used_now;
// using FUNC_END is not needed;
// on function exit the LOCK destructor is called and pops m_used_now;
// if you did not use FUNC_BEGIN ser_print will just get used_now() from previous function
// i.e. most recent value set by previous functions calling FUNC_BEGIN;

// LOCK_CACHE computes hash lookup on first execution of function;
// LOCK looks up used in LOCK_CACHE instead of recomputing m_used hash lookup each time;
// much faster !!!!;

#define FUNC_BEGIN static serial_print_lock_cache LOCK_CACHE(__FILE__, __func__); serial_print_lock LOCK(LOCK_CACHE.m_used, __func__);

#define FUNC_BEGIN_ALWAYS_ON serial_print_lock LOCK(true);

#define FUNC_BEGIN_ALWAYS_OFF serial_print_lock LOCK(false);

#define FUNC_END { }

#define SNPRINTF_BUF_MAX 99

/* macros error checking end ----------------------------------------------- */
/* error checking typedefs begin ----------------------------------------------- */

// weather_exception does not use serial_print       and can be set up first;
// serial_print      does     use weather_exception  (but it could easily be made independent);

typedef const __FlashStringHelper *format_string;

typedef bool result_type;

struct serial_print_entry {
  bool m_used;
  format_string m_file;
  format_string m_func;
};

extern const char main_name[];

class serial_print {
public:
  serial_print(void) : m_setup_done(false) {
    // stack should never be empty
    // after this entry for global scope which should never be popped;
    // the only things calling push and pop should be LOCK constructor and destructor;
    m_stack_top = 0;
    push_used_now(true, main_name);
  }
  void setup(void);
  void copy_flash_to_char(format_string fff, char *buf);
  void copy_char_to_char(const char *fff, char *buf);
  void save_names1(format_string file, format_string func);
  void save_names2(format_string file, format_string func);
  void save_names1(const char *file, const char *func);
  void save_names2(const char *file, const char *func);
  uint16_t hash_function(const char *file, const char *func);
  void store(bool used, format_string file, format_string func, int tag=-1);
  bool load(const char *file_raw, const char *func);
  bool check_stack(int new_top);
  bool used_now(void);
  void push_used_now(bool used, const char *func);
  void pop_used_now(void);
  void backtrace(void);
  template <typename TTT>
  void diagnostic(format_string where,
                  format_string result,
                  int attempt,
                  int jjj,
                  int used,
                  TTT file,
                  TTT func,
                  bool empty,
                  int tag=-1);
  int m_stack_top;
  bool m_used_now[SERIAL_PRINT_STACK_MAX];
  const char *m_func_now[SERIAL_PRINT_STACK_MAX];
  bool m_setup_done;
  char m_file1[SERIAL_PRINT_BUF_MAX];
  char m_file2[SERIAL_PRINT_BUF_MAX];
  char m_func1[SERIAL_PRINT_BUF_MAX];
  char m_func2[SERIAL_PRINT_BUF_MAX];
  format_string m_hash_file[SERIAL_PRINT_HASH_MAX];
  format_string m_hash_func[SERIAL_PRINT_HASH_MAX];
  bool m_hash_used[SERIAL_PRINT_HASH_MAX];
  int m_store_pass;
  int m_store_fail;
  int m_store_dupl;
  // collision
  int m_store_coll;
};

// global ----------------------------

extern serial_print static_serial_print;

// global ----------------------------

// declare variables of this type as static in FUNC_BEGIN
class serial_print_lock_cache {
public:
  serial_print_lock_cache(const char *file, const char *func) {
    m_used = static_serial_print.load(file, func);
  }
  bool m_used;
};

class serial_print_lock {
public:
  // FUNC_BEGIN provides serial_print_lock_cache.m_used;
  serial_print_lock(bool used, const char *func) {
    static_serial_print.push_used_now(used, func);
  }
  ~serial_print_lock(void) {
    static_serial_print.pop_used_now();
  }
};

class weather_exception {
public:
  weather_exception(void) {
    clear_all();
  }
  void clear_all(void) { m_nerror = 0; }
  bool any_set(void) { return m_nerror; }
  void push(const char *function_name, format_string msg);
  void print(void);
  byte m_nerror;
  const char *m_function_name[EXCEPTION_STACK_MAX];
  format_string m_msg[EXCEPTION_STACK_MAX];
}; // class weather_exception;

/* error checking typedefs end ----------------------------------------------- */
/* functions begin ----------------------------------------------- */

int setup_communication_ser0(void);
int setup_communication_ser1(void);
void setup_delay(void);
void pause_on_start(void);
void weather_halt(void);
void check_halt(void);
void print_uint32(uint32_t n);
void print_hex_8(byte d);
void print_hex_16(uint16_t w);
void print_dec_8(byte d);
void pack_long_into_uint16_t(uint16_t &H, uint16_t &L, uint32_t D);
void memcpy_pack_float_into_uint16_t(uint16_t &WH, uint16_t &WL, float FL);
void memcpy_unpack_uint16_t_into_float(uint16_t WH, uint16_t WL, float &FL);
void divider_line(format_string str, int n);
void divider_line_error_lead(void);
void divider_line_result(result_type result, format_string name);
void divider_line_loop_begin(void);
void divider_line_sensor(format_string name);
void ser_println(void);
void ser_store(bool used, format_string file, format_string func);
void ser_options(void);
void ser_setup(void);
void weather_error_fatal(const char *function_name, format_string msg);
void weather_error_return(const char *function_name, format_string msg);
void weather_error_condition(const char *function_name, format_string msg);
void weather_error_catch(const char *function_name);
void watchdog_setup(void);
void watchdog_reset(void);

/* functions end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

extern weather_exception wexception;

extern bool halt_state;

extern int memory_free_min;

extern char snprintf_buf[SNPRINTF_BUF_MAX];

/* globals end ----------------------------------------------- */
/* templates begin ----------------------------------------------- */

template <typename TTT>
void ser_print(TTT const& arg) {
  if (static_serial_print.used_now()) Serial.print(arg);
}

template <typename TTT>
void ser_print(TTT const& arg, int fmt) {
  if (static_serial_print.used_now()) Serial.print(arg, fmt);
}

template <typename TTT>
void ser_println(TTT const& arg) {
  if (static_serial_print.used_now()) Serial.println(arg);
}

template <typename TTT>
void ser_println(TTT const& arg, int fmt) {
  if (static_serial_print.used_now()) Serial.println(arg, fmt);
}
/* templates end ----------------------------------------------- */

#endif
// eee eof
