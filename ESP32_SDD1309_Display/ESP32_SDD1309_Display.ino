#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// So the point of this program is to run a SSD1309 screen for a parking gate. 
// The display should show the welcome screen per default when there is no interaction yet. 
// In the current state of the code. The program take input from the serial if that input is between 1-4 (including 1 & 4)
// it will then run the drawParkingSpots() that takes in two inputs. The parking spot and the start time. Since this function as of right now 
// blinks at the parking spot being called it blinks for 3 seconds which is why it needs a start time to make sure it only blinks for 3 seconds. 
// If the serial input is something other than 1, 2, 3 or 4 the display will show "No Access" from the noAccess() function. 


// Pin connections using ESP32
// From SSD1309 -> ESP32
// GND     -> GND
// VCC     -> 3V
// SCL     -> D22
// SDA     -> D21
// RES     -> Rx2/GPIO16

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,16);

unsigned long blinkStartTime = 0;  // Timestamp when the blinking starts

void setup(void) {
  u8g2.begin();
  Serial.begin(9600);
}

void loop(void) {
  static int selectedSpot = 0;
  Welcome();

  if (Serial.available()) {
    //Couldnt just parseInt since the new line fucked it uup so now I take in a string and convert it. 
    String inputString = Serial.readStringUntil('\n'); 
    inputString.trim(); 
    int input = inputString.toInt(); 

    if (input >= 1 && input <= 4) {
      selectedSpot = input;
      Serial.println(selectedSpot);
      blinkStartTime = millis(); // Start the blink timer
      drawParkingSpots(selectedSpot, blinkStartTime);
    }
    else {
      noAccess();
    }
    delay(2000);
  }
}


void drawParkingSpots(int selectedSpot, unsigned long startTime) {
  unsigned long endTime = startTime + 3000;  // Set end time for 3 seconds later

  while (millis() < endTime) {
    u8g2.clearBuffer();  // Clear the buffer each cycle

    int displayWidth = u8g2.getDisplayWidth();
    int topRoad = u8g2.getDisplayHeight() / 2;
    int buttRoad = u8g2.getDisplayHeight() - 3; //Had to subtract 3 since the display was bugging and the line carried over. 
    int spotWidth = displayWidth / 4;
    int spotHeight = 25;
    int textOffsetX = (spotWidth / 4) - 3;
    int textOffsetY = (spotHeight / 2) - 5; 

    // Draw the road lines
    u8g2.drawLine(0, topRoad, displayWidth - 3, topRoad);
    u8g2.drawLine(0, buttRoad, displayWidth - 3, buttRoad);

    // Draw the dotted line in the middle of the road
    for (int x = 0; x < displayWidth; x += (3 + 5)) {
      u8g2.drawLine(x, topRoad + ((buttRoad - topRoad) / 2), x + 3, topRoad + ((buttRoad - topRoad) / 2));
    }

    u8g2.setFont(u8g2_font_ncenB08_tr);

    // Draw the parking spots and handle the blinking
    for (int i = 0; i < 4; i++) {
      char spotLabel[3];
      sprintf(spotLabel, "%d", i + 1);//plus 1 since the parking spots are from 1-4

      int x = i * spotWidth + (spotWidth / 4);
      u8g2.drawFrame(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
      u8g2.drawStr(x + textOffsetX, topRoad - textOffsetY, spotLabel);  //Puts in the parking spot numbers. 

      if (i + 1 == selectedSpot) {
        if ((millis() - startTime) / 100 % 2 == 0) { //Blink every 100 ms
          u8g2.setDrawColor(0);//Blinking is done by turning it black
          u8g2.drawBox(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
          u8g2.setDrawColor(1);
        }
      }
    }
    u8g2.sendBuffer();//send it     
  }
}


void Welcome() {
  u8g2.clearBuffer();                  
  u8g2.setFont(u8g2_font_ncenB14_tr);  
  u8g2.drawStr(12, 32, "Welcome!");    
  u8g2.sendBuffer();                   
}

void noAccess() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(12, 32, "No Access");
  u8g2.sendBuffer();
}