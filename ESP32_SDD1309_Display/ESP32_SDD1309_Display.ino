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

unsigned long blinkStartTime = 0; // Timestamp when the blinking starts

void setup(void) {
  u8g2.begin();
  Serial.begin(9600);
}

void loop(void) {
  static int selectedSpot = 0;
  if (Serial.available()) {
    int input = Serial.parseInt();
    if (input >= 1 && input <= 4) {
      selectedSpot = input;
      blinkStartTime = millis(); // Start the blink timer
    }
  }
  drawParkingSpots(selectedSpot, blinkStartTime);
  delay(100); // Delay adjusted for faster loop execution, hence faster blinking
}

void drawParkingSpots(int selectedSpot, unsigned long startTime) {
  u8g2.clearBuffer();  // Clear the buffer

  int displayWidth = u8g2.getDisplayWidth();
  int topRoad = u8g2.getDisplayHeight() / 2;
  int buttRoad = u8g2.getDisplayHeight() - 3;
  int spotWidth = displayWidth / 4;
  int spotHeight = 25;
  int textOffsetX = (spotWidth / 4) - 3; // X offset from the beginning of each parking spot
  int textOffsetY = (spotHeight / 2) - 5; // Y offset from the top of the parking spot

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
    sprintf(spotLabel, "%d", i+1);

    int x = i * spotWidth + (spotWidth / 4);
    u8g2.drawFrame(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
    u8g2.drawStr(x + textOffsetX, topRoad - textOffsetY, spotLabel); // Text is placed in the original position

    if (i + 1 == selectedSpot && (millis() - startTime) < 3000) {
      if ((millis() / 100) % 2 == 0) { // faster blink
        u8g2.setDrawColor(0); // Make black
        u8g2.drawBox(x, topRoad - spotHeight + 1, spotWidth - (2 * (spotWidth / 4)), spotHeight);
        u8g2.setDrawColor(1);
      }
    }
  }
  u8g2.sendBuffer();
}
