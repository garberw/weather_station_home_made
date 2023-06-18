#include <Arduino.h>

void setup(void) {
	Serial.begin(9600);
	Serial1.begin(9600);
}

int count = 0;

#define MAX 1
byte buf[MAX];

#define STR 20
byte str[STR];

void fn00(void) {
	Serial1.readBytes(buf, MAX);
	Serial.print("echo client read \"");
	Serial.print(buf[0], HEX);
	Serial.print("\" from server\n");
	delay(1000);
}

void fn_receive(void) {
	char *ptr = ((char *) str);
	Serial1.readBytes(str, STR);
	Serial.print("echo read str = \"");
	for (int b = 0; b < STR; b++) {
		Serial.print(ptr[b]);
	}
	Serial.println("\"");
	delay(1000);
}


void fn_send(void) {
  char *ptr = ((char *) str);
  sprintf(ptr, "hello world %d; ", count);
  byte len = strlen(ptr);
  Serial.println(ptr);
  Serial1.write(str, len);
  count++;
  delay(1000);
}

void fn01(void) {
	Serial.println("fn01 server receive");
	fn_send();
}

void fn02(void) {
	Serial.println("fn02 client receive");
	fn_receive();
}

void fn03(void) {
	Serial1.readBytes(str, STR);
	Serial.print("echo read str = \"");
	for (int b = 0; b < STR; b++) {
		Serial.print((char) str[b]);
	}
	Serial.println("\"");
	Serial1.write(str, STR);
	delay(1000);
}

void loop(void) {
	// fn01();
  fn02();
}

// eee eof
