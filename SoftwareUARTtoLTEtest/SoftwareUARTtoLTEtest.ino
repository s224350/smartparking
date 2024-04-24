#include <HardwareSerial.h>

#define TXpin 17
#define RXpin 16

HardwareSerial MySerial(1);

void setup() {
  
  Serial.begin(115200); //standard hardware serial
  MySerial.setRxBufferSize(256);
  MySerial.begin(115200, SERIAL_8N1, RXpin, TXpin);

  delay(500);
  Serial.println("Hello world!");
} 

void loop() {
  if (MySerial.available()) {
    String inc = MySerial.readStringUntil('\r');
    Serial.print(inc);
  }
  if (Serial.available()) {
    String out = Serial.readStringUntil('\r');
    MySerial.println(out);
  }
}