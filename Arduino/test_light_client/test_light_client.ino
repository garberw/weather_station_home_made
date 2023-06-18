#include <Arduino.h>

void setup(void) {
	Serial.begin(9600);
	Serial1.begin(9600);
}

byte count = 0;
#define STR 20
byte str[STR];

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

void loop(void) {
  // fn01();
  fn02();
}

// eee eof
