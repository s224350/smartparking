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
  // Array representing the blinking state of parking spots (1 = no blink, 0 = blink)
  int occSpots[4] = {0, 1, 0, 1};  // Only spot 3 will blink
  unsigned long blinkStartTime = 0;  // Time variable to record the start time for blinking

  Welcome();
  Serial.println("Hello idiot");

  if (Serial.available()) {
    // Reading a number from the serial monitor
    String inputString = Serial.readStringUntil('\n');
    
    inputString.trim();
    int input = inputString.toInt();

    // Validate the input (input is ignored in your current request but might be used to trigger the action)
    if (input >= 1 && input <= 4) {
      blinkStartTime = millis();  // Capture the current time
      Serial.println("Triggering the display of parking spots.");
      drawParkingSpots(occSpots, blinkStartTime);
    } else {
      noAccess();  // Handle invalid input
    }

    //delay(2000); // Delay to avoid overwhelming the serial input
  }
}



void drawParkingSpots(int occSpots[], unsigned long startTime) {
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