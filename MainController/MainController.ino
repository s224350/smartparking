//#include <HardwareSerial.h>

#define LTEbaudrate 115200 // can autobaud 9600, 19200, 38400, 57600, 115200, 230400, 460800, and 921600
                           // but it seems Arduino Due can't do more than 115200
#define USBSerial Serial
#define LTESerial Serial3
#define CamSerial Serial2

byte testPicture[155] = {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x08,0x02,0x03,0x00,0x00,0x00,0xb9,0x61,0x56,0x18,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,0x00,0x04,0x67,0x41,0x4d,0x41,0x00,0x00,0xb1,0x8f,0x0b,0xfc,0x61,0x05,0x00,0x00,0x00,0x09,0x50,0x4c,0x54,0x45,0xff,0xff,0xff,0x00,0x94,0xff,0x00,0x00,0x00,0x84,0xd2,0x2e,0x7c,0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0e,0xc3,0x00,0x00,0x0e,0xc3,0x01,0xc7,0x6f,0xa8,0x64,0x00,0x00,0x00,0x1b,0x49,0x44,0x41,0x54,0x18,0xd3,0x63,0x60,0x0c,0x65,0xe0,0x48,0x05,0x21,0x06,0x51,0x10,0x52,0x88,0x64,0xd0,0x5a,0xc9,0xc0,0x18,0x0a,0x00,0x1f,0x93,0x02,0xfd,0xf5,0x73,0xed,0xbe,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};

void setup() {
  USBSerial.begin(115200); // Serial0 communicating with PC over USB
  LTESerial.begin(LTEbaudrate); // Serial1 communicating with LTE module over UART
  CamSerial.begin(115200); // Serial2 communicating with ESP camera over UART
  delay(500);
  USBSerial.println("Hello world!");
} 

void loop() {
  if (LTESerial.available()) { // read anything from the LTE module if it has anything to say
    String inc = LTESerial.readStringUntil('\r');
    USBSerial.print(inc);
  }

  if (USBSerial.available()) { // allow the user at the serial monitor to enter commands
    String out = Serial.readStringUntil('\r');
    // some debug commands to be activated through the serial monitor:
    if (out.equals("SETUPRADIO1")) {
      setupRadio1(); // Network settings on the LTE module
    } else if (out.equals("SETUPRADIO2")) {
      setupRadio2(); // LTE settings on the LTE module
    } else if (out.equals("SETUPHTTP")) {
      setupHTTP(); // HTTP settings to interface with the backend server
    } else if (out.equals("FLUSHALL")) {
      flushall();
    } else if (out.equals("GETPIC")) {
      requestPicture(); // make the camera take a picture and transfer it to the LTE module
    } else if (out.equals("RESETSERIAL")) {
      resetSerial();
    } else if (out.equals("GETTEST")) {
      getAndReadTest();
    } else if (out.startsWith("BYTES ")) {
      getBytes(out);
    } else if (out.startsWith("READTEST ")) {
      readTest(out);
    } else {
      LTESerial.println(out); //otherwise send the command directly to the LTE module
    }
  }
}

void setupRadio1() {
  //network settings
  send("AT+CFUN=0\r"); // turn off the radio
  send("AT+UMNOPROF=100\r"); // set MNO (Mobile Network Operator) profile for Europe
  send("AT+CFUN=16\r"); // reboot radio module
  send("AT+UMNOPROF?\r"); // check if MNO is set correctly
  delay(200);
  send("AT+CFUN=0\r"); // turn off the radio again
  send("AT+CGDCONT=1,\"IPV4V6\",\"telenor\"\r"); // Define a PDP context with CID 1, connect to telenor
}
void setupRadio2() {
  //LTE settings
  send("AT+CFUN=1\r"); // turn on the radio
  delay(4000);
  send("AT+COPS?;+CEREG?;+CGATT?;+CGDCONT?\r"); // four commands that show info about the current connection.
  send("AT+UPSD=0,0,0;AT+UPSD=0,100,1\r"); // set packet-switched data profile 0 to use IPv4
  send("AT+CGACT=1,1\r"); // activate PDP-context 1
  send("AT+UPSDA=0,2\r"); // load settings for profile 0
  delay(200);
  send("AT+UPSDA=0,3\r"); // activate settings for profile 0
  delay(500);
  send("AT+UPING=\"8.8.8.8\"\r"); //try to ping Google
}

void setupHTTP() {
  send("AT+UHTTP=1,0,\"20.52.253.18\"\r");
  //send("AT+UHTTP=1,1,\"20.52.253.18\"\r");
  send("AT+UHTTP=1,4,0\r");
  send("AT+UHTTP=1,5,80\r");
  //send("AT+UHTTP=1,6,0\r");
  send("AT+USECPRF=0,0,0\r");
  send("AT+UHTTP=1,6,1,0\r");
}

