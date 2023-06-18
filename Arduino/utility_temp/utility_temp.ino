// globals begin ----------------------------------------------------

const long BAUD = 9600;

uint32_t T15_usec;
uint32_t T35_usec;

/*
// RE and DE on MAX485 chip
const int RTS_pin = 5;
// RO on MAX485 chip
const int RX_pin = 6;
// DI on MAX485 chip
const int TX_pin = 7;
*/

const int RS485Transmit = HIGH;
const int RS485Receive  = LOW;

const int FRAME_MAX = 128;

const int TARGET_ADDRESS = 2;

// { 0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B };
// { 0x02, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x38 };
// { 0x03, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC5, 0xE9 };
// { 0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x5E };

// 2 readings (temp)(humidity);
// 2 bytes each; 4 effective bytes;
byte req_dat[] = { 0x02, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x38 };
byte rep_dat[] = { 0x01, 0x03, 0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

byte req_addr[] = { 0xFA, 0x03, 0x00, 0x64, 0x00, 0x02, 0x90, 0x5F };
byte rep_addr[] = { 0x01, 0x03, 0x02, 0x55, 0x3C, 0x00, 0x01, 0x3A, 0x79 };

byte req_addr_set[] = { 0x01, 0x06, 0x00, 0x66, 0x00, 0x02, 0xE8, 0x14 };
byte rep_addr_set[] = { 0x01, 0x06, 0x00, 0x66, 0x00, 0x02, 0xE8, 0x27 };

/*
byte req_baud[] = { 0x01, 0x03, 0x07, 0xD1, 0x00, 0x01, 0xD5, 0x47 };
byte rep_baud_4800[] = { 0x01, 0x03, 0x02, 0x00, 0x01, 0x79, 0x84 };  // 1 == 4800;
byte rep_baud_9600[] = { 0x01, 0x03, 0x02, 0x00, 0x02, 0x39, 0x85 };  // 2 == 9600;
const int rep_baud_max = sizeof(rep_baud_4800);
byte rep_baud[rep_baud_max];
							
byte req_baud_set_4800[] = { 0x01, 0x06, 0x07, 0xD1, 0x00, 0x01, 0x19, 0x47 };  // 1 == 4800;
byte rep_baud_set_4800[] = { 0x01, 0x06, 0x02, 0x00, 0x01, 0x79, 0x48 };

byte req_baud_set_9600[] = { 0x01, 0x06, 0x07, 0xD1, 0x00, 0x02, 0x59, 0x46 };  // 2 == 9600;
byte rep_baud_set_9600[] = { 0x01, 0x06, 0x02, 0x00, 0x02, 0x39, 0x49 };
*/

const int req_dat_max      = sizeof(req_dat);
const int rep_dat_max      = sizeof(rep_dat);
const int req_addr_max     = sizeof(req_addr);
const int rep_addr_max     = sizeof(rep_addr);
const int req_addr_set_max = sizeof(req_addr_set);
const int rep_addr_set_max = sizeof(rep_addr_set);
/*
const int req_baud_max          = sizeof(req_baud);
const int rep_baud_4800_max     = sizeof(rep_baud_4800);
const int rep_baud_9600_max     = sizeof(rep_baud_9600);
const int req_baud_set_4800_max = sizeof(req_baud_set_4800);
const int rep_baud_set_4800_max = sizeof(rep_baud_set_4800);
const int req_baud_set_9600_max = sizeof(req_baud_set_9600);
const int rep_baud_set_9600_max = sizeof(rep_baud_set_9600);
*/

// SoftwareSerial RS485Serial(RX_pin, TX_pin);
#define RS485Serial Serial1
char buf[200];
bool initialized = false;



// globals end ----------------------------------------------------

void halt(void) {
	Serial.println("halting");
	Serial.println("flushing buffers");
	Serial.flush();
	while (1) {
		delay(1000);
	}
}

void print_buf(const char *msg, const byte input[], int MAX) {
  sprintf(buf, msg); Serial.print(buf);
  for(int i = 0; i < MAX; i++) {
    sprintf(buf, " %02x", input[i]); Serial.print(buf);
  }
  Serial.println();
}

void calc_timeout(void) {
	// fixme
  const int bits_per_packet = 13;
  bool low_latency = false;
  if (BAUD == 1000000 && low_latency) {
    T15_usec = 1;
    T35_usec = 10;
  } else if (BAUD >= 115200 && low_latency){
    T15_usec = 75;
    T35_usec = 175;
  } else if (BAUD > 19200) {
    T15_usec = 750;
    T35_usec = 1750;
  } else {
    /*
    T15_usec = 15000000/BAUD; // 1T * 1.5 = T1.5
    T35_usec = 35000000/BAUD; // 1T * 3.5 = T3.5
    */
    // fixme these are minimums; try slightly larger;
    static const float nudge = 1.0;
    float char_per_sec = ((float) BAUD) / ((float) bits_per_packet);
    float T_usec = 1.0e6 / char_per_sec;
    T15_usec = ((uint32_t) (1.5 * nudge * T_usec));
    T35_usec = ((uint32_t) (3.5 * nudge * T_usec));
  }
}

/*
  omit the last 2 CRC bytes in the calculation; len = max - 2;
  Compute the MODBUS RTU CRC of input[0]...input[len-1]  len bytes;
  measured or read (input[len] = mH) and (input[len+1] = mL);
  calculated       cHL;
*/
int ModRTU_CRC(byte input[], int MAX) {
  int len = MAX - 2;
  if (len < 1) {
    Serial.print("error len too small; len = "); Serial.println(len);
  }
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)input[pos];          // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else {                        // Else LSB is not set
        crc >>= 1;                    // Just shift right
      }
    }
  }
  // crc has low and high bytes swapped
  uint16_t mHL = word(input[len], input[len + 1]);
  uint16_t cHL = word(crc & 0xFF, crc >> 8);
  if (cHL != mHL) {
    sprintf(buf, "crc error\n"); Serial.print(buf);
    sprintf(buf, "cHL=%04x\n", cHL); Serial.print(buf);
    sprintf(buf, "mHL=%04x\n", mHL); Serial.print(buf);
    print_buf("checksum input", input, MAX);
    return -1;
  }
  return 0;
}

