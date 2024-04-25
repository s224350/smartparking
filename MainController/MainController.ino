//#include <HardwareSerial.h>

#define LTEbaudrate 115200 // can autobaud 9600, 19200, 38400, 57600, 115200, 230400, 460800, and 921600
#define USBSerial Serial
#define LTESerial Serial1
#define CamSerial Serial2

byte testPicture[155] = {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x08,0x02,0x03,0x00,0x00,0x00,0xb9,0x61,0x56,0x18,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,0x00,0x04,0x67,0x41,0x4d,0x41,0x00,0x00,0xb1,0x8f,0x0b,0xfc,0x61,0x05,0x00,0x00,0x00,0x09,0x50,0x4c,0x54,0x45,0xff,0xff,0xff,0x00,0x94,0xff,0x00,0x00,0x00,0x84,0xd2,0x2e,0x7c,0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0e,0xc3,0x00,0x00,0x0e,0xc3,0x01,0xc7,0x6f,0xa8,0x64,0x00,0x00,0x00,0x1b,0x49,0x44,0x41,0x54,0x18,0xd3,0x63,0x60,0x0c,0x65,0xe0,0x48,0x05,0x21,0x06,0x51,0x10,0x52,0x88,0x64,0xd0,0x5a,0xc9,0xc0,0x18,0x0a,0x00,0x1f,0x93,0x02,0xfd,0xf5,0x73,0xed,0xbe,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};

void setup() {
  USBSerial.begin(115200); // Serial0 communicating with PC over USB
  LTESerial.begin(LTEbaudrate); // Serial1 communicating with LTE module over UART
  CamSerial.begin(115200); // Serial2 communicating with ESP camera over UART
  delay(500);
  Serial.println("Hello world!");
} 

void loop() {
  if (LTESerial.available()) {
    String inc = LTESerial.readStringUntil('\r');
    USBSerial.print(inc);
  }

  if (CamSerial.available()) {
    String inc = CamSerial.readStringUntil('\r');
    if (inc.equals("SEND")){
      USBSerial.println("Received SEND from Cam");
      fromCamToLTE();
    } else {
      USBSerial.println("Received unknown command from Cam");
    }
  }

  if (USBSerial.available()) {
    String out = Serial.readStringUntil('\r');
    // some debug commands to be activated through the serial monitor:
    if (out.equals("SETUPRADIO")) {
      setupRadio();
    } else if (out.equals("SETUPHTTP")) {
      setupHTTP();
    } else if (out.equals("TRANSFERTEST")) {
      transferTestPictureToLTE();
    } else if (out.equals("FLUSHALL")) {
      flushall();
    } else { //otherwise send the command directly to the LTE module
      LTESerial.println(out);
    }
  }
}

void setupRadio() {
//network settings
  send("AT+CFUN=0\r");
  send("AT+UMNOPROF=100\r");
  send("AT+CFUN=16\r");
  send("AT+UMNOPROF?\r");
  send("AT+CFUN=0\r");
  send("AT+CGDCONT=1,\"IPV4V6\",\"telenor\"\r");
//LTE settings
  send("AT+CFUN=1\r");
  send("AT+COPS?\r");
  send("AT+CEREG?\r");
  send("AT+CGATT?\r");
  send("AT+CGDCONT?\r");
  send("AT+UPSD=0,0,0\r");
  send("AT+UPSD=0,100,1\r");
  send("AT+CGACT=1,1\r");
  send("AT+UPSDA=0,2\r");
  send("AT+UPSDA=0,3\r");
  send("AT+UPING=\"8.8.8.8\"\r"); //try to ping Google
}

void setupHTTP() {
  send("AT+UHTTP=1,0,\"20.52.253.18\"\r");
  send("AT+UHTTP=1,1,\"20.52.253.18\"\r");
  send("AT+UHTTP=1,4,0\r");
  send("AT+UHTTP=1,5,80\r");
  send("AT+UHTTP=1,6,0\r");
}

void transferTestPictureToLTE() {
  send("AT+UDWNFILE=\"picture\",155\r");
  delay(200);
  LTESerial.flush();
  LTESerial.write(testPicture,155);
  LTESerial.flush();
}

void fromCamToLTE() {
  String inc = CamSerial.readStringUntil('\r');
  int bytecount = inc.toInt();
  byte buf[64];
  send("AT+UDWNFILE=\"picture\","+(String)bytecount+"\r");
  delay(200);
  while (bytecount > 0) {
    if (bytecount >= 64) {
      USBSerial.println("Sending 64 bytes to LTE");
      CamSerial.readBytes(buf, 64);
      LTESerial.flush();
      LTESerial.write(buf, 64);
      LTESerial.flush();
      bytecount = bytecount - 64;
    } else {
      USBSerial.println("Sending "+(String)bytecount+" bytes to LTE");
      CamSerial.readBytes(buf, bytecount);
      LTESerial.flush();
      LTESerial.write(buf, bytecount);
      LTESerial.flush();
      bytecount = 0;
    }
  }
  while (LTESerial.available() == 0) {} // wait for response
  String response = LTESerial.readStringUntil('\r');

  // picture should now be in the LTE module's memory
  // try to send it to server:
  send("AT+UHTTPC=1,4,\"/upload\",\"myresponse\",\"picture\",3");
}
void send(String message) {
  LTESerial.print(message);
  delay(50);
  while (LTESerial.available()) {
    String out = LTESerial.readStringUntil('\r');
    USBSerial.println(out);
  }
}

void flushall() {
  LTESerial.flush();
  USBSerial.flush();
  CamSerial.flush();
}
