/*
  Modbus RTU Weather Client

  This sketch is a Modbus RTU client.
  It reads from Modbus RTU servers (weather sensors) and outputs them to the serial monitor.
  The sensors include:

  temperature and humidity 
  wind
  light and UV
  rain

  It also uses LoRa radio to transmit (TX) them to the receiving (RX)
  arduino radio inside the house.  
  This RX arduino is hooked up by usb to a computer running weewx.
  (fixme specify weewx driver).
*/
// ctx = client transmit = request;
// crx = client receive  = reply;

// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

/* headers begin ----------------------------------------------- */

#include <weather_lib_util.h>
#include <weather_lib_timer.h>
#include <weather_lib_counter.h>
#include <weather_lib_modbus.h>
#include <weather_lib_radio.h>
#include <weather_wra_tsys.h>

/* headers end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

weather_modbus wmod;
weather_data wdat;
weather_radio wrad;
weather_timer stopw;
loop_counter_single loopc_downstream;
loop_counter_outside loopc_outside;
weather_tsys wtsys;

#define ATTEMPTS_MAX_UPSTREAM_MODBUS 20
#define ATTEMPTS_MAX_DOWNSTREAM_RADIO 20

/* globals end ----------------------------------------------- */

void setup() {
  setup_communication_ser0();
  watchdog_setup();
  setup_communication_ser1();
	wexception.clear_all();
  ser_setup();
	FUNC_BEGIN;
  
  int res = -1;
	wmod.setup_frame_array();
	stopw.setup();
  ser_println(F("Modbus Client and Radio Transmitter"));
  ser_println(F("receive from these 4 Modbus Servers:"));
	ser_println(F("(1) Sensor Temp and Humidity"));
  ser_println(F("(2) Sensor Wind Speed and Direction"));
  ser_println(F("(3) Sensor Light and UV"));
  ser_println(F("(4) Sensor Rain"));
	Serial.flush();
  wmod.setup_buffers();
	wmod.crc_debug();

	wdat.setup();

	loopc_downstream.m_title[ws_LOOPC_COUNT] = F("loopc_downstream");
	loopc_downstream.set_all_count_zero();
	loopc_downstream.set_all_done_false();
	loopc_downstream.m_count_max = ATTEMPTS_MAX_DOWNSTREAM_RADIO;

	loopc_outside.m_count_max = ATTEMPTS_MAX_UPSTREAM_MODBUS;
	loopc_outside.set_all_count_zero();
	loopc_outside.set_all_done_false();
	wtsys.setup();

  wrad.setup();

	PRINT_FREE_MEMORY(F("setup"));
	
	setup_delay();
	ERROR_CATCH;
	return;
 catch_block:
	ser_println(F("error:  setup bug"));
	wexception.print();
	weather_halt();
}

