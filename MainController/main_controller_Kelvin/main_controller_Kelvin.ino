#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>
#include <HardwareSerial.h>
//#include <SoftwareSerial.h> //virker ikke på due
//#define RX 13
//#define TX 7


//lora setup
//HardwareSerial loraSerial(1);
#define LTEbaudrate 115200
#define USBSerial Serial
#define loraSerial Serial1 //til due board
#define CamSerial Serial2
#define LTESerial Serial3
//#define loraSerial Serial1 //til uno board
//#define RxPin 19//19
//#define TxPin 18 //18
#define BAUDRATE 9600
#define SER_BUF_SIZE 1024
String str;

//servo setup
#define servoPin 2 // Pin for the servo motor
#define closeAngle 180 // Servo angle for "closed" position
#define openAngle 90 
Servo servo; // Instance of the Servo class
int angle = closeAngle; // Current angle of the servo motor
int gateWait = 3000; //how long the gate is opened for


int spotID = 1; //1 is set as a placeholder

// Ultrasonic Sensor konfiguration
const int trigPinIn = 51; //51
const int echoPinIn = 53; //53

const int trigPinOut = 50; //50
const int echoPinOut = 52; //52

float detectionDistance = 5.0;
//Definition af lydens hastighed
#define SOUND_SPEED 0.034
int state[4] = {0,0,0,0};

//display setup
unsigned long blinkStartTime = 0;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,13);


void setup() {
  servo.attach(servoPin); // Attach the servo motor to the specified pin
  servo.write(angle); // Initialize servo to the closed position
  //display setup:
  u8g2.begin();
  // ultrasound setup:
  pinMode(trigPinIn, OUTPUT);
  pinMode(echoPinIn, INPUT);
  pinMode(trigPinOut, OUTPUT);
  pinMode(echoPinOut, INPUT);
  USBSerial.begin(115200);
  CamSerial.begin(115200);
  LTESerial.begin(115200);


  //initialising lora antenna
  //loraSerial.setRxBufferSize(SER_BUF_SIZE);
  //loraSerial.begin(BAUDRATE, SERIAL_8N1, RxPin, TxPin);
  loraSerial.begin(57600); //due
  loraSerial.setTimeout(1000);


  //lora setup
  pinMode(3,OUTPUT);
  pinMode(3,HIGH);
  delay(200);
  pinMode(3,LOW);

  lora_autobaud();
  USBSerial.println("Initing LoRa");
  runLoRaCommand("sys get ver",false);
  delay(100);
  runLoRaCommand("mac pause",false);
  runLoRaCommand("radio set mod lora"); 
  runLoRaCommand("radio set freq 868000000");
  runLoRaCommand("radio set pwr 14");
  runLoRaCommand("radio set sf sf7");
  runLoRaCommand("radio set afcbw 41.7");
  runLoRaCommand("radio set rxbw 20.8");
  runLoRaCommand("radio set prlen 8");
  runLoRaCommand("radio set crc on");
  runLoRaCommand("radio set iqi off");
  runLoRaCommand("radio set cr 4/5");
  runLoRaCommand("radio set wdt 60000");  //disable for continuous reception
  runLoRaCommand("radio set sync 12");
  runLoRaCommand("radio set bw 250");
  runLoRaCommand("radio rx 0");
  setupHTTP();
  
  USBSerial.println("starting loop");



}

void loop() {
  if (USBSerial.available()){
    USBSerial.println("Trying to call carleft()");
      carLeft();
    USBSerial.println("Done calling carleft()");
  }
  delay(200);
  USBSerial.println("looping");
  // Checking for incoming car:
  if (checkIncomingCar()){ //check if a car is coming in
    USBSerial.println("Car has pulled up");
    display_Welcome(); //display welcome on the display
    USBSerial.println("Taking picture");
    if (getPicAndUpload()){ //if license plate is accepted
      USBSerial.println("License plate accepeted");
      drawParkingSpots(state); 
      USBSerial.println("Opening gate");     
      openGate();
    }
    else{ //if license plate is not accepted
      USBSerial.println("License plate rejected");
      display_noAccess();
    }
  }

  //Listening for lora updates:
  if (loraSerial.available()){
    USBSerial.println("Received lora message");
    String loraMessage = readLoRaMessage();
    
    if (processMessage(loraMessage)){ //processMessage returns true if the message is valid and false if the message is invalid
      USBSerial.println("Message is valid");
      String data = loraMessage.substring(11); //gets the last 2 chars of the string EFxx msg


      char messageCharArray[data.length()+1];
      data.toCharArray(messageCharArray, data.length()+1);
      int messageLong = strtol(messageCharArray, NULL, 16);
      byte messageToSend = messageLong & 0xff;

      USBSerial.print("Sending Spot update:");
      USBSerial.println(messageToSend,HEX);

      spotUpdate(messageToSend);
    }
    else{ //recieved invalid message from lora
      USBSerial.println("recieved invalid message from lora");
    }
  }
  //checking for outgoing cars
  delay(200);
  if (checkOutgoingCar()){
    USBSerial.println("Car has left parking lot");
    carLeft();
    USBSerial.println("Opening gate");
    openGate();
  }
}