void requestPicture() {
  // tells the ESP Cam to take a picture and transmit it over UART. 
  CamSerial.write(98); // one pre-agreed byte that means "take picture and send it"
  USBSerial.println("Waiting for CamSerial...");
  while (CamSerial.available() == 0) {} // wait for a response
  byte buf[4]; // the camera will first send 4 bytes informing of the size of the picture
  CamSerial.readBytes(buf, 4);
  long bytecount = 0; // the bytes arrive with the least significant first

  bytecount += ((long)buf[0]); // convert the four bytes into a long
  bytecount += ((long)buf[1]) << 8;
  bytecount += ((long)buf[2]) << 16;
  bytecount += ((long)buf[3]) << 24;

  USBSerial.println("Received bytes:"); // debug information to let us know if the bytes arrived correctly
  for (int i = 0; i < 4; i++) {
    USBSerial.print(buf[i], HEX);
    USBSerial.print(", ");
  }
  
  send("AT+UDWNFILE=\"picture\","+(String)bytecount+"\r"); // send command to LTE module to prepare it for receiving the file
  while (LTESerial.available() == 0) {} // wait for response
  while (LTESerial.available()) { // read response
    String inc = LTESerial.readStringUntil('\r');
    USBSerial.print(inc);
  }

  byte incomingbyte;
  CamSerial.write(99); // tell cam to send the rest of the data
  while (CamSerial.available() == 0) {} //wait for data to be available
  while (bytecount > 0) { // receive and send one byte at a time
    if (!CamSerial.available()) {
      continue;
    }
    incomingbyte = CamSerial.read();
    LTESerial.write(incomingbyte);
    bytecount--;
  }
}

void sendPicture() { 
  LTESerial.print("AT+UHTTPC=1,4,\"/DUE/upload\",\"uploadresponse\",\"picture\",3\r");
}

void getBytes(String message) {
  message = message.substring(6); // cut off the first 5 characters of message (which will be "BYTES ")
  USBSerial.println("Trying to send: "+message);
  LTESerial.print(message+"\r");
  byte inc;
  char buffer[4];
  long starttime = millis();
  while (millis() - starttime < 2000) {
    if (LTESerial.available()) {
      inc = LTESerial.read();
      sprintf(buffer, "%02X, ", inc);
      USBSerial.print(buffer);
    }
  }
  USBSerial.println();
}

void send(String message) {
  LTESerial.print(message);
  delay(60);
  while (LTESerial.available()) {
    String out = LTESerial.readStringUntil('\r');
    USBSerial.println(out);
  }
}

void getAndReadTest() {
  LTESerial.print("AT+UHTTPC=1,1,\"/DUE/hello\",\"getresponse\"\r");
  delay(100);
  String response = readfile("getresponse");
  USBSerial.print("File read: ");
  USBSerial.println(response);
}

void readTest(String command) {
  command = command.substring(9); // cut off the first 5 characters of message (which will be "READTEST ")
  USBSerial.println("Trying to read file \""+command+"\"");
  String filecontents = readfile(command);
  USBSerial.print("File read: ");
  USBSerial.println(filecontents);
}

void getStateFromServer() {
  LTESerial.print("AT+UHTTPC=1,1,\"/DUE/state\",\"getresponse\"\r");
  delay(100);
}

String readfile(String filename) {
  // function that reads a text file stored on the LTE module, returning it as string.
  // LTE module returns the file in the format:
  // +URDFILE: "filename",[filesize],"Actual contents of the file"\r\nOK\r\n
  byte buf[100];
  LTESerial.print("AT+URDFILE=\""+filename+"\"\r");
  delay(50);
  LTESerial.readBytesUntil('\n',buf,100); // removes the command echo from buffer
  for (int i = 0; i < filename.length()+13; i++) {
    LTESerial.read(); // removes the first part from buffer: +URDFILE: "filename",
  }
  LTESerial.readBytesUntil('\"',buf,100); // removes the next part including the file size
  // now the only remaining part are the actual file contents

  String output = LTESerial.readString();
  output = output.substring(0, output.length()-7); // cuts off the last quotation mark and the OK
  return output;
}

void resetSerial() { // possibly helpful
  USBSerial.println("Attempting to reset Serials...");
  LTESerial.end();
  USBSerial.end();
  delay(500);
  LTESerial.begin(LTEbaudrate);
  USBSerial.begin(115200);
  delay(500);
  USBSerial.println("Serials rebooted.");
}

void flushall() { // possibly helpful
  USBSerial.println("Flushing all Serials");
  LTESerial.flush();
  USBSerial.flush();
  CamSerial.flush();
}