void process_downstream_radio(void) {
	FUNC_BEGIN;
	int res = -1;
	divider_line(F("C"), 55); ser_println();
	ser_println(F("radio; send the complete packet;"));
	divider_line(F("C"), 55); ser_println();
	loopc_outside.print_done();
	// stopw.start(2);
	wrad.pack_radio_frame(wdat, loopc_outside);
	// stopw.stop(2, F("pack_radio_frame"));
	// stopw.start(2);
	res = wrad.transmit_then_ack();
	// stopw.stop(2, F("transmit_then_ack"));
	if (res == 0) {
		loopc_downstream.count_loopc(WDAT_PASS);
		divider_line(F("C"), 55); ser_println();
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

void process_upstream_modbus(format_string name, device_idx ws, int timeout_read) {
	FUNC_BEGIN;
	int res = -1;
	divider_line_sensor(name);
	// stopw.start(2);
	res = wmod.client_tx_rx(ws, timeout_read);
	// stopw.stop(2, F("client_tx_rx"));
  if (res == 0) {
		// stopw.start(2);
		wmod.unpack_sensor(ws, wdat);
		// wmod.unpack_tsys(ws, wdat);
		wdat.print_outside_sensor(ws);
		wdat.print_outside_tsys(ws);
		// stopw.stop(2, F("unpack print"));
		loopc_outside.increment(ws, WDAT_PASS);
	} else {
		loopc_outside.increment(ws, WDAT_FAIL);
	}
}

void process_upstream_tsys(format_string name, device_idx ws) {
	FUNC_BEGIN;
  divider_line(F("="), 30); ser_println(); ser_println(F("advanced_read_tsys begin"));
  wtsys.advanced_read_client(wdat);
  // no need to pack or unpack; it is already in wdat;
  wdat.print_outside_tsys_client();
  if (true) {
    loopc_outside.increment(ws_TSYS, WDAT_PASS);
  } else {
    loopc_outside.increment(ws_TSYS, WDAT_FAIL);
  }
}

/*
// values prior to 10/7/22
int tr_wind = 75;
int tr_temp = 320;
int tr_light = 1200;
int tr_rain = 600;
*/
// sensor gives up after several bad readings; send incomplete packet;
// to speed up light decrease integration time in configureSensor();
// tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
void loop() {
	FUNC_BEGIN;
  int res = -1;
	wexception.clear_all();
	stopw.setup();
	stopw.start(0);
	divider_line_loop_begin();

	PRINT_FREE_MEMORY(F("loop_begin"));
	
	// uncomment to not use sensor
	loopc_outside.m_done[ws_WIND] = true;
	// loopc_outside.m_done[ws_TEMP] = true;
	// loopc_outside.m_done[ws_LIGH] = true;
	// loopc_outside.m_done[ws_RAIN] = true;
	// loopc_outside.m_done[ws_TSYS] = true;
  int tr_wind = 5000;
  int tr_temp = 2000;
  int tr_light = 2000;
  int tr_rain = 2000;

	// !(radio done) iff (loopc_outside.finished_all());
	// "else if" ensures we either send radio or measure sensors once per loop but not both
	if (loopc_outside.finished_all() && !loopc_downstream.finished_all()) {
		stopw.start(1);
		ser_println(F("======== stage 2 process_downstream_radio ALL SENSORS"));
		process_downstream_radio();
		stopw.stop(1, F("process_downstream_radio ALL SENSORS"));
	} else if (!loopc_outside.finished_wind()) {
		stopw.start(1);
		ser_println(F("======== stage 1A process_upstream_modbus WIND"));
		process_upstream_modbus(F("wind"), ws_WIND, tr_wind);
		stopw.stop(1, F("process_upstream_modbus WIND"));
  } else if (!loopc_outside.finished_temp()) {
		stopw.start(1);
		ser_println(F("======== stage 1B process_upstream_modbus TEMP"));
		process_upstream_modbus(F("temp"), ws_TEMP, tr_temp);
		stopw.stop(1, F("process_upstream_modbus TEMP"));
  } else if (!loopc_outside.finished_light()) {
		stopw.start(1);
		ser_println(F("======== stage 1C process_upstream_modbus LIGHT"));
		process_upstream_modbus(F("light"), ws_LIGHT, tr_light);
		stopw.stop(1, F("process_upstream_modbus LIGHT"));
  } else if (!loopc_outside.finished_rain()) {
		stopw.start(1);
		ser_println(F("======== stage 1D process_upstream_modbus RAIN"));
		process_upstream_modbus(F("rain"), ws_RAIN, tr_rain);
		stopw.stop(1, F("process_upstream_modbus RAIN"));
  } else if (!loopc_outside.finished_tsys()) {
		stopw.start(1);
		ser_println(F("======== stage 1E process_upstream_tsys TSYS"));
		process_upstream_tsys(F("tsys"), ws_TSYS);
		stopw.stop(1, F("process_upstream_sensor TSYS"));
	}
  // delay(100);  // commented out 4/19/22
	delay(0);    // added 10/8/22

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
