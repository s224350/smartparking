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

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16);

void setup(void) {
  u8g2.begin();
  Serial.begin(9600);
}

void loop(void) {
  drawParkingSpots();
  delay(1000);
}

void drawParkingSpots() {
  u8g2.clearBuffer(); // Clear the buffer

  int displayWidth = u8g2.getDisplayWidth();
  int topRoad = u8g2.getDisplayHeight() / 2; // Base line in the middle of the display
  int buttRoad = u8g2.getDisplayHeight() - 3; // Bottom of the display, adjusted by -3
  int spotWidth = displayWidth / 4; 
  int spotHeight = 25; 
  int lineWidth = 3;
  int gapWidth = 5; // Gap between lines to create the dotted effect

  // Draw the roads
  u8g2.drawLine(0, topRoad, displayWidth-3, topRoad);//-3 cause skærm
  u8g2.drawLine(0, buttRoad, displayWidth-3, buttRoad);

  // Draw the dotted line
  for (int x = 0; x < displayWidth; x += (lineWidth + gapWidth)) {
    u8g2.drawLine(x, topRoad + ((buttRoad - topRoad) / 2), x + lineWidth, topRoad + ((buttRoad - topRoad) / 2));
  }

  // Draw the parking spots
  for (int i = 0; i < 4; i++) {
    int x = i * spotWidth + (spotWidth / 4); // Adding offset for gap
    u8g2.drawBox(x, 0, (spotWidth - (2 * (spotWidth / 4))), spotHeight);
  }
  
  u8g2.sendBuffer(); 
}