//LTE funktioner-----------------------------------------------------------------------------------------------------
void sendLTE(String message){
  USBSerial.print("sent '");
  USBSerial.print(message);
  USBSerial.println("' to LTE");
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

bool getPicAndUpload() {
  // called by one ultrasonic sensor when a car approaches the gate
  send("AT+UDELFILE=\"picture\"\r"); // delete the existing picture
  requestPicture();
  LTESerial.print("AT+UHTTPC=1,4,\"/DUE/upload\",\"uploadresponse\",\"picture\",3\r"); // POST the file to the server.
  delay(4000);
  return getHTTPContent("uploadresponse").equals("1");

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

  String messageToSendA = "AT+UHTTPC=1,5,\"/DUE/carLeft\",\"carLeftResponse\",\"\",1\r";

  USBSerial.println("Sending stuff");
  //USBSerial.println("Sending first part");
  //LTESerial.print("AT+UHTTPC=1,5,\""); // send a POST with no content.
  //USBSerial.println("Sending sec part");
  //LTESerial.print("/DUE/carLeft\","); // send a POST with no content.
  //USBSerial.println("Sending third part");
  //LTESerial.print("\"carLeftResponse\","); // send a POST with no content.
  //USBSerial.println("Sending forth part");
  //LTESerial.print("\"\",1"); // send a POST with no content.
  //USBSerial.println("Sending r part");
  //TESerial.print("\r"); // send a POST with no content.
  LTESerial.print(messageToSendA); // send a POST with no content.
  USBSerial.println("Done sending stuff");
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



//Display funktioner-----------------------------------------------------------------------------------------------------
void drawParkingSpots(int occSpots[]) {
  unsigned long startTime = millis();
  unsigned long endTime = startTime + 3000;  // Set end time for 3 seconds later

  while (millis() < endTime) {
    u8g2.clearBuffer();

    int displayWidth = u8g2.getDisplayWidth();
    int topRoad = u8g2.getDisplayHeight() / 2;
    int buttRoad = u8g2.getDisplayHeight() - 3;
    int spotWidth = displayWidth / 4;
    int spotHeight = 25;
    int textOffsetX = (spotWidth / 4) - 3;
    int textOffsetY = (spotHeight / 2) - 5;

    u8g2.drawLine(0, topRoad, displayWidth - 3, topRoad);
    u8g2.drawLine(0, buttRoad, displayWidth - 3, buttRoad);

    for (int x = 0; x < displayWidth; x += (3 + 5)) {
      u8g2.drawLine(x, topRoad + ((buttRoad - topRoad) / 2), x + 3, topRoad + ((buttRoad - topRoad) / 2));
    }

    u8g2.setFont(u8g2_font_ncenB08_tr);

    for (int i = 0; i < 4; i++) {
      char spotLabel[3];
      sprintf(spotLabel, "%d", i + 1);

      int x = i * spotWidth + (spotWidth / 4);
      u8g2.drawFrame(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
      u8g2.drawStr(x + textOffsetX, topRoad - textOffsetY, spotLabel);

      // Only blink the spot if the array element is 0
      if (occSpots[i] == 0) {
        if ((millis() - startTime) / 100 % 2 == 0) {  // Blink every 100 ms
          u8g2.setDrawColor(0);
          u8g2.drawBox(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
          u8g2.setDrawColor(1);
        }
      }
    }
    u8g2.sendBuffer();
  }
}

void display_Welcome() {
  u8g2.clearBuffer();                  
  u8g2.setFont(u8g2_font_ncenB14_tr);  
  u8g2.drawStr(12, 32, "Welcome!");    
  u8g2.sendBuffer();                   
}

void display_noAccess() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(12, 32, "No Access");
  u8g2.sendBuffer();
}

//-----------------------------------------------------------------------------------------------------

 // Ultralydsensor
float measureDistanceIn() {
  digitalWrite(trigPinIn, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinIn, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinIn, LOW);
  long duration = pulseIn(echoPinIn, HIGH);
  float distanceCm = duration * SOUND_SPEED / 2;
  return distanceCm;
}

bool checkIncomingCar(){
  USBSerial.print("checkIncomingCar distance: ");
  USBSerial.println(measureDistanceIn());
  return measureDistanceIn() < detectionDistance;

}

float measureDistanceOut() {
  digitalWrite(trigPinOut, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinOut, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinOut, LOW);
  long duration = pulseIn(echoPinOut, HIGH);
  float distanceCm = duration * SOUND_SPEED / 2;
  return distanceCm;
}

bool checkOutgoingCar(){
  USBSerial.print("checkOutGoingCar distance: ");
  float disout = measureDistanceOut();
  USBSerial.println(disout);
  return disout < detectionDistance;
}


//Gate--------------------------------------------------------------

void openGate() {
  // Example of using the servo in a loop (you can customize the behavior as needed)
  servo.write(openAngle); // Set servo to open position
  USBSerial.println("Servo set to open position.");
  delay(gateWait); // Wait for 2 seconds
  servo.write(closeAngle); // Set servo back to closed position
  USBSerial.println("Servo returned to closed position.");
}
//Camera--------------------------------------------------------------
void get_Picture(){
  USBSerial.println("*Snap* - Yep. This one's going in my cringe compilation");
}
bool send_Picture(){
  USBSerial.println("Cringe picture was sent to my compilation");
  return true;
}

void getAvailableSpots(int (& spotArray)[4]){
  USBSerial.print("assigned spots: ");
  for (int i = 0; i<4;i++){
    spotArray[i] = random(2);
    USBSerial.print(spotArray[i]);
    USBSerial.print(", ");
  }
  USBSerial.println("");

}

//lora--------------------------------------------------------------


//Proces når besked modtages her printes beskeden, status skiftes og en ack sendes tilbage
boolean processMessage(String receivedMessage) {
    receivedMessage = receivedMessage.substring(10);

    //Validate message length
    if (receivedMessage.length() != 4){
      return false;
    }

    //Convert message to char array
    char messageCharArray[receivedMessage.length()+1];
    receivedMessage.toCharArray(messageCharArray, receivedMessage.length()+1);

    //Convert message to int(2 bytes)
    int messageLong = strtol(messageCharArray, NULL, 16);

    //Extract frame fields using bitwise operations
    byte applicationID = (messageLong >> 8);
    byte spotID = (messageLong >> 1) & 0x7f;
    bool occupied = messageLong & 0x1; 

    //Validate application ID
    if (applicationID != 0xef){
      return false;
    }

    USBSerial.println("Received message from parking sensor");
    USBSerial.print("Application ID:");
    USBSerial.println(applicationID, HEX);
    USBSerial.print("Parking spot:");
    USBSerial.println(spotID+1,DEC);
    USBSerial.print("Occupied:");
    USBSerial.println(occupied);
    delay(50);
    sendAck(spotID);
    
    runLoRaCommand("radio rx 0");
    return true;
}

//Run a LoRa command, and print error if command fails and validateReponse is true
void runLoRaCommand(String command, bool validateReponse){
  loraSerial.println(command);
  str = readLoRaMessage();
  USBSerial.print("Command:");
  USBSerial.print(command);
  USBSerial.print("  Reponse:");
  USBSerial.println(str);

  if (validateReponse && str.equals("ok") == false){
    USBSerial.print("ERROR during: ");
    USBSerial.println(command);
  }
}

//Set validateReponse to true if not defined
void runLoRaCommand(String command){
  runLoRaCommand(command,true);
}

//Read response from LoRa module, and discard \r\n 
String readLoRaMessage(){
  String response = loraSerial.readStringUntil('\r');
  loraSerial.readStringUntil('\n');
  return response;
}

//Send acknoledgement packet
void sendAck(byte parkingSpotID) {
  int ackMessage = ((0xef << 8) + 0x1) | (parkingSpotID << 1);
  USBSerial.print("Sent ACK: ");
  USBSerial.println(ackMessage);
  String txCommand = "radio tx " + String(ackMessage,HEX);
  runLoRaCommand(txCommand);
  USBSerial.println(readLoRaMessage());
}


void lora_autobaud() {
 String response = "";
 while (response.equals("")) {
   delay(1000);
   USBSerial.println("autobaud");
   loraSerial.write((byte)0x00);
   loraSerial.write(0x55);
   loraSerial.println();
   loraSerial.println("sys get ver");
   response = loraSerial.readStringUntil('\n');
 }
}

//--------------------------------------------------------------
