/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>
#include <Servo.h>

#if __has_include("credentials.h")
#include "credentials.h"
#else
#define BLYNK_TOKEN ""
#define WIFI_SSID ""
#define WIFI_PASS ""
#endif

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = BLYNK_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

#define LED     D0

#define POWER_SERVO_IDLE 15
#define POWER_SERVO_PRESSED 70
#define START_SERVO_IDLE 180
#define START_SERVO_PRESSED 96

#define POWER_SERVO_DURATION 500
#define START_SERVO_DURATION 2000

#define STATE_IDLE 0
#define STATE_CONTRACTING 1

int powerState = STATE_IDLE;
unsigned long powerStartTime = 0;
int startState = STATE_IDLE;
unsigned long startStartTime = 0;
int startAfterPowerOn = 0;

Servo powerServo;
Servo startServo;

void contractPower() {
  powerState = STATE_CONTRACTING;
  powerStartTime = millis();
  powerServo.write(POWER_SERVO_PRESSED);
}

void contractStart() {
  startState = STATE_CONTRACTING;
  startStartTime = millis();
  startServo.write(START_SERVO_PRESSED);
}

// Full program: Turn on, then start
BLYNK_WRITE(V0) {
  if(param.asInt()) {
    contractPower();
    startAfterPowerOn = 1;
  }
}

// Only turn on
BLYNK_WRITE(V1) {
  if(param.asInt()) {
    contractPower();
    startAfterPowerOn = 0;
  }
}

// Only press start
BLYNK_WRITE(V2) {
  if(param.asInt()) {
    contractStart();
  }
}

// Move on/off arm manually
BLYNK_WRITE(V3) {
  powerServo.write(param.asInt());
}

// Move start arm manually
BLYNK_WRITE(V4) {
  startServo.write(param.asInt());
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  pinMode(LED, OUTPUT);

  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 443);

  powerServo.attach(D2);
  powerServo.write(POWER_SERVO_IDLE);
  
  startServo.attach(D3);
  startServo.write(START_SERVO_IDLE);
}

void loop()
{
  Blynk.run();

  switch(powerState) {
    case STATE_CONTRACTING:
      if(millis() >= powerStartTime + POWER_SERVO_DURATION) {
        powerState = STATE_IDLE;
        powerServo.write(POWER_SERVO_IDLE);

        if(startAfterPowerOn) {
          contractStart();
        }
      }
      break;
  }

  switch(startState) {
    case STATE_CONTRACTING:
      if(millis() >= startStartTime + START_SERVO_DURATION) {
        startState = STATE_IDLE;
        startServo.write(START_SERVO_IDLE);
      }
      break;
  }
}
