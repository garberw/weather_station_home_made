// -*- mode: C++ -*-
/*
  Radio Receiver and Indoor Temp Sensor

  This sketch is a radio receiver for weather sensors.
  It receives the sensor values over LoRa radio.
  It outputs them to the usb port
  where they are read by the weewx_wmod driver.
  They can be read directly by 'cat /dev/ttyUSBwi' for example.

  Example sketch showing how to create a simple messaging client (receiver)
  with the RH_RF95 class. RH_RF95 class does not provide for addressing or
  reliability, so you should only use RH_RF95 if you do not need the higher
  level messaging abilities.
*/

#include <weather_lib_util.h>
#include <weather_lib_timer.h>
#include <weather_lib_counter.h>
#include <weather_lib_radio.h>

#include <weather_wra_bme688.h>
#include <weather_wra_pas_co2.h>

/* macros begin ----------------------------------------------- */

#define HPASCAL_PER_PASCAL (0.01)
#define KOHM_PER_OHM (0.001)

// fixme this should be the only thing you need to modify for the summer
// #define STATION_WMOD_OUTSIDE_RUNNING false
#define STATION_WMOD_OUTSIDE_RUNNING true

// #define BLINK_LED true

// ATTEMPTS_WDAT_RADIO_MAX=20 was too low; took about 40 seconds;
// takes 40 seconds for client to complete a full cycle with client loop delay(5000);
// ATTEMPTS_WDAT_RADIO_MAX=50 should have at least two cycles;
#define ATTEMPTS_MAX_UPSTREAM_RADIO 50
// should almost always work
#define ATTEMPTS_MAX_UPSTREAM_SENSOR 5

/* macros end ----------------------------------------------- */
/* globals begin ----------------------------------------------- */

weather_radio wrad;
weather_data wdat;
weather_timer stopw;
loop_counter_single loopc_downstream;
loop_counter_outside loopc_outside;
loop_counter_inside loopc_inside;
weather_bme688 iaq;
weather_pas_co2 wco2;

/* globals end ----------------------------------------------- */
// fixme check this;    ??????????????????
// int res = iaq.advanced_read(wdat);

void setup(void) {
  setup_communication_ser0();
  // while (!Serial);
  watchdog_setup();
  wexception.clear_all();
  ser_setup();
  FUNC_BEGIN;
  watchdog_reset();

  stopw.setup();
  stopw.start(0);
  ser_println(F("Radio Receiver"));
  ser_println(F("receive from these 4 Modbus Servers:"));
  ser_println(F("(1) Sensor Temp and Humidity"));
  ser_println(F("(2) Sensor Wind Speed and Direction"));
  ser_println(F("(3) Sensor Light and UV"));
  ser_println(F("(4) Sensor Rain"));
  ser_println(F("send output on usb to weewx"));
  Serial.flush();

  wdat.setup();
  watchdog_reset();

  loopc_downstream.m_title[ws_LOOPC_COUNT] = F("loopc_downstream");
  loopc_downstream.m_count_max = 1;
  loopc_downstream.set_all_count_zero();
  loopc_downstream.set_all_done_false();
	
  loopc_outside.m_count_max = ATTEMPTS_MAX_UPSTREAM_RADIO;
  loopc_outside.set_all_count_zero();
  loopc_outside.set_all_done_false();
	
  loopc_inside.m_count_max = ATTEMPTS_MAX_UPSTREAM_SENSOR;
  loopc_inside.set_all_count_zero();
  loopc_inside.set_all_done_false();

  wrad.setup();
	
  iaq.setup();

  wco2.setup();

  PRINT_FREE_MEMORY(F("setup"));
  
  setup_delay();

  watchdog_reset();
  stopw.stop(0, F("setup end"));
  ERROR_CATCH;
  return;
 catch_block:
  watchdog_reset();
  stopw.stop(0, F("setup end"));
  ser_println(F("error:  setup bug"));
  wexception.print();
  weather_halt();
}

void process_downstream_weewx(void) {
  FUNC_BEGIN;
  /*
  ser_println(F("-------- call pack_weewx_frame"));
  wrad.pack_weewx_frame(wdat, loopc_outside, loopc_inside);
  ser_println(F("-------- call transmit_weewx_frame"));
  wrad.transmit_weewx_frame();
  */
  int date_time = 0;
  wdat.print_inside_outside_all_weewx(date_time, loopc_outside, loopc_inside);
  if (true) {
    ser_println(F("-------- stage loopc_downstream PASS"));
    loopc_downstream.count_loopc(WDAT_PASS);
  } else {
    ser_println(F("-------- stage loopc_downstream FAIL"));
    loopc_downstream.count_loopc(WDAT_FAIL);
  }
  if (loopc_downstream.finished_all()) {
    ser_println(F("-------- stage loopc_downstream.finished_all"));
    wdat.set_all_data_zero();
    loopc_inside.set_all_done_false();
    loopc_inside.set_all_count_zero();
    loopc_outside.set_all_done_false();
    loopc_outside.set_all_count_zero();
    loopc_downstream.set_all_done_false();
    loopc_downstream.set_all_count_zero();
  }
}

