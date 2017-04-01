// --------------------------------------------
//              Smart Desk V1.0
// Smart desk controlled by Alexa through IFTTT
// Written but not maintained by: Sergio Zamora
// --------------------------------------------

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

Adafruit_NeoPixel strip(ledCount, ledPin, ledType);

void pinIni() {
  pinMode(standPin, OUTPUT);
  pinMode(sitPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(standPin, HIGH);
  digitalWrite(sitPin, HIGH);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
}

void warningSounds() {
  analogWrite(buzzerPin, 5);
  delay(buzzerTimmer);
  analogWrite(buzzerPin, 0);
  delay(buzzerTimmer / 2);
  analogWrite(buzzerPin, 10);
  delay(buzzerTimmer);
  analogWrite(buzzerPin, 0);
}

int changeMode(String mode) {
  int pin = D0;
  warningSounds();
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
  digitalWrite(pin, LOW);
  delay(movementTimmer);

  digitalWrite(pin, HIGH);
  strip.setPixelColor(0, 0, 0, 0);
  strip.setPixelColor(1, 0, 0, 0);
  strip.show();
  return 0;
}

void setup() {
  Particle.function("changeMode", changeMode);
  Particle.variable("isStanding", isStanding);
  strip.begin();
  strip.show();
  changeMode("stand");
}

void loop() {

}
