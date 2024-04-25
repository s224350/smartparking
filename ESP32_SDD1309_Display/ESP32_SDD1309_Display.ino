#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Pin connections using ESP32
// From    --> SSD1309 -> ESP32
// GND     -> GND
// VCC     -> 3V
// SCL     -> D22
// SDA     -> D21
// RES     -> Rx2/GPIO16

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/16);

unsigned long blinkStartTime = 0;  // Timestamp when the blinking starts

void setup(void) {
  u8g2.begin();
  Serial.begin(9600);
}

void loop(void) {
  static int selectedSpot = 0;
  Welcome();

  if (Serial.available()) {
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
    u8g2.clearBuffer();  // Clear the buffer each cycle to redraw

    int displayWidth = u8g2.getDisplayWidth();
    int topRoad = u8g2.getDisplayHeight() / 2;
    int buttRoad = u8g2.getDisplayHeight() - 3;
    int spotWidth = displayWidth / 4;
    int spotHeight = 25;
    int textOffsetX = (spotWidth / 4) - 3;   // X offset from the beginning of each parking spot
    int textOffsetY = (spotHeight / 2) - 5;  // Y offset from the top of the parking spot

    // Draw the roads
    u8g2.drawLine(0, topRoad, displayWidth - 3, topRoad);
    u8g2.drawLine(0, buttRoad, displayWidth - 3, buttRoad);

    // Draw the dotted line
    for (int x = 0; x < displayWidth; x += (3 + 5)) {
      u8g2.drawLine(x, topRoad + ((buttRoad - topRoad) / 2), x + 3, topRoad + ((buttRoad - topRoad) / 2));
    }

    u8g2.setFont(u8g2_font_ncenB08_tr);

    // Draw the parking spots and handle the blinking
    for (int i = 0; i < 4; i++) {
      char spotLabel[3];
      sprintf(spotLabel, "%d", i + 1);

      int x = i * spotWidth + (spotWidth / 4);
      u8g2.drawFrame(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
      u8g2.drawStr(x + textOffsetX, topRoad - textOffsetY, spotLabel);  // Text is placed in the original position

      if (i + 1 == selectedSpot) {
        if ((millis() - startTime) / 100 % 2 == 0) {  // Blink every 100 ms
          u8g2.setDrawColor(0);                       // Toggle to black to create a blinking effect
          u8g2.drawBox(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
          u8g2.setDrawColor(1);  // Reset draw color to white
        }
      }
    }

    u8g2.sendBuffer();  // Send the buffer to the display
    delay(10);          // Short delay to reduce CPU load and slow down the loop a bit
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