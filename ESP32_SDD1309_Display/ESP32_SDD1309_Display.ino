#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

//Pin connections using ESP32
//From  --> SSD1309 -> ESP32
//GND -> GND
//VCC -> 3V
//SCL -> D22
//SDA -> D21
//RES -> Rx2/GPIO16



U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/16);

void Spot(num) {
  
  u8g2.sendBuffer();
}



void setup(void) {
  u8g2.begin();
}

void loop(void) {

  displayStopSign();
  delay(1000);
  displayCarAnimation();
}