int crc_debug(void) {
  int res = 0;
  Serial.println("crc_debug begin");
  res |= ModRTU_CRC(req_dat, req_dat_max);
	// no rep_dat
	res |= ModRTU_CRC(req_addr, req_addr_max);
	// no rep_addr
  res |= ModRTU_CRC(req_addr_set, req_addr_set_max);
	// no rep_addr_set
  if (res == 0) {
    Serial.println("crc_debug PASS");
  } else {
    Serial.println("crc_debug FAIL !!!!!!!!!!!!!!!!!!");
		halt();
		return -1;
  }
  return 0;  
}

const bool DEBUG_REQUEST_REPLY = false;
int request_reply(byte *request, int request_max, byte *reply, int reply_max) {
  int res = -1;
	if (DEBUG_REQUEST_REPLY) {
		Serial.println("request_reply begin");
	}
	//  digitalWrite(RTS_pin, RS485Transmit);
	if (DEBUG_REQUEST_REPLY) {
		print_buf("request = ", request, request_max);
	}
  RS485Serial.write(request, request_max);
  RS485Serial.flush();
	delayMicroseconds(T35_usec); // added 4/19/22
	//  digitalWrite(RTS_pin, RS485Receive);
	// put delay here since pin is set high here;
	int tr = 50;
	delay(tr);
	int nread = 0;
	bool overflow_frame = false;
	bool overflow_data = false;
	while (RS485Serial.available()) {
		byte B = RS485Serial.read();
		if (!overflow_data) {
			reply[nread] = B;
			nread++;
			if (nread >= reply_max) overflow_data = true;
		} else if (!overflow_frame) {
			nread++;
			if (nread >= FRAME_MAX) overflow_frame = true;
		}
		delayMicroseconds(T15_usec);
	}
	if (DEBUG_REQUEST_REPLY) {
		Serial.print("read "); Serial.print(nread); Serial.println(" bytes");
	}
	if (overflow_frame) {
		Serial.println("overflow frame; nread > FRAME_MAX;");
		return  -1;
	}
	if (nread == 0) {
		Serial.println("underflow frame; nread = 0;");
		return -1;
	}
	if (nread > reply_max) {
		Serial.println("nread > reply_max");
	}
	if (nread < reply_max) {
		Serial.println("nread < reply_max");
		return -1;
	}
	if (DEBUG_REQUEST_REPLY) {
		print_buf("complete reply   = ", reply, nread);
	}
  if (nread != reply_max) {
    Serial.println("error:  nread != reply_max");
    Serial.print("nread = "); Serial.println(nread);
    Serial.print("reply_max = "); Serial.println(reply_max);
    Serial.println("request_reply read FAIL !!!!!!!!!!!!!!!!!!!!!!");
  }
  res = ModRTU_CRC(reply, reply_max);
  if (res != 0) {
    Serial.println("request_reply crc FAIL !!!!!!!!!!!!!!!!!!!!!!");
		return -1;
  }
	if (DEBUG_REQUEST_REPLY) {
		Serial.println("request_reply crc PASS");
	}
  return 0;
}

