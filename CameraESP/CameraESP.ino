#include "esp_camera.h"

// Define camera variables
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#define FRAMESIZE_96x96   7
#define flashLED_Channel 10
#define photoinput_pin 14
int sensorValue;
void setup() {
  Serial.begin(115200);

  pinMode(photoinput_pin, INPUT);

  // Camera setup
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_VGA;
  //formats
  //FRAMESIZE_UXGA (1600 x 1200)
  //FRAMESIZE_QVGA (320 x 240)
  //FRAMESIZE_CIF (352 x 288)
  //FRAMESIZE_VGA (640 x 480)
  //FRAMESIZE_SVGA (800 x 600)
  //FRAMESIZE_XGA (1024 x 768)
  //FRAMESIZE_SXGA (1280 x 1024)

  //config.pixel_format = PIXFORMAT_JPEG; //YUV422,GRAYSCALE,RGB565,JPEG

    config.jpeg_quality = 40; //63 lowest quality and 10 is the lowest
    config.fb_count = 2;
    
      //initialiser flash LED
      ledcSetup(flashLED_Channel, 5000, 8); 
      ledcAttachPin(4, flashLED_Channel);

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

}

void loop() {

  if(Serial.available()){
    byte buffer =Serial.read();
    if(buffer == 98){
          sensorValue = analogRead(photoinput_pin); //read analog photoresistor data

          if (sensorValue > 3000){ //flashLED activation threshold
            sensorValue = 3000;
          }

          sensorValue = map(sensorValue,0,3000 ,255,0); //mapping from 0-3000 to 255-0 (meaning values are also reversed)
        captureAndProcessImage(sensorValue); 
    }

  }

}

void captureAndProcessImage(int LED_strength) { //denne funktion tager et billede og deallokere fra frame bufferen
  camera_fb_t *fb = NULL;

  ledcWrite(flashLED_Channel, LED_strength); //turn on flashLED
  delay(150);
  fb = esp_camera_fb_get(); //take picture
  
  ledcWrite(flashLED_Channel, 0); //turn off flashLED
  
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }


  uint8_t *imageBuffer = fb->buf;
  size_t imageLen = fb->len;
  long image_size = (long) imageLen;
  //get image length into correct byte format
  byte imageSizeBytes[4];
  imageSizeBytes[0] = image_size & 0xff;
  imageSizeBytes[1] = image_size >> 8 & 0xff;
  imageSizeBytes[2] = image_size >> 16 & 0xff;
  imageSizeBytes[3] = image_size >> 24 & 0xff;
  Serial.write(imageSizeBytes,4);


  while(Serial.read() != 99){ //wait for signal to send image data
    delay(1);
  }
  for (size_t i = 0; i < imageLen; i++) { //send image data

  Serial.write(imageBuffer[i]);

  }
  
  
  esp_camera_fb_return(fb); //deallocation of framebuffer
}


