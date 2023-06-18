/*
  Modbus RTU Server for Rain Sensor


  This sketch is a Modbus RTU server for the rain sensor.
	It reads in inches:
    rain_daily
    rain_hourly
    rain_daily_until_last_hour
  and outputs them to the serial monitor.
  As a modbus server it replies to client request packets
  with reply packets of sensor data.  

  Circuit:
   - Adafruit Metro Mini dev board
   - ANMBest modbus breakout board with MAX485 chip
   - Adafruit TSL2591 breakout board; provides luminosity and IR 
   - Adafruit LTR390 breakout board; provides UV
  connect SCL to I2C Clock
  connect SDA to I2C Data
  connect Vin to 3.3-5V DC
  connect GROUND to common ground

  Circuit:
   - MKR board
   - Winners® Modbus RS485 Temperature and Humidity:
   - External 9-36 V power Supply
   - MKR 485 shield
     - ISO GND connected to GND of the Modbus RTU sensor and the Power supply V-
     - Power supply V+ connected to V+ sensor
     - Y connected to A/Y of the Modbus RTU sensor
     - Z connected to B/Z of the Modbus RTU sensor
     - Jumper positions

  Authors:
  Lady Ada;           (date unknown); light sensor code;
    https://www.adafruit.com/product/2590  metro mini dev board;
    https://www.adafruit.com/product/1980  TSL2591 light sensor breakout;
    https://www.adafruit.com/product/4831  LTR390 UV light sensor breakout;
    https://www.adafruit.com/product/3073  RFM96W LoRa radio breakout 433 MHz;
  Philippe de Craene; September 2019; modbus code;         dcphilippe@yahoo.fr
    https://create.arduino.cc/projecthub/philippedc/arduino-esp8266-rs485-modbus-anemometer-45f1d8
  Abhijit Borah       (date unknown); rain code;           
    https://www.instructables.com/Arduino-Rain-Gauge-Calibration/
  M. Suzuki           (date unknown); rain code;           msuzuki777
    https://www.instructables.com/Arduino-Weather-Station-Part3-Rain/
  Juan Bester         (date unknown); modbus code;         bester.juan@gmail.com
    http://www.zihatec.de
    https://github.com/angeloc/simplemodbusng
  William Garber;     March 2022;     edited modbus code;

	license:         GNU GPL. https://www.gnu.org/licenses/gpl.html
*/
// ctx = client transmit = request;
// crx = client receive  = reply;

#include <Wire.h>
#include <RTClib.h>
#include <weather_modbus.h>

const int RAIN_pin = 2;

const int BUCKET_TIPS_RESET = 10000;    // must fit in uint16_t
const int CHECK_TIME_SEC = 7;
// static const uint32_t DEBOUNCE_DELAY_MSEC = 50;   // fixme check msec
const uint32_t DEBOUNCE_DELAY_MSEC = 300;   // fixme check msec

class rain_volatile {
public:
	static void count_bucket_tip_ISR(void);
	void setup_pins(void);
	void setup_data(void);
	bool new_tips(void) { return (m_rain_bucket_tips > m_rain_bucket_tips_prev); }
	double rain_mm(void) { return ((float) m_rain_bucket_tips) * RAIN_HEIGHT_PER_BUCKET_TIP_MM; }
	double rain_inch(void) { return ((float) m_rain_bucket_tips) * RAIN_HEIGHT_PER_BUCKET_TIP_INCH; }
	void print_rain(void);
	copy_from(weather_data_main &w) {
		m_rain_bucket_tips = w.m_rain_bucket_tips;
		m_rain_reset       = w.m_rain_reset;
	}
	copy_to(weather_data_main &w) {
		w.m_rain_bucket_tips = m_rain_bucket_tips;
		w.m_rain_reset       = m_rain_reset;
	}
	static volatile uint32_t m_last_debounce_time_msec;
  static volatile uint16_t m_rain_bucket_tips_prev;
  static volatile uint16_t m_rain_bucket_tips;
  static volatile uint16_t m_rain_reset;
}; // class rain_volatile;

/* globals begin ----------------------------------------------- */

weather_modbus wmod;
weather_data_main wdat;
rain_volatile rdat;

volatile uint32_t rain_volatile::m_last_debounce_time_msec;
volatile uint16_t rain_volatile::m_rain_bucket_tips_prev;
volatile uint16_t rain_volatile::m_rain_bucket_tips;
volatile uint16_t rain_volatile::m_rain_reset;

RTC_Millis rtc;
stopwatch stopw;

/* globals end ----------------------------------------------- */

// reading millis() once is OK, you get the value that was set when the interrupt occurs.
// But as millis() requires interrupts as well to get updated,
// you can't rely on millis() changing whilst executing the ISR.

// interrupt service routine (ISR) 
// assigned to the interrupt for RAIN_pin by rain_setup_pins()
void rain_volatile::count_bucket_tip_ISR(void) {
	uint32_t time_msec = millis();
	// fixme if time wraps around (overflows) uint32_t  
	// it should still work
	if (time_msec - m_last_debounce_time_msec > DEBOUNCE_DELAY_MSEC) {
		m_rain_bucket_tips++;
		m_last_debounce_time_msec = time_msec;
		if (m_rain_bucket_tips >= BUCKET_TIPS_RESET) {
			// set m_rain_reset to false after reporting to modbus client
			m_rain_reset = true;
			m_rain_bucket_tips = 0;
			m_rain_bucket_tips_prev = 0;
		}
	}
}