int get_address(void) {
  int res = -1;
  Serial.println("get_address begin");
  res = request_reply(req_addr, req_addr_max, rep_addr, rep_addr_max);
  if (res != 0) {
		Serial.println("get_addr request_reply FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return -1;
  }
	byte addr = rep_addr[0];
	Serial.print("address = "); Serial.println(addr);
	return 0;
}

int set_address(void) {
  int res = -1;
  Serial.println("set_address begin");
  res = request_reply(req_addr_set, req_addr_set_max, rep_addr_set, rep_addr_set_max);
	// you can not see reply until reboot of arduino
	/*
  if (res != 0) {
		Serial.println("get_addr request_reply FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		return -1;
  }
	long addr = ((long) rep_addr[3]) << 8 + ((long) rep_addr[4]);
	Serial.print("addr = "); Serial.println(addr);
	*/
  return 0;
}

bool DEBUG_GET_DAT = false;
int get_dat(void) {
  int res = -1;
  Serial.println("get_dat begin");
  res = request_reply(req_dat, req_dat_max, rep_dat, rep_dat_max);
  if (res != 0) {
    Serial.println("get_dat request_reply FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    // delay(5 * 1000);   // commented out 4/19/22
    return -1;
  }
	if (DEBUG_GET_DAT) {
		print_buf("sensor reply     = ", rep_dat, rep_dat_max);
	}
  int16_t temperature_i = word(rep_dat[3], rep_dat[4]);
  int16_t humidity_i    = word(rep_dat[5], rep_dat[6]);
	float   temperature   = ((float) temperature_i) / 100.0;
	float   humidity      = ((float) humidity_i   ) / 100.0;
	// convert to Fahrenheit
	temperature = temperature * 9.0 / 5.0 + 32.0;
	Serial.print("temperature_i = "); Serial.println(temperature_i);
	Serial.print("humidity_i    = "); Serial.println(humidity_i);
	Serial.print("temperature   = "); Serial.println(temperature, 8);
	Serial.print("humidity      = "); Serial.println(humidity, 8);
  // delay(100);    // commented out 4/19/22
  // delay(5 * 1000); // commented out 4/19/22
	return 0;
}

void setup() {
	int res = -1;
	//  pinMode(RTS_pin, OUTPUT);
  // note this is always 9600 baud !!!!
  Serial.begin(9600);
  while(!Serial);
  sprintf(buf,"utility_temp\n"); Serial.print(buf);
	res = crc_debug();
	calc_timeout();
  // start the Modbus serial port
  RS485Serial.begin(BAUD);
  delay(1000);
}

void loop() {
  int res = -1;
  Serial.println("LOOP BEGIN ===================================================");
	res = get_address();
	res = get_dat();
	//	res = set_address();
	//	halt();
}

// eee eof
