#define LTEbaudrate 115200 // can autobaud 9600, 19200, 38400, 57600, 115200, 230400, 460800, and 921600
                           // but it seems Arduino Due can't do more than 115200
#define USBSerial Serial
#define LTESerial Serial3
#define CamSerial Serial2

int state[4] = {0,0,0,0};

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
      flushall(); // flush all the Serials. Not used for anything, I just thought it might help (it didn't)
    } else if (out.equals("RESETSERIAL")) {
      resetSerial(); // end USBSerial and LTESerial, then begin them again. Also didn't help.
    } else if (out.equals("GETPIC")) {
      requestPicture(); // make the camera take a picture and transfer it to the LTE module
    } else if (out.equals("CARLEFTTEST")) {
      carLeft(); // test sending the carLeft message to the server
    } else if (out.startsWith("SPOTUPDATETEST ")) {
      spotUpdateTest(out); // test sending a spotupdate byte to the server
    } else if (out.startsWith("BYTES ")) {
      getBytes(out); // send a command to the LTE module, but get the answer printed as hex bytes instead of a String.
    } else if (out.startsWith("READTEST ")) {
      readTest(out); // test the readFile() function
    } else if (out.startsWith("HTTPREAD ")) {
      HTTPReadTest(out); // test the getHTTPContent() function
    } else {
      LTESerial.println(out); //otherwise send the command directly to the LTE module
    }
  }
}

void setupRadio1() {
  //network settings
  send("AT+CFUN=0\r"); // turn off the radio
  delay(200);
  send("AT+UMNOPROF=100\r"); // set MNO (Mobile Network Operator) profile for Europe
  delay(200);
  send("AT+CFUN=16\r"); // reboot radio module
  delay(500);
  send("AT+UMNOPROF?\r"); // check if MNO is set correctly
  delay(200);
  send("AT+CFUN=0\r"); // turn off the radio again
  delay(200);
  send("AT+CGDCONT=1,\"IPV4\",\"telenor.iot\"\r"); // Define a PDP context with CID 1, connect to telenor
  delay(200);
}
void setupRadio2() {
  //LTE settings
  send("AT+CFUN=1\r"); // turn on the radio
  delay(10000);
  send("AT+COPS?;+CEREG?;+CGATT?;+CGDCONT?\r"); // four commands that show info about the current connection.
  delay(200);
  send("AT+UPSD=0,0,0;AT+UPSD=0,100,1\r"); // set packet-switched data profile 0 to use IPv4
  delay(200);
  send("AT+CGACT=1,1\r"); // activate PDP-context 1
  delay(200);
  send("AT+UPSDA=0,2\r"); // load settings for profile 0
  delay(200);
  send("AT+UPSDA=0,3\r"); // activate settings for profile 0
  delay(2000);
  send("AT+UPING=\"8.8.8.8\"\r"); //try to ping Google
}

void setupHTTP() {
  send("AT+UHTTP=1,0,\"20.52.253.18\"\r"); // define HTTP profile 1 to send to 20.52.253.18
  delay(100);
  send("AT+UHTTP=1,4,0\r"); // set profile 1 to use authentication type 0 (no authentication)
  delay(100);
  send("AT+UHTTP=1,5,80\r"); // set profile 1 to send on port 80
  delay(100);
  send("AT+USECPRF=0,0,0\r"); // set security config profile 0 to not use certificate validation
  delay(100);
  send("AT+UHTTP=1,6,1,0\r"); // set HTTP profile 1 to use TLS with security config 0
}