void process_upstream_radio(void) {
  FUNC_BEGIN;
  int res = -1;
  ser_println(F("wrad.receive_then_ack"));
  res = wrad.receive_then_ack();
  if (res == 0) {
    ser_println(F("-------- stage loopc_outside.increment PASS"));
    ser_println(F("-------- unpack_radio_frame"));
    wrad.unpack_radio_frame(wdat, loopc_outside);
    ser_println(F("-------- print_outside_all"));
    wdat.print_outside_all1();
    ser_println(F("-------- print_inside_all"));
    wdat.print_inside_all();
    ser_println(F("-------- loopc_outside.print_done"));
    loopc_outside.print_done();
    ser_println(F("-------- loopc_inside.print_done"));
    loopc_inside.print_done();
    ser_println(F("-------- stage loopc_outside.increment PASS"));
    loopc_outside.increment(ws_WIND, WDAT_PASS);
    loopc_outside.increment(ws_TEMP, WDAT_PASS);
    loopc_outside.increment(ws_LIGH, WDAT_PASS);
    loopc_outside.increment(ws_RAIN, WDAT_PASS);
  } else {
    ser_println(F("-------- stage loopc_outside.increment FAIL"));
    loopc_outside.increment(ws_WIND, WDAT_FAIL);
    loopc_outside.increment(ws_TEMP, WDAT_FAIL);
    loopc_outside.increment(ws_LIGH, WDAT_FAIL);
    loopc_outside.increment(ws_RAIN, WDAT_FAIL);
  }
}

void process_upstream_sensor(format_string name) {
  FUNC_BEGIN;
  int res1 = -1;
  int res2 = -1;
  divider_line_sensor(name);
  // stopw.start(2);
  res1 = iaq.advanced_read(wdat);
  ser_print("iaq return res1 = "); ser_println(res1);
  // stopw.stop(2, F("advanced_read_temp"));
  // stopw.start(2);
  res2 = wco2.advanced_read(wdat);
  ser_print("wco2 return res2 = "); ser_println(res2);
  // stopw.stop(2, F("advanced_read_co2"));
  if ((res1 == 0) && (res2 == 0)) {
    ser_println(F("-------- stage loopc_inside.increment PASS"));
    // stopw.start(2);
    wdat.print_inside_all();
    loopc_inside.increment(ws_INSIDE_TEMP, WDAT_PASS);
    // stopw.stop(2, F("print"));
  } else {
    ser_println(F("-------- stage loopc_inside.increment FAIL"));
    loopc_inside.increment(ws_INSIDE_TEMP, WDAT_FAIL);
  }
  return;
}

// either measure sensors or receive radio or transmit weewx once per loop but not more than one
void loop(void) {
  FUNC_BEGIN;
  int res = -1;
  wexception.clear_all();
  stopw.setup();
  stopw.start(0);
  divider_line_loop_begin();
  
  PRINT_FREE_MEMORY(F("loop_begin"));
  
  if (loopc_inside.finished_all()) {
    //		if (true) {
    if (!STATION_WMOD_OUTSIDE_RUNNING ||
        (loopc_outside.finished_all() && !loopc_downstream.finished_all())) {
      ser_println(F("======== stage 3 loopc_outside.finished_all && !loopc_downstream.finished_all"));
      ser_println(F("======== ... stage call process_downstream_weewx"));
      stopw.start(1);
      process_downstream_weewx();
      stopw.stop(1, F("process_downstream_weewx"));
    } else if (!loopc_outside.finished_all()) {
      ser_println(F("======== stage 2 !loopc_outside.finished_all; call process_upstream_radio"));
      stopw.start(1);
      process_upstream_radio();
      stopw.stop(1, F("process_upstream_radio"));
    }
  } else if (!loopc_inside.finished_temp()) {
    ser_println(F("======== stage 1 !finished_temp; call process_upstream_sensor TEMP"));
    stopw.start(1);
    process_upstream_sensor(F("temp"));
    stopw.stop(1, F("process_upstream_sensor TEMP"));
  }
  // delay(100); // commented out 4/19/22
  delay(1000); // added 10/2/22;
  
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
