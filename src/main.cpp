// #include <Wire.h>
// #include <SPI.h>
// #include <Adafruit_PN532.h>
// #include "esp_log.h"
// #include <Arduino.h>
// #include <HCSR04.h>
// #include <Servo.h>
// #include <time.h>
// #include <eloquent_esp32cam.h>
// #include <WiFi.h>
// #include <HTTPClient.h>
// #include <cJSON.h>

// // Great example - https://github.com/eloquentarduino/EloquentEsp32cam/blob/main/examples/Take_Picture/Take_Picture.ino
// using eloq::camera;
// //// begin nfc
// static const char *TAG = "PN532";

// template <typename T>
// void espLogIPrintln(const T &value)
// {
//   String text = String(value);
//   ESP_LOGI(TAG, "%s", text.c_str());
// }

// #define ESPLOGI(value) espLogIPrintln(value)
// const char *ssid = "Wolfy";
// const char *password = "Abcdefgh";
// const char *serverUrl = "https://defa-129-94-128-29.ngrok-free.app";
// // If using the breakout with SPI, define the pins for SPI communication.
// #define PN532_SCK (1)
// #define PN532_MOSI (42)
// #define PN532_SS (2)
// #define PN532_MISO (41)

// #define PN532_IRQ (47)
// #define PN532_RESET (3)

// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// const int buzzer = 8;
// //// end nfc
// const int trig_pin = 46;
// const int echo_pin = 3;
// const int servo_pin = 21;

// const int led_pin = 35;

// int pos = 0; // variable to store the servo position

// struct HTTP_Data
// {
//   camera_fb_t *frame;
//   char uid[64];
// };

// Servo servo;
// HCSR04 hc(46, 3); // initialisation class HCSR04 (trig pin , echo pin)

// void sendHTTP(struct HTTP_Data *data, const char *url)
// {
  
// }
// void setup()
// {
//   pinMode(led_pin, OUTPUT);
//   //// start servo
//   servo.attach(servo_pin);
//   //// end servo
//   //// start nfc
//   ESP_LOGI(TAG, "Hello!");

//   nfc.begin();
//   delay(5000);

//   uint32_t versiondata = nfc.getFirmwareVersion();
//   if (!versiondata)
//   {
//     ESP_LOGE(TAG, "Didn't find PN53x board");
//     while (1)
//       ; // halt
//   }

//   ESP_LOGI(TAG, "Found chip PN5%02X", (versiondata >> 24) & 0xFF);
//   ESP_LOGI(TAG, "Firmware ver. %d.%d",
//            (versiondata >> 16) & 0xFF,
//            (versiondata >> 8) & 0xFF);

//   // Set retry attempts
//   nfc.setPassiveActivationRetries(0xFF);

//   ESP_LOGI(TAG, "Waiting for an ISO14443A card");

//   //// end nfc
//   pinMode(echo_pin, INPUT);
//   pinMode(trig_pin, OUTPUT);
//   pinMode(buzzer, OUTPUT);

//   digitalWrite(trig_pin, LOW);
//   digitalWrite(buzzer, LOW);

//   // begin camera
//   camera.pinout.freenove_s3();
//   camera.brownout.disable();
//   camera.resolution.vga();
//   camera.quality.best();
//   camera.config.fb_location = CAMERA_FB_IN_DRAM;

  
//   while (!camera.begin().isOk()) {

//     ESPLOGI("locked here");
//     ESPLOGI(camera.exception.toString());
//   }

//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED)
//     delay(500);

//   Serial.begin(9600);
// }

// // if a card is scanned within the last minute, store it in memory
// // any new card scanned overwrites the old card
// // Process
// // card scanned -> buzzer activates -> detect can -> rotate motor -> take photo (and flash light) and upload metadata
// // OR
// // detect can -> rotate motor -> take photo (and flash light) and upload metadata

// // if there is more than 1 minute before card scan, set card to none