void requestPicture() {
  // tells the ESP Cam to take a picture and transmit it over UART, to be saved on the LTE module memory
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

String getPicAndUpload() {
  // called by one ultrasonic sensor when a car approaches the gate
  send("AT+UDELFILE=\"picture\"\r"); // delete the existing picture
  requestPicture();
  LTESerial.print("AT+UHTTPC=1,4,\"/DUE/upload\",\"uploadresponse\",\"picture\",3\r"); // POST the file to the server.
  delay(4000);
  return getHTTPContent("uploadresponse");

}

void send(String message) {
  // sends a command to the LTE module, waits briefly for a response
  LTESerial.print(message);
  delay(100);
  while (LTESerial.available()) {
    String out = LTESerial.readStringUntil('\r');
    USBSerial.println(out);
  }
}

String readFile(String filename) {
  // function that reads a text file stored on the LTE module, returning it as string.
  // LTE module returns the file in the format:
  // +URDFILE: "filename",[filesize],"Actual contents of the file"\r\nOK\r\n
  LTESerial.print("AT+URDFILE=\""+filename+"\"\r");
  delay(20);
  char buf[2000];
  int bytesplaced = LTESerial.readBytes(buf, 2000);
  buf[bytesplaced] = '\0'; // terminate the string with a null  
  String output = String(buf); // convert the string into a String
  for (int i = 0; i < 5; i++) {
    output = output.substring(output.indexOf("\"")+1); // keep cutting down the String every time a quotation mark is encountered, 5 times
  }
  output = output.substring(0, output.length()-7); // cuts off the last quotation mark and the OK
  return output;
}

String getHTTPContent(String input) {
  // HTTP response always has an empty line separating the header and the content
  // go through and look for CRLF CRLF.
  int index = 0;
  for (int index = 0; index < input.length(); index++) {
    if (input.charAt(index)   == '\r' && 
        input.charAt(index+1) == '\n' &&
        input.charAt(index+2) == '\r' &&
        input.charAt(index+3) == '\n') {
          return input.substring(index+4);
    }
  }
  return "";
}

void spotUpdate(byte input) {
  // called by the remote ultrasonic sensors when a car enters or leaves a spot
  // 7 bit spotID + 1 bit occupiedstatus (0 = free, 1 = occupied)
  
  LTESerial.print("AT+UHTTPC=1,5,\"/DUE/update\",\"updateresponse\",\"");
  LTESerial.write(input);
  LTESerial.print("\",2\r");
  LTESerial.flush();
  delay(4000);
  while (LTESerial.available()) {
    LTESerial.read(); // empty the incoming buffer.
  }
  String newstate = getHTTPContent(readFile("updateresponse"));
  USBSerial.println("Got newstate: "+newstate);
  
  for (int i = 0; i < 4; i++) {
    state[i] = (int) (newstate[i]-48); // convert from char to int, '0' is 48.
  }
} 

void carLeft() {
  // called by the local ultrasonic sensor when a car leaves the parking lot
  LTESerial.print("AT+UHTTPC=1,5,\"/DUE/carLeft\",\"carLeftResponse\",\"\",1\r"); // send a POST with no content.
  // response doesn't matter and is overwritten every time.
}

/////////////////////////////////////
////////// Tests and debug //////////
/////////////////////////////////////

void readTest(String command) {
  // tests the readFile() command.
  command = command.substring(9); // cut off the first 9 characters of message (which will be "READTEST ")
  USBSerial.println("Trying to read file \""+command+"\"");
  String filecontents = readFile(command);
  USBSerial.print("File read: ");
  USBSerial.println(filecontents);
  String HTTPcontents = getHTTPContent(filecontents);
  USBSerial.print("HTTP content: ");
  USBSerial.println(HTTPcontents);
}

void HTTPReadTest(String command) {
  command = command.substring(9); // cut off the first 9 characters of message (which will be "HTTPREAD ")
  String fileContents = readFile(command);
  String HTTPContents = getHTTPContent(fileContents);
  USBSerial.println("HTTPREAD got contents: "+HTTPContents);
}

void spotUpdateTest(String command) {
  // tests the spotUpdate() command
  // command should be called with an 8-bit binary number
  command = command.substring(15); // cut off the first 15 characters of message (which will be "SPOTUPDATETEST ")
  if (command.length() != 8) { // must be a full byte
    USBSerial.println("Invalid input.");
    return;
  }

  char **ptr; // necessary for strtol()
  const char *commandChars = command.c_str();
  long byteLong = strtol(commandChars, ptr, 2); 
  byte functionInput = (byte) byteLong;
  USBSerial.print("Sending ");
  USBSerial.print(functionInput, BIN);
  USBSerial.println(" to spotUpdate()");
  spotUpdate(functionInput);
}

void resetSerial() { 
  // possibly helpful
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

void getBytes(String message) {
  // send a command to the LTE module, print the output to terminal as a list of hex bytes.
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
