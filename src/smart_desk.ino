// --------------------------------------------
//             Smart Desk V1.1.0
// Smart desk controlled by Alexa through IFTTT
// Written but not maintained by: Sergio Zamora
// --------------------------------------------

PRODUCT_ID(3893); // replace by your product ID
PRODUCT_VERSION(4); // increment each time you upload to the console
#define PRODUCT_VERSION_STRING "4"

SYSTEM_MODE(AUTOMATIC);
STARTUP( pinIni() );

#include "neopixel.h"

#define standPin D1
#define sitPin D2
#define buzzerPin A5
#define buzzerTimmer 200  //milliseconds
#define movementTimmerDefault 13000  //milliseconds
#define ledPin A4
#define ledCount 2
#define ledType WS2812B
#define motionPin D7
#define errorMargin 1 // minutes

int movementTimmer = movementTimmerDefault;
bool isStanding = false; //during setup, desk initializes to stand
int standTarget = 30; //minutes per hour
int delayForAutoStand = 1; //minutes; Not doing auto sit due to risk of crushing something.
int timeLastChange = 0; //seconds
int minInMode = 0; //user friendly time in minutes
bool notificationSent = false;
bool areNotificationsOn = true;
int lastCount = 0;
int motionDetected = false;
int standingMinutes = 0;
int sittingMinutes = 0;
int idleMinutes = 0;
int averageMinStanding = 0;
String statusMessage = "";

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
  } else if (mode.equals("slack") && isStanding) {
    isStanding = false;
    pin = sitPin;
    strip.setPixelColor(1, 0, 0, 10); //blue
    strip.setPixelColor(0, 0, 0, 255);
    movementTimmer = 4000;
  }
  strip.show();
  warningSounds(2);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(movementTimmer);

  pinMode(pin, INPUT_PULLDOWN);
  Particle.publish("changeMode", mode);
  initializeSystem();
  return 0;
}

int setStandTargetAndRestart(String target) {
  int temp = target.toInt();
  if ((temp > 0) && (temp < 60)) {
    warningSounds(1);
    standTarget = temp;
    areNotificationsOn = true;
    initializeSystem();
  }
  return 0;
}

int turnNotificationsOff (String nothing) {
  warningSounds(1);
  areNotificationsOn = false;
  initializeSystem();
  return 0;
}

void initializeSystem() {
  movementTimmer = movementTimmerDefault;
  timeLastChange = Time.now();
  notificationSent = false;
  strip.setPixelColor(1, 0, 0, 0);
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
}

void setup() {
  Particle.publish("Firmware Version", PRODUCT_VERSION_STRING);
  Particle.function("changeMode", changeMode); //Move the desk to sit or stand mode
  Particle.function("setTarget", setStandTargetAndRestart); //Set the stand target and restarts the minutes in mode (stand/sit) to zero
  Particle.function("turnNotiOff", turnNotificationsOff); //Turns notification system off
  Particle.variable("isStanding", isStanding);
  Particle.variable("minInMode", minInMode);
  Particle.variable("standTarget", standTarget);
  Particle.variable("NotiStatus", areNotificationsOn);
  Particle.variable("statusMess", statusMessage);
  Particle.variable("stanM", standingMinutes);
  Particle.variable("sittM", sittingMinutes);
  Particle.variable("idleM", idleMinutes);
  Particle.variable("averM", averageMinStanding);
  strip.begin();
  strip.show();
  changeMode("stand");
}

void loop() {
  //Detecting motion
  if (digitalRead(motionPin) == HIGH)
    motionDetected = true;

  if ((Time.now() - lastCount) > 60) { //Do this once every 60 seconds
    lastCount = Time.now();

    //Tracking time in different states
    if (motionDetected) {
      if (isStanding)
        standingMinutes++;
      else
        sittingMinutes++;
      motionDetected = false;
    } else {
      if ((standingMinutes != 0) || (sittingMinutes != 0)) //Only start counting idle time once we know the user is present
        idleMinutes++;
      if (idleMinutes > (6 * 60)) { //Assuming that if the user is gone for 6 hours it is done for the day.
        standingMinutes = 0;
        sittingMinutes = 0;
        idleMinutes = 0;
      }
    }

    //Calculating performance vs target
    if ((standingMinutes + sittingMinutes) >= 1) {
      averageMinStanding = (standingMinutes * 60) / (standingMinutes + sittingMinutes);
      if ((averageMinStanding < (standTarget + errorMargin)) && (averageMinStanding > (standTarget - errorMargin)))
        statusMessage = "You are doing great! On average, you are standing " + String(averageMinStanding) + " minutes every hour. Keep up the good work!";
      else if (averageMinStanding > standTarget)
        statusMessage = "You are standing too much! On average, you are standing " + String(averageMinStanding) + " minutes every hour. Try sitting a bit more.";
      else if (averageMinStanding < standTarget)
        statusMessage = "You are sitting too much! On average, you are sitting " + String(60 - averageMinStanding) + " minutes every hour. Try standing a bit more.";
    } else {
      statusMessage = "You need to use your smart desk for at least 5 minutes before you can check your performance.";
    }
  }

  //Sending notifications to keep the user on track for their goal
  minInMode = (Time.now() - timeLastChange) / 60;

  if (areNotificationsOn) {
    if (isStanding) {
      if (minInMode >= standTarget) { // If you have been standing longer than target
        if (!notificationSent) { // Only tell me once
          notificationSent = true;
          strip.setPixelColor(1, 10, 0, 0); //red
          strip.setPixelColor(0, 10, 0, 0);
          strip.show();
          warningSounds(1);
        }
      }
    } else {
      if (minInMode >= (60 - standTarget)) { // If you have been sitting longer than target
        if (!notificationSent) { // Only tell me once
          notificationSent = true;
          strip.setPixelColor(1, 0, 10, 0); //green
          strip.setPixelColor(0, 0, 10, 0);
          strip.show();
          warningSounds(1);
        } else if (minInMode >= (60 - standTarget + delayForAutoStand)) { // Auto stand 1 minute after notification set. Not doing auto sit due to risk of crushing something.
          changeMode("stand");
        }
      }
    }
  }
}
