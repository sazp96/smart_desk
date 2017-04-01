// --------------------------------------------
//              Smart Desk V1.0
//         Alexa controlled smart desk
// Written but not maintained by: Sergio Zamora
// --------------------------------------------

SYSTEM_MODE(AUTOMATIC);
STARTUP( pinIni() );

int standPin = D1;
int sitPin = D2;
int buzzerPin = A5;
int ledPin = A4;
#define movementTimmer 12000  //milliseconds

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

int changeMode(String mode) {
  int pin = D0;
  if (mode.equals("sit"))
    pin = sitPin;
  else
    pin = standPin;

  digitalWrite(pin, LOW);
  delay(movementTimmer);
  digitalWrite(pin, HIGH);
  return 0;
}

void setup() {
  Particle.function("changeMode", changeMode);
}

void loop() {

}
