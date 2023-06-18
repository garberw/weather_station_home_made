// atlas Indoor Temp Sensor
/*
  fixme
  DateTime returned by m_rtc.now() does not know about DST (daylight savings time);
  check if python driver knows about DST;
*/
/*
  in this sync version sync clock uses utc;
  just use unixepoch but have to conform to AcuriteAtlas format for record merge;
  converted weewx driver and rtl_433 to use unixepoch instead of format string YYYY-MM-DD HHMMSS;
  change /etc/rtl_433/rtl_433.conf in -M section for GMT/UTC unixepoch integer
  "report_meta time:unix"
  and in /etc/weewx/weewx_atlas update rtl_433 option to include -M unix instead of -M utc;
*/

#include <weather_lib_util.h>
#include <weather_lib_timer.h>
#include <weather_lib_data.h>
// #include <weather_lib_counter.h>
// #include <weather_lib_rtc.h>
#include <weather_wra_bme688.h>
#include <weather_wra_pas_co2.h>

/* macros begin ----------------------------------------------- */

#define ATTEMPTS_MAX_TIME_SYNC 4

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

// time_with_sync time_sync;
weather_bme688 iaq;
weather_pas_co2 wco2;
weather_data wdat;
weather_timer stopw;
// loop_counter_single loopc_time_sync;
loop_counter_single loopc_rest;

/* globals end ----------------------------------------------- */

void setup(void) {
  setup_communication_ser0();
  //  while (!Serial);
  watchdog_setup();
  wexception.clear_all();
  ser_setup();
	FUNC_BEGIN;
  
  stopw.setup();
  ser_println(F("Sensor BME688 Temperature; Humidity; Pressure; Gas Resistance;"));
  ser_println(F("send output on ttyUSB*/ttyACM* to weewx"));

  //  time_sync.setup();

	PRINT_FREE_MEMORY(F("setup1"));
  wdat.setup();

	PRINT_FREE_MEMORY(F("setup2"));
  iaq.setup();

	PRINT_FREE_MEMORY(F("setup2"));
  wco2.setup();

	PRINT_FREE_MEMORY(F("setup3"));

  /*
  loopc_time_sync.m_title[ws_LOOPC_COUNT] = F("loopc_time_sync");
  loopc_time_sync.m_count_max = ATTEMPTS_MAX_TIME_SYNC;
  loopc_time_sync.set_all_count_zero();
  loopc_time_sync.set_all_done_false();
  */
  
  loopc_rest.m_title[ws_LOOPC_COUNT] = F("loopc_rest");
  loopc_rest.m_count_max = 1;
  loopc_rest.set_all_count_zero();
  loopc_rest.set_all_done_false();
  
  setup_delay();
  ERROR_CATCH;
  return;
 catch_block:
  ser_println(F("error:  setup bug"));
  wexception.print();
	weather_halt();
}

void loop(void) {
	FUNC_BEGIN;
	int res = -1;
  uint32_t date_time = 0;
  wexception.clear_all();

  stopw.setup();
  stopw.start(0);
  divider_line_loop_begin();
  
	PRINT_FREE_MEMORY(F("loop_begin"));

  /*
  if (loopc_rest.finished_all() && !loopc_time_sync.finished_all()) {

    // it will pass automatically until timeout triggers it
    // then it will attempt to read from the timeserver
    res = time_sync.sync();

    // then loopc_time_sync will make ATTEMPTS_MAX_TIME_SYNC attemps before giving up
    if (res == 0) {
      loopc_time_sync.count_loopc(WDAT_PASS);
    } else {
      loopc_time_sync.count_loopc(WDAT_FAIL);
      if (loopc_time_sync.give_up()) {
        // specific to time_sync
        // unlike other situations when loopc gives up
        // we also have to reset the time_sync timeout;
        // now also loopc_time_sync will be finished_all()
        ser_println(F("-------- give up; reset timeout"));
        time_sync.reset_timeout();
      }
      // if not give_up() it will still be in timeout and attempt to sync on next loop();
    }
    if (loopc_time_sync.finished_all()) {
      loopc_time_sync.set_all_done_false();
      loopc_time_sync.set_all_count_zero();
      loopc_rest.set_all_done_false();
      loopc_rest.set_all_count_zero();
    }

  } else if (!loopc_rest.finished_all()) {
  */
  loopc_rest.set_all_done_false();
  loopc_rest.set_all_count_zero();
	
    // ==========================================
    stopw.start(1);
    divider_line_sensor(F("temp"));
    ser_println("process_upstream_sensor");
    res = iaq.advanced_read(wdat);
    ERROR_CONDITION((res == 0), F("process_upstream_sensor FAIL iaq"));
    res = wco2.advanced_read(wdat);
    ERROR_CONDITION((res == 0), F("process_upstream_sensor FAIL wco2"));
    ser_println("state wdat increment PASS");
    wdat.print_inside_all();
    stopw.stop(1, F("process_upstream_sensor TEMP"));
    // ==========================================
    stopw.start(1);
    divider_line_sensor(F("weewx"));
    ser_println("process_downstream_weewx");
    
    // see notes at beginning of file
    // date_time = time_sync.unix_epoch();
    date_time = 0;   // sentinel; impossible time;
    wdat.print_inside_all_weewx(date_time);
    stopw.stop(1, F("process_downstream_weewx"));
    // ==========================================

    // fixme 4/2/23   need to check print_inside_all_weewx() does not flood weewx_atlas driver;
    /////////	ser_println(F("delay(23000);"));
    ///////// delay(23000);
    ser_println(F("delay(0);"));

    // the rest always pass
    loopc_rest.count_loopc(WDAT_PASS);
    // fixme IMPORTANT do not swamp weewx  check this
    // delay(6500);
    // delay(2000);
  /*
  }
  */
  
  stopw.stop(0, F("whole loop"));
	PRINT_FREE_MEMORY(F("loop_end"));
  Serial.flush();
	ERROR_CATCH;
  watchdog_reset();
	return;
 catch_block:
  wexception.print();
  watchdog_reset();
}

// eee eof
