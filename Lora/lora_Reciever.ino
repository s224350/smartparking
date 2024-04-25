
#include <Arduino.h>
#include <HardwareSerial.h>
//LoRa
//GND -GND
//3V3 - 3V3
//RST - 23
//Rx - Tx
//Tx - Rx


//Sensor
//Gnd - Gnd
//Vcc - VIN
//Trig - 5
//Echo -18


// LoRa konfiguration
HardwareSerial loraSerial(1);
#define RxPin 16
#define TxPin 17
#define BAUDRATE 9600
#define SER_BUF_SIZE 1024


// Ultrasonic Sensor konfiguration
const int trigPin = 5;
const int echoPin = 18;


//Definition af lydens hastighed
#define SOUND_SPEED 0.034


// Variable til dist og tid og status og tjek
long duration;
float distanceCm;
bool spotstatus = false; //falsk for occ true for ledig
unsigned long lastTime = 0;
int checkPeriod = 1000; //tjekker status hvert sek
int consistentCount = 0;
int requiredConsistency = 5; //antal ens gange før status skift


// LoRa kommunikations variable
String str;
unsigned long lastTransmitTime = 0;
const int transmitInterval = 10000;
const int maxRetries = 3;
int retryCount = 0;
bool messageSent = false;
String message = "EF01";
bool ack = false;

void setup() {
 //Sensor setup
 //pinMode(trigPin, OUTPUT);
 //pinMode(echoPin, INPUT);


 Serial.begin(57600);


 //LoRa setup
 loraSerial.setRxBufferSize(SER_BUF_SIZE);
 loraSerial.begin(BAUDRATE, SERIAL_8N1, RxPin, TxPin);
 loraSerial.setTimeout(1000);
 lora_autobaud();


 Serial.println("Initing LoRa");


 str = loraSerial.readStringUntil('\n');
 Serial.println(str);
 loraSerial.println("sys get ver");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("mac pause");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set mod lora"); 
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set freq 869400000");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set pwr 14");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set sf sf7");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set afcbw 41.7");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set rxbw 20.8");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set prlen 8");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set crc on");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set iqi off");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set cr 4/5");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set wdt 60000");  //disable for continuous reception
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set sync 12");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


 loraSerial.println("radio set bw 250");
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


  loraSerial.println("radio rx 0");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);

 Serial.println("starting loop");
}


void loop() {
 //set tid
 unsigned long currentTime = millis();


 // Tag sensor målinger hvert sekund og eval spot status
 /*if (currentTime - lastTime >= checkPeriod) {
   lastTime = currentTime;
   measureDistance();
   evaluateSpot();
 }
*/   

      if (!ack){
        Serial.println("in ACK checker");
        loraSerial.println("radio rxstop");
        str = loraSerial.readStringUntil('\n');
        //Serial.println(str);
        transmitMessage(message);

        loraSerial.println("radio rx 0"); 
        str = loraSerial.readStringUntil('\n');
        //Serial.println(str);
        delay(1000);
      }

       /*
 // transmission og retransmission af opdatering - tjekker tid for sidste retrans samt antal forsøg
 if (messageSent && (currentTime - lastTransmitTime > transmitInterval)) {
   if (retryCount < maxRetries) {
     transmitMessage(message);
     retryCount++;
   } else {
     Serial.println("Max retries reached, giving up on message.");
     retryCount = 0;
     messageSent = false;
   }
 }
*/ 

 if(loraSerial.available()){
    
    Serial.println("in loraSerial.available");
    str = loraSerial.readStringUntil('\r');
    loraSerial.readStringUntil('\n');
    String ACK = str.substring(10); 
    if(ACK.equals(message)){
      Serial.println("ACK confirmed");
      ack = true;
    }else{
    
    }

 }

 
}


//sensor funktion til måling af distance til objekt
void measureDistance() {
 digitalWrite(trigPin, LOW);
 delayMicroseconds(2);
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 distanceCm = duration * SOUND_SPEED / 2;
 Serial.print("Distance (cm): ");
 Serial.println(distanceCm);
}


void evaluateSpot() {
 bool previousStatus = spotstatus;


 // Sæt ny status
 bool newStatus = (distanceCm < 6);




 if (newStatus == previousStatus) {
   // reset counter hvis status ikke er ændret
   consistentCount = 0;
 } else {
   // Hvis status er ændret lig en til consistency counter
   consistentCount++;
   // Check om consistent nok gange
   if (consistentCount >= requiredConsistency) {
     // Ændre status
     spotstatus = newStatus;
     consistentCount = 0; // Reset consistency counter


     // Print ny status og klargør transmission med besked
     Serial.println(spotstatus ? "Spot is now occupied" : "Spot is now unoccupied");
     message = spotstatus ? "03" : "02"; //03 occ 02 ledig
     transmitMessage(message);
     messageSent = true;
     lastTransmitTime = millis();
   }
 }
}


//Besked funktion
void transmitMessage(String message) {
 Serial.print("Sending message: ");
 Serial.println(message);
 loraSerial.print("radio tx "); 
 loraSerial.println(message);
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);
 str = loraSerial.readStringUntil('\n');
 Serial.println(str);


}




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
