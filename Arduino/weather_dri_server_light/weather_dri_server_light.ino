/*
  Modbus RTU Server for Light Sensor

  This sketch is a Modbus RTU server for light (luminosity IR and UV) sensors.
  It reads the light values and outputs them to the serial monitor.
  As a modbus server it replies to client request packets
  with reply packets of light sensor data.  

	- Adafruit TSL2591 breakout board; provides luminosity and IR 
	- Adafruit LTR390 breakout board; provides UV
*/

#include <weather_lib_util.h>
#include <weather_lib_timer.h>
#include <weather_lib_counter.h>
#include <weather_lib_modbus.h>

#include <weather_wra_tsl2591.h>
#include <weather_wra_ltr390.h>
#include <weather_wra_tsys.h>

/* macros begin ----------------------------------------------- */

#define ATTEMPTS_MAX_DOWNSTREAM_MODBUS 20
#define ATTEMPTS_MAX_UPSTREAM_SENSOR 5

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

weather_modbus wmod;
weather_data wdat;
weather_tsl2591 weather_tsl;
weather_ltr390 weather_ltr;
weather_timer stopw;
loop_counter_single loopc_downstream;
loop_counter_outside loopc_outside;
weather_tsys wtsys;

/* globals end ----------------------------------------------- */

void setup(void) {
  setup_communication_ser0();
  watchdog_setup();
  setup_communication_ser1();
	wexception.clear_all();
  ser_setup();
	FUNC_BEGIN;
  
	int res = -1;
	wmod.setup_frame_array();
	wexception.clear_all();
	stopw.setup();
  ser_println(F("Modbus Server"));
	ser_println(F("Light Sensor"));
  wmod.setup_buffers();
	wmod.crc_debug();

  wdat.setup();
  
	loopc_downstream.m_title[ws_LOOPC_COUNT] = F("loopc_downstream");
	loopc_downstream.set_all_count_zero();
	loopc_downstream.set_all_done_false();
	loopc_downstream.m_count_max = ATTEMPTS_MAX_DOWNSTREAM_MODBUS;
	
	loopc_outside.m_count_max = ATTEMPTS_MAX_UPSTREAM_SENSOR;
	loopc_outside.set_all_count_zero();
	loopc_outside.set_all_done_false();
	loopc_outside.m_done[ws_LIGH] = false;
	wtsys.setup();
	
	weather_tsl.setup();
	weather_ltr.setup();

	PRINT_FREE_MEMORY(F("setup"));

	setup_delay();
	ERROR_CATCH;
	return;
 catch_block:
	ser_println(F("error:  setup bug"));
	wexception.print();
	weather_halt();
}

void process_downstream_modbus(format_string name, device_idx ws, int timeout_read) {
	FUNC_BEGIN;
	int res = -1;
	// sensor data stored in m_frame[frame_idx_O]
	// should not be overwritten
	divider_line_sensor(name);
	// stopw.start(2);
	wmod.pack_sensor(ws, wdat);
	// stopw.stop(2, F("pack_sensor"));
	// stopw.start(2);
	res = wmod.server_rx_tx(ws, timeout_read);
	// stopw.stop(2, F("server_rx_tx"));
  if (res == 0) {
		loopc_downstream.count_loopc(WDAT_PASS);
  } else {
		loopc_downstream.count_loopc(WDAT_FAIL);
	}
	if (loopc_downstream.finished_all()) {
		wdat.set_all_data_zero();
		loopc_outside.set_all_done_false();
		loopc_outside.set_all_count_zero();
		loopc_downstream.set_all_done_false();
		loopc_downstream.set_all_count_zero();
		ser_println(F(" start reading sensors again;"));
	}
}

void process_upstream_sensor(format_string name, device_idx ws) {
	FUNC_BEGIN;
	// loop counter to make sure this is not stale after several failed reads;
	// stopw.start(2);
	divider_line(F("="), 30); ser_println(); ser_println(F("advanced_read_light begin"));
	weather_tsl.advanced_read(wdat);
	// stopw.stop(2, F("advanced_read_light"));
	// stopw.start(2);
	divider_line(F("="), 30); ser_println();	ser_println(F("advanced_read_UV begin"));
	weather_ltr.advanced_read(wdat);
	// stopw.stop(2, F("advanced_read_UV"));
	// stopw.start(2);
	// divider_line(F("="), 30); ser_println(); ser_println(F("pack_light begin"));
	// pack m_frame_array[frame_idx_O]
	// wmod.pack_sensor(ws, wdat);
	wdat.print_outside_sensor(ws);
	// stopw.stop(2, F("pack print"));
	if (true) {
		loopc_outside.increment(ws, WDAT_PASS);
	} else {
		loopc_outside.increment(ws, WDAT_FAIL);
	}
}

void process_upstream_tsys(format_string name, device_idx ws) {
	FUNC_BEGIN;
  divider_line(F("="), 30); ser_println(); ser_println(F("advanced_read_tsys begin"));
  wtsys.advanced_read_light(wdat);
  // wmod.pack_tsys(ws, wdat);
  wdat.print_outside_tsys(ws);
  if (true) {
    loopc_outside.increment(ws_TSYS, WDAT_PASS);
  } else {
    loopc_outside.increment(ws_TSYS, WDAT_FAIL);
  }
}

// REQUIRE:  loop() takes (time ~ weather_modbus::TIMEOUT_READ)
// and completes within weather_modbus::TIMEOUT_CLIENT

// fixme
// const int loop_max = 5;;
// int loop_count = 0;
// bool light_reading_done = false;

void loop() {
	FUNC_BEGIN;
  int res = -1;
	wexception.clear_all();
	stopw.setup();
	stopw.start(0);
	divider_line_loop_begin();

	PRINT_FREE_MEMORY(F("loop_begin"));
	
  // int tr_light = 0;  // commented out 4/19/22
  // int tr_light = 200;   // until 10/19/22
  int tr_light = 5000;   // until 10/19/22

	if (loopc_outside.finished_light() && loopc_outside.finished_tsys() && !loopc_downstream.finished_all()) {
		stopw.start(1);
    ser_println(F("======== stage 2 process_downstream_modbus"));
		process_downstream_modbus(F("light"), ws_LIGHT, tr_light);
		stopw.stop(1, F("process_downstream_modbus LIGHT"));
  } else if (!loopc_outside.finished_light()) {
		stopw.start(1);
    ser_println(F("======== stage 1A process_upstream_sensor LIGHT"));
		process_upstream_sensor(F("light"), ws_LIGHT);
		stopw.stop(1, F("process_upstream_sensor LIGHT"));
	} else if (!loopc_outside.finished_tsys()) {
		stopw.start(1);
    ser_println(F("======== stage 1B process_upstream_tsys"));
		process_upstream_tsys(F("tsys"), ws_LIGHT);
		stopw.stop(1, F("process_upstream_tsys TSYS"));
	}

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
