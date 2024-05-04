//Library setup
#include <Arduino.h>
#include <HardwareSerial.h>

//LoRa pin setup
//GND -GND
//3V3 - 3V3
//Rx - Tx
//Tx - Rx

//LoRa configuration
HardwareSerial loraSerial(1);
#define RxPin 16
#define TxPin 17
#define BAUDRATE 57600
#define SER_BUF_SIZE 1024

String str;

void setup() {
  //Set baudrate
  Serial.begin(57600);
  pinMode(23,OUTPUT);
  pinMode(23,LOW);
  
  //LoRa setup
  loraSerial.setRxBufferSize(SER_BUF_SIZE);
  loraSerial.begin(BAUDRATE, SERIAL_8N1, RxPin, TxPin);
  loraSerial.setTimeout(1000);
  lora_autobaud();

  Serial.println("Initing LoRa");
  runLoRaCommand("sys get ver",false);
  delay(100);
  runLoRaCommand("mac pause",false);
  runLoRaCommand("radio set mod lora"); 
  runLoRaCommand("radio set freq 869400000");  //Frequency
  runLoRaCommand("radio set pwr 14");  //Power
  runLoRaCommand("radio set sf sf7");  //Spreadingfactor
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
  Serial.println("starting loop");
}


void loop() {
  Serial.println("looping");
  delay(1000);
  //Checks for message and acting accordingly
  if (loraSerial.available()) {
    processMessage(readLoRaMessage());
  }
}


//Process for message reception - Printing message, changing status and transmitting ACK
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

    Serial.println("Received message from parking sensor");
    Serial.print("Application ID:");
    Serial.println(applicationID, HEX);
    Serial.print("Parking spot:");
    Serial.println(spotID+1,DEC);
    Serial.print("Occupied:");
    Serial.println(occupied);
    delay(50);
    sendAck(spotID);
    
    runLoRaCommand("radio rx 0");
    return true;
}

//Run a LoRa command, and print error if command fails and validateReponse is true
void runLoRaCommand(String command, bool validateReponse){
  loraSerial.println(command);
  str = readLoRaMessage();
  Serial.print("Command:");
  Serial.print(command);
  Serial.print("  Reponse:");
  Serial.println(str);

  if (validateReponse && str.equals("ok") == false){
    Serial.print("ERROR during: ");
    Serial.println(command);
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
  Serial.print("Sent ACK: ");
  Serial.println(ackMessage);
  String txCommand = "radio tx " + String(ackMessage,HEX);
  runLoRaCommand(txCommand);
  Serial.println(readLoRaMessage());
}

//LoRa autobaud function
void lora_autobaud() {
 String response = "";
 while (response.isEmpty()) {
   delay(1000);
   loraSerial.write((byte)0x00);
   loraSerial.write(0x55);
   loraSerial.println();
   loraSerial.println("sys get ver");
   response = loraSerial.readStringUntil('\r');
 }
}
