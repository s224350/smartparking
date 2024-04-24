#include <SoftwareSerial.h>

#define MYPORT_TX 32
#define MYPORT_RX 33

EspSoftwareSerial::UART myPort;

void setup() {
  
  Serial.begin(115200); //standard hardware serial
  myPort.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);

  delay(500);
  Serial.println("Hello world!");

  if (!myPort) { // If the object did not initialize, then its configuration is invalid
  Serial.println("Invalid EspSoftwareSerial pin configuration, check config"); 
  while (1) { // Don't continue with invalid configuration
    delay (1000);
  }
} 


}

void loop() {
  
  if (myPort.available()) {
    String inc = myPort.readString();
    Serial.println(inc);
  }
  if (Serial.available()) {
    String out = Serial.readString();
    myPort.print(out);
  }
}
