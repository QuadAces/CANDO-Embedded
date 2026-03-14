#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "esp_log.h"
#include <Arduino.h>
#include <HCSR04.h>
#include <Servo.h>
#include <time.h>
#include <eloquent_esp32cam.h>
#include <WiFi.h>
#include <HTTPClient.h>
// Great example - https://github.com/eloquentarduino/EloquentEsp32cam/blob/main/examples/Take_Picture/Take_Picture.ino
using eloq::camera;
//// begin nfc
static const char *TAG = "PN532";
const char *ssid = "wifi";
const char *password = "password";
const char *serverUrl = "http://your-server.com/upload";
// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK (1)
#define PN532_MOSI (42)
#define PN532_SS (2)
#define PN532_MISO (41)

#define PN532_IRQ (47)
#define PN532_RESET (3)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

const int buzzer = 8;
//// end nfc
const int trig_pin = 46;
const int echo_pin = 3;
const int servo_pin = 21;

const int led_pin = 35;

int pos = 0; // variable to store the servo position

Servo servo;
HCSR04 hc(46, 3); // initialisation class HCSR04 (trig pin , echo pin)
void setup()
{
  //// start servo
  servo.attach(servo_pin);
  //// end servo
  //// start nfc
  ESP_LOGI(TAG, "Hello!");

  nfc.begin();
  delay(5000);

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    ESP_LOGE(TAG, "Didn't find PN53x board");
    while (1)
      ; // halt
  }

  ESP_LOGI(TAG, "Found chip PN5%02X", (versiondata >> 24) & 0xFF);
  ESP_LOGI(TAG, "Firmware ver. %d.%d",
           (versiondata >> 16) & 0xFF,
           (versiondata >> 8) & 0xFF);

  // Set retry attempts
  nfc.setPassiveActivationRetries(0xFF);

  ESP_LOGI(TAG, "Waiting for an ISO14443A card");

  //// end nfc
  pinMode(echo_pin, INPUT);
  pinMode(trig_pin, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(trig_pin, LOW);
  digitalWrite(buzzer, LOW);

  // begin camera
  camera.pinout.freenove_s3();
  camera.brownout.disable();
  camera.resolution.vga();
  camera.quality.best();

  while (!camera.begin().isOk())
    Serial.println(camera.exception.toString());

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  Serial.begin(9600);
}

// if a card is scanned within the last minute, store it in memory
// any new card scanned overwrites the old card
// Process
// card scanned -> buzzer activates -> detect can -> rotate motor -> take photo (and flash light) and upload metadata
// OR
// detect can -> rotate motor -> take photo (and flash light) and upload metadata

// if there is more than 1 minute before card scan, set card to none

char uid_string[64] = {0};
char *ptr = uid_string;
time_t last_scanned = time(0);

void loop()
{

  //// start nfc
  boolean success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  // poll every X seconds
  if (success)
  {
    last_scanned = time(0);
    ESP_LOGI(TAG, "Found a card!");
    ESP_LOGI(TAG, "UID Length: %d bytes", uidLength);

    ptr = uid_string;
    for (uint8_t i = 0; i < uidLength; i++)
    {
      ptr += sprintf(ptr, " 0x%02X", uid[i]);
    }

    ESP_LOGI(TAG, "UID Value:%s", uid_string);
  }

  //// end nfc

  Serial.println(hc.dist());
  if (hc.dist() < 150)
  {
    // rotate
    servo.write(72);
    // flash
    digitalWrite(led_pin, HIGH);
    // take photo
    delay(500);
    if (!camera.capture().isOk())
    {
      Serial.println(camera.exception.toString());
      return;
    }

    // print image info
    // consider that esp_camera_fb_return() might need to be called to return memory back
    Serial.printf(
        "JPEG size in bytes: %d. Width: %dpx. Height: %dpx.\n",
        camera.getSizeInBytes(),
        camera.resolution.getWidth(),
        camera.resolution.getHeight());
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");
    int response = http.POST(camera.frame->buf, camera.frame->len);
    Serial.print("HTTP Response: ");
    Serial.println(response);
    http.end();
    // turn off flash
    digitalWrite(led_pin, LOW);

    // upload (check timestamp of last scanned card to see if its worth it)
  }

  delay(100);
}