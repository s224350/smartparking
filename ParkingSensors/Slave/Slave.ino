//Library setup
#include <Arduino.h>
#include <HardwareSerial.h>

//LoRa pin setup
//GND -GND
//3V3 - 3V3
//Rx - Tx
//Tx - Rx

//Sensor pin setup
//Gnd - Gnd
//Vcc - VIN
//Trig - 5
//Echo -18


//LoRa configuration
HardwareSerial loraSerial(1);
#define RxPin 16
#define TxPin 17
#define BAUDRATE 9600
#define SER_BUF_SIZE 1024

//Sound speed definition
#define SOUND_SPEED 0.034


// Ultrasonic Sensor configuration
const int trigPin = 5;
const int echoPin = 18;

//State variables
float detectionDistance = 10.0;
int prevState = false;


// LoRa communications variables
String str;
const int retransmitInterval = 10000;
bool messageAcknowledged  = true;

//Light-sleep variable
int sleepTime = 2000;

//Spot and statusupdate varibles and definitions
int spotID = 0;
byte applicationID = 0xef;  //ApplicationID to distinguish between other applications
int msgAvailable = (applicationID << 8) | (spotID << 1 | 0); //0 -> not occupied
int msgOccupied = (applicationID << 8) | (spotID << 1 | 1);  //1 -> occupied
int msgAck = msgOccupied; //ACK is occupied message
int packetToSend;

void setup() {
//Sensor setup
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);

  //Set baudrate
Serial.begin(115200);
Serial.println("hello");

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
  runLoRaCommand("radio set freq 869400000"); //Frequency
  runLoRaCommand("radio set pwr 14"); //Power
  runLoRaCommand("radio set sf sf7"); //Spreadingfactor
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

   //Enabling wake up from light-sleep
  esp_sleep_enable_timer_wakeup(sleepTime*1000);
}


void loop() {

  Serial.println(".");
  bool newState = isCarParked();

 //Spot status assessments - Spotstatus updated sent if change in state
  if (newState != prevState) {
    Serial.println("State changed");
    prevState = newState;
    packetToSend = (newState) ? msgOccupied : msgAvailable;
    messageAcknowledged = false;
  }

   //Retransmitting update if ACK isn't received
  if (!messageAcknowledged){
    Serial.println("Transmitting message");
    runLoRaCommand("radio rxstop");
    transmitMessage(packetToSend);
    runLoRaCommand("radio rx 0"); 
    delay(1000);
  }

 //ACK recieving logic -> receiving messages while listening and checking if correct ACK
  if(loraSerial.available()){
    Serial.print("Recevied Data:");
    String message = readLoRaMessage();
    Serial.print(message);
    if (message.length() == 14){
      message = message.substring(10);

      char buffer[message.length()+1];
      message.toCharArray(buffer, message.length()+1);
      int messageValue = strtol(buffer, NULL, 16);

      Serial.print("msgAck:");
      Serial.println(msgAck,DEC);
      Serial.print("Message:");
      Serial.println(messageValue,DEC);
      if(msgAck == messageValue){
        Serial.println("ACK confirmed");
        messageAcknowledged = true;
      }
    }
  }

  //Makes sure all lora serial data in buffer is transmitted
  loraSerial.flush();
 //Makes sure all serial data is transmitted
  Serial.flush();
  //ESP put in light sleep
  esp_light_sleep_start();
}


//Sensor function used to measure distance to object
float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  float distanceCm = duration * SOUND_SPEED / 2;
  return distanceCm;
}

bool isCarParked(){
  return measureDistance() < detectionDistance;
}



//Message function
void transmitMessage(int intmessage) {
  String message = String(intmessage,16);
  Serial.print("Sending message: ");
  Serial.println(message);
  String txCommand = "radio tx " + message;
  runLoRaCommand(txCommand);
  Serial.println(readLoRaMessage());
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

//LoRa autobaud function
void lora_autobaud() {
 String response = "";
 while (response.isEmpty()) {
   delay(1000);
   loraSerial.write((byte)0x00);
   loraSerial.write(0x55);
   loraSerial.println();
   loraSerial.println("sys get ver");
   response = loraSerial.readStringUntil('\n');
 }
}
