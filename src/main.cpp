#include <Arduino.h>

/* ---------------------------- Include Embedded ---------------------------- */

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <HCSR04.h>
#include <Servo.h>
#include <time.h>
#include <cJSON.h>

/* ------------------------- Include Camera and HTTP ------------------------ */

#include <eloquent_esp32cam.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_log.h"

/* -------------------------------- Embedded -------------------------------- */

#define LED_PIN 35
#define BUZZER_PIN 8
#define TRIG_PIN 46
#define ECHO_PIN 3
#define SERVO_PIN 21

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK (1)
#define PN532_MOSI (42)
#define PN532_SS (2)
#define PN532_MISO (41)

#define PN532_IRQ (47)
#define PN532_RESET (3)

Servo servo;
HCSR04 ultrasonic(46, 3); // initialisation class HCSR04 (trig pin , echo pin)
int pos = 0; // variable to store the servo position

/* ----------------------------------- NFC ---------------------------------- */

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
char uid_string[64] = {0};
char *ptr = uid_string;
time_t last_scanned = time(0);

/* ----------------------------- Camera and HTTP ---------------------------- */

// Great example - https://github.com/eloquentarduino/EloquentEsp32cam/blob/main/examples/Take_Picture/Take_Picture.ino

using eloq::camera;

static const char *TAG = "CANDO";

template <typename T>
void espLogIPrintln(const T &value) {
  String text = String(value);
  ESP_LOGI(TAG, "%s", text.c_str());
}

#define ESPLOGI(value) espLogIPrintln(value)

const char *ssid = "Wolfy";
const char *password = "Abcdefgh";
const char *post_url = "http://astrometrical-gwyneth-oafishly.ngrok-free.dev/upload-image";

struct HTTP_Data {
  camera_fb_t *frame;
  char uid[64];
};

/* -------------------------------------------------------------------------- */
/*                                   SET UP                                   */
/* -------------------------------------------------------------------------- */

// embedded previously had Serial.begin(9600);
void setup() {
  Serial.begin(115200);
  
  /* -------------------------------- Embedded -------------------------------- */

  pinMode(LED_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  servo.attach(SERVO_PIN);

  /* ----------------------------------- NFC ---------------------------------- */

  nfc.begin();
  delay(5000);

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    ESP_LOGE(TAG, "Didn't find PN53x board");
    while (1); // halt
  }

  ESP_LOGI(TAG, "Found chip PN5%02X", (versiondata >> 24) & 0xFF);
  ESP_LOGI(TAG, "Firmware ver. %d.%d", (versiondata >> 16) & 0xFF,  (versiondata >> 8) & 0xFF);

  // Set retry attempts
  nfc.setPassiveActivationRetries(0xFF);

  ESP_LOGI(TAG, "Waiting for an ISO14443A card");

  /* ----------------------------- CAMERA AND HTTP ---------------------------- */

  // begin camera (freenove S3 pinout)
  camera.pinout.freenove_s3();
  camera.brownout.disable();
  camera.resolution.vga();
  camera.quality.best();
  camera.config.fb_location = CAMERA_FB_IN_DRAM;

  while (!camera.begin().isOk()) {
    ESPLOGI(camera.exception.toString());
  }

  // Connect to WiFi
  ESPLOGI("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  ESPLOGI("WiFi connected");
  ESPLOGI(WiFi.localIP());
}

/* -------------------------------------------------------------------------- */
/*                                    LOOP                                    */
/* -------------------------------------------------------------------------- */

// if a card is scanned within the last minute, store it in memory
// any new card scanned overwrites the old card
// Process
// card scanned -> buzzer activates -> detect can -> rotate motor -> take photo (and flash light) and upload metadata
// OR
// detect can -> rotate motor -> take photo (and flash light) and upload metadata

// if there is more than 1 minute before card scan, set card to none

void loop()
{
  /* ----------------------------------- NFC ---------------------------------- */
  
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;
  uint16_t timeout = 1000; // 1 second (i think)

  // ########################################## code hangs here atm
  boolean nfc_success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeout);

  // poll every X seconds
  if (nfc_success) {
    last_scanned = time(0);
    ESP_LOGI(TAG, "Found a card!");
    ESP_LOGI(TAG, "UID Length: %d bytes", uidLength);

    ptr = uid_string;
    for (uint8_t i = 0; i < uidLength; i++) {
      ptr += sprintf(ptr, " 0x%02X", uid[i]);
    }

    ESP_LOGI(TAG, "UID Value:%s", uid_string);
  }

  /* ----------------------------- CHECK DISTANCE ----------------------------- */

  // if distance is too far, end early (15cm)
  if (ultrasonic.dist() > 15) {
    return;
  }

  /* ------------- vvvvvvvvvv IF ULTRASONIC DETECTS CAN vvvvvvvvvv ------------ */

  // rotate servo
  servo.write(72);
  delay(2000);

  // turn on flash
  digitalWrite(LED_PIN, HIGH);
  
  // take photo
  if (!camera.capture().isOk()) {
    ESPLOGI(camera.exception.toString());
    delay(1000);
    return;
  }

  // turn off flash
  digitalWrite(LED_PIN, LOW);

  // print image info
  Serial.printf("JPEG size: %d bytes. Width: %dpx. Height: %dpx.\n", camera.getSizeInBytes(), camera.resolution.getWidth(), camera.resolution.getHeight());

  // some value
  servo.write(0);

  /* -------------------------- POST IMAGE TO SERVER -------------------------- */

  // upload (check timestamp of last scanned card to see if its worth it)

  // attach header 
  // HTTP_Data data;
  // data.frame = camera.frame;
  // if (difftime(last_scanned, time(0)) < 60) {
  //   strcpy(data.uid, uid_string);
  // }
  // else {
  //   data.uid[0], '\0';
  // }

  HTTPClient http;
  http.begin(post_url);
  int httpCode = http.sendRequest("POST", camera.frame->buf, camera.frame->len);

  if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      ESPLOGI(http.getString());
    }
  }
  else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  
  // consider that esp_camera_fb_return() might need to be called to return memory back
  esp_camera_fb_return(camera.frame);
  delay(5000);
}