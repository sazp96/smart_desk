// --------------------------------------------
//             Smart Desk V1.0.2
// Smart desk controlled by Alexa through IFTTT
// Written but not maintained by: Sergio Zamora
// --------------------------------------------

PRODUCT_ID(3893); // replace by your product ID
PRODUCT_VERSION(3); // increment each time you upload to the console

SYSTEM_MODE(AUTOMATIC);
STARTUP( pinIni() );

#include "neopixel.h"

#define standPin D1
#define sitPin D2
#define buzzerPin A5
#define buzzerTimmer 200  //milliseconds
#define movementTimmer 12000  //milliseconds
#define ledPin A4
#define ledCount 2
#define ledType WS2812B
bool isStanding = false; //during setup, desk initializes to stand
int standTarget = 30; //minutes per hour
int delayForAutoStand = 1; //minutes; Not doing auto sit due to risk of crushing something.
int timeLastChange = 0; //seconds
int minInMode = 0; //user friendly time in minutes
bool notificationSent = false;
bool areNotificationsOn = true;

Adafruit_NeoPixel strip(ledCount, ledPin, ledType);

void pinIni() {
  pinMode(standPin, INPUT_PULLDOWN);
  pinMode(sitPin, INPUT_PULLDOWN);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
}

void warningSounds(int beeps) {
  analogWrite(buzzerPin, 5);
  delay(buzzerTimmer);
  analogWrite(buzzerPin, 0);

  if (beeps == 2) {
    delay(buzzerTimmer / 2);
    analogWrite(buzzerPin, 10);
    delay(buzzerTimmer);
    analogWrite(buzzerPin, 0);
  }
}

int changeMode(String mode) {
  int pin = D0;
  warningSounds(2);
  if (mode.equals("sit") && isStanding) {
    isStanding = false;
    pin = sitPin;
    strip.setPixelColor(1, 10, 0, 0); //red
    strip.setPixelColor(0, 255, 0, 0);
  } else if (mode.equals("stand") && !isStanding) {
    isStanding = true;
    pin = standPin;
    strip.setPixelColor(1, 0, 255, 0); //green
    strip.setPixelColor(0, 0, 10, 0);
  }
  strip.show();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(movementTimmer);

  timeLastChange = Time.now();
  pinMode(pin, INPUT_PULLDOWN);
  notificationSent = false;
  strip.setPixelColor(1, 0, 0, 0);
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
  return 0;
}

int setStandTargetAndRestart(String target) {
  int temp = target.toInt();
  if ((temp > 0) && (temp < 60)) {
    warningSounds(1);
    standTarget = temp;
    timeLastChange = Time.now();
    areNotificationsOn = true;
    notificationSent = false;
    strip.setPixelColor(1, 0, 0, 0);
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
  }
  return 0;
}

int turnNotificationsOff (String nothing) {
  warningSounds(1);
  areNotificationsOn = false;
  notificationSent = false;
  strip.setPixelColor(1, 0, 0, 0);
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
  return 0;
}

void setup() {
  Particle.function("changeMode", changeMode); //Move the desk to sit or stand mode
  Particle.function("setTarget", setStandTargetAndRestart); //Set the stand target and restarts the minutes in mode (stand/sit) to zero
  Particle.function("turnNotiOff", turnNotificationsOff); //Turns notification system off
  Particle.variable("isStanding", isStanding);
  Particle.variable("minInMode", minInMode);
  Particle.variable("standTarget", standTarget);
  Particle.variable("NotiStatus", areNotificationsOn);
  strip.begin();
  strip.show();
  changeMode("stand");
}

void loop() {
  minInMode = (Time.now() - timeLastChange) / 60;

  if (areNotificationsOn) {
    if (isStanding) {
      if (minInMode >= standTarget) { // If you have been standing longer than target
        if (!notificationSent) { // Only tell me once
          notificationSent = true;
          warningSounds(1);
          strip.setPixelColor(1, 10, 0, 0); //red
          strip.setPixelColor(0, 10, 0, 0);
          strip.begin();
          strip.show();
        }
      }
    } else {
      if (minInMode >= (60 - standTarget)) { // If you have been sitting longer than target
        if (!notificationSent) { // Only tell me once
          notificationSent = true;
          warningSounds(1);
          strip.setPixelColor(1, 0, 10, 0); //green
          strip.setPixelColor(0, 0, 10, 0);
          strip.begin();
          strip.show();
        } else if (minInMode >= (60 - standTarget + delayForAutoStand)) { // Auto stand 1 minute after notification set. Not doing auto sit due to risk of crushing something.
          changeMode("stand");
        }
      }
    }
  }
}