// char uid_string[64] = {0};
// char *ptr = uid_string;
// time_t last_scanned = time(0);

// void loop()
// {
//   ESPLOGI("Looping..."); 
//   //// start nfc
//   boolean success;
//   uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
//   uint8_t uidLength;

//   success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

//   // poll every X seconds
//   if (success)
//   {
//     last_scanned = time(0);
//     ESP_LOGI(TAG, "Found a card!");
//     ESP_LOGI(TAG, "UID Length: %d bytes", uidLength);

//     ptr = uid_string;
//     for (uint8_t i = 0; i < uidLength; i++)
//     {
//       ptr += sprintf(ptr, " 0x%02X", uid[i]);
//     }

//     ESP_LOGI(TAG, "UID Value:%s", uid_string);
//   }

//   //// end nfc

//   ESPLOGI(hc.dist());
//   if (hc.dist() < 150)
//   {
//     // rotate
//     servo.write(72);
//     // flash
//     digitalWrite(led_pin, HIGH);
//     // take photo
//     delay(50);
//     if (!camera.capture().isOk())
//     {
//       ESPLOGI(camera.exception.toString());
//       return;
//     }

//     // print image info
//     // consider that esp_camera_fb_return() might need to be called to return memory back
//     Serial.printf(
//         "JPEG size in bytes: %d. Width: %dpx. Height: %dpx.\n",
//         camera.getSizeInBytes(),
//         camera.resolution.getWidth(),
//         camera.resolution.getHeight());

//     // turn off flash
//     digitalWrite(led_pin, LOW);
//     HTTP_Data data;
//     data.frame = camera.frame;

//     if (difftime(last_scanned, time(0)) < 60)
//     {
//       strcpy(data.uid, uid_string);
//     }
//     else
//     {
//       data.uid[0], '\0';
//     }
//     ESPLOGI("Uploading...");
//     sendHTTP(&data, serverUrl);
//     esp_camera_fb_return(camera.frame);

//     // upload (check timestamp of last scanned card to see if its worth it)
//   }

//   delay(100);
// }

#include <Arduino.h>
#include <eloquent_esp32cam.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_log.h"

using eloq::camera;

static const char *TAG = "CANDO";

template <typename T>
void espLogIPrintln(const T &value)
{
  String text = String(value);
  ESP_LOGI(TAG, "%s", text.c_str());
}

#define ESPLOGI(value) espLogIPrintln(value)

const char *ssid = "Wolfy";
const char *password = "Abcdefgh";
const char *post_url = "http://astrometrical-gwyneth-oafishly.ngrok-free.dev/upload-image";

void setup()
{
  Serial.begin(115200);

  // begin camera (freenove S3 pinout)
  camera.pinout.freenove_s3();
  camera.brownout.disable();
  camera.resolution.vga();
  camera.quality.best();
  camera.config.fb_location = CAMERA_FB_IN_DRAM;

  while (!camera.begin().isOk())
  {
    ESPLOGI(camera.exception.toString());
  }

  // connect to WiFi
  ESPLOGI("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  ESPLOGI("WiFi connected");
  ESPLOGI(WiFi.localIP());
}

void loop()
{
  // take photo
  if (!camera.capture().isOk())
  {
    ESPLOGI(camera.exception.toString());
    delay(1000);
    return;
  }

  Serial.printf("JPEG size: %d bytes. Width: %dpx. Height: %dpx.\n",
                camera.getSizeInBytes(),
                camera.resolution.getWidth(),
                camera.resolution.getHeight());

  // POST image to server
  HTTPClient http;
  http.begin(post_url);
  int httpCode = http.sendRequest("POST", camera.frame->buf, camera.frame->len);

  if (httpCode > 0)
  {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK)
      ESPLOGI(http.getString());
  }
  else
  {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  esp_camera_fb_return(camera.frame);

  delay(5000);
}
