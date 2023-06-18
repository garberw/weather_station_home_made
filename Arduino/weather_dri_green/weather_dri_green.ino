// wmod Indoor Temp Sensor
/* headers begin ----------------------------------------------- */

#include <weather_lib_util.h>
#include <weather_lib_timer.h>
#include <weather_lib_data.h>
#include <weather_wra_bme688.h>
#include <weather_wra_pas_co2.h>

/* headers end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

weather_bme688 iaq;
weather_pas_co2 wco2;
weather_data wdat;
weather_timer stopw;

/* globals end ----------------------------------------------- */

void setup(void) {
  setup_communication_ser0();
  watchdog_setup();
	wexception.clear_all();
  ser_setup();
	FUNC_BEGIN;
  
  stopw.setup();
  ser_println(F("Sensor BME688 Temperature; Humidity; Pressure; Gas Resistance;"));
  ser_println(F("send output on ttyUSB*/ttyACM* to weewx"));
  Serial.flush();

  wdat.setup();
  iaq.setup();
  wco2.setup();

	PRINT_FREE_MEMORY(F("setup"));
	
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
	ser_println(F("RTC_NONE def"));
  stopw.setup();
  stopw.start(0);
  divider_line_loop_begin();
  
	PRINT_FREE_MEMORY(F("loop_begin"));
	
  // ==========================================
  stopw.start(1);
  divider_line_sensor(F("temp"));
  ser_println(F("process_upstream_sensor"));
  res = iaq.advanced_read(wdat);
  ERROR_CONDITION((res == 0), F("process_upstream_sensor FAIL iaq"));
  res = wco2.advanced_read(wdat);
  ERROR_CONDITION((res == 0), F("process_upstream_sensor FAIL wco2"));
  ser_println(F("state wdat increment PASS"));
  wdat.print_inside_all();
  stopw.stop(1, F("process_upstream_sensor TEMP"));
  // ==========================================
  stopw.start(1);
  divider_line_sensor(F("weewx"));
  ser_println(F("process_downstream_weewx"));
	date_time = 0;
  wdat.print_inside_all_weewx(date_time);
  stopw.stop(1, F("process_downstream_weewx"));
  // ==========================================
  // delay(100);

	ser_println(F("SMALL_RUN_LOOP_DELAY def"));
	// fixme 4/2/23   need to check print_inside_all_weewx() does not flood weewx_green driver;
	///////// ser_println(F("delay(5000);"));
  ///////// delay(5000);
	ser_println(F("delay(0);"));
  delay(0);

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