void rain_volatile::setup_pins(void) {
  pinMode(RAIN_pin, INPUT);
  m_rain_bucket_tips = 0;
  m_rain_bucket_tips_prev = 0;
  m_rain_reset = true;
  int interrupt = digitalPinToInterrupt(RAIN_pin);
  attachInterrupt(interrupt, count_bucket_tip_ISR, CHANGE);
}

void rain_volatile::setup_data(void) {
	m_last_debounce_time_msec = 0;
  m_rain_bucket_tips_prev = 0;
  m_rain_bucket_tips = 0;
  m_rain_reset = true;
	rtc.begin(DateTime(__DATE__, __TIME__));
}

void rain_volatile::print_rain(void) {
  DateTime now = rtc.now();
	int change = m_rain_bucket_tips - m_rain_bucket_tips_prev;
	Serial.print(now.hour());
	Serial.print(F(":"));
	Serial.print(now.minute());
	Serial.print(F(" since last change "));
  Serial.print(F(" bucket tips_prev = ")); Serial.print(m_rain_bucket_tips_prev);
  Serial.print(F(" bucket tips = "));      Serial.print(m_rain_bucket_tips);
  Serial.print(F(" change = "));           Serial.print(change);
	Serial.print(F(" total rain inch = "));       Serial.print(rain_inch(), 8);
	Serial.println();
}

void setup(void) {
	int res = -1;
  rdat.setup_pins();
  rdat.setup_data();
	
	wmod.setup_communication();
	wmod.setup_errors();

	dtoggle.clear_all();
	dtoggle.set(DISPLAY_FATAL);
	//	dtoggle.set(DISPLAY_COMPARE_CRC);
	//	dtoggle.set(DISPLAY_SENSOR_PRINT);
	dtoggle.set(DISPLAY_PRINT_WIND);
	dtoggle.set(DISPLAY_PRINT_TEMP_OUT);
	dtoggle.set(DISPLAY_PRINT_LIGHT);
	dtoggle.set(DISPLAY_PRINT_RAIN);
	dtoggle.set(DISPLAY_PRINT_DONE);
	//	dtoggle.set(DISPLAY_SENSOR_WRITE);
	//	dtoggle.set(DISPLAY_SENSOR_READ);
	//	dtoggle.set(DISPLAY_CLIENT_TX_RX);
	//	dtoggle.set(DISPLAY_SERVER_RX_TX);
	dtoggle.set(DISPLAY_CRC_DEBUG);
	dtoggle.set(DISPLAY_TIMING);
  dtoggle.set(DISPLAY_PRINT_WEATHER_DATA_MAIN);
  dtoggle.set(DISPLAY_PRINT_WEATHER_DATA_SATELLITE);
	dtoggle.set(DISPLAY_PRINT_TEMP_IN);

	//  dtoggle.set(DISPLAY_RADIO_UNPACK_WORD);
	//  dtoggle.set(DISPLAY_RADIO_UNPACK_HEADER);
	//  dtoggle.set(DISPLAY_RADIO_UNPACK_TERM);
	//  dtoggle.set(DISPLAY_RADIO_PACK_WEEWX_FRAME);
	//  dtoggle.set(DISPLAY_RADIO_PACK_RADIO_FRAME);
	//  dtoggle.set(DISPLAY_RADIO_UNPACK_RADIO_FRAME);
  dtoggle.set(DISPLAY_RADIO_CHECK_RADIO_FRAME_MAX);
	//  dtoggle.set(DISPLAY_RADIO_PRINT_FRAME_TO_SERIAL);
	//  dtoggle.set(DISPLAY_RADIO_TRANSMIT_THEN_ACK);
	//  dtoggle.set(DISPLAY_RADIO_RECEIVE_THEN_ACK);

	stopw.setup(dtoggle.get(DISPLAY_TIMING));
	
	Serial.println(F("Modbus Server"));
	Serial.println(F("Rain Sensor"));
	wmod.setup_buffers();
	wmod.crc_debug();
	wdat.set_all_data_zero();
  // slow in starting the serial monitor
  delay(4000);
}

bool rain_reading_done = false;

void loop(void) {
	int res = -1;
	weather_exception e;

	stopw.start(0);
	if (dtoggle.get(DISPLAY_TIMING)) {
		Serial.println(F("============================================================ LOOP BEGIN"));
	}
	check_halt();

	// want to spend as much time checking for rain as possible; do not miss bucket tip;
	// only print changes;
	stopw.start(1);

	if (rdat.new_tips() || rdat.m_rain_reset) {
		stopw.start(2);
		Serial.println();
		if (rdat.m_rain_reset) {
			Serial.println(F("rain reset"));
		} else {
			Serial.println(F("new tips"));
		}
		rdat.print_rain();
		rdat.m_rain_bucket_tips_prev = rdat.m_rain_bucket_tips;
		rain_reading_done = false;
		stopw.stop(2, F("count print"));
	}
	
	if (!rain_reading_done) {
		stopw.start(2);
		Serial.println();
		Serial.println(F("============================== pack_rain begin"));
		rdat.copy_to(wdat);
		// pack m_frame_array[frame_idx_O]
		wmod.pack_rain(wdat);
		rain_reading_done = true;
		stopw.stop(2, F("pack_rain"));
	}

	stopw.stop(1, F("sensor all"));
//	Serial.println(F("============================== server_rx_tx(rain) begin"));

	stopw.start(1);
  // int tr = 1000;   // before 4/19/22;
  int tr = 0;
	res = wmod.server_rx_tx(ws_RAIN, tr, e);
	e.print_error_codes();
	if (res == 0) {
		rdat.m_rain_reset = false;
		Serial.println(F("   success; sent"));
	} 
	stopw.stop(1, F("server_rx_tx"));
	stopw.stop(0, F("================================ whole loop"));
}
// eee eof
