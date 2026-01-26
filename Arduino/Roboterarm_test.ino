#include <Servo.h>

// Wir erstellen 4 Servo-Objekte
Servo servoBase;      // Basis-Drehung (Ganz unten)
Servo servoShoulder;  // Schulter (Der unterste Hebe-Arm)
Servo servoElbow;     // Ellenbogen (Der mittlere Hebe-Arm)
Servo servoWrist;     // Handgelenk (Der oberste Arm vor dem Magneten)

// Konfiguration der Pins
const int pinBase = 3;
const int pinShoulder = 5;
const int pinElbow = 6;
const int pinWrist = 9;

void setup() {
  Serial.begin(9600);
  Serial.println("--- 4-Achsen Roboter-Test gestartet ---");

  // WICHTIG: Erst die Position setzen, DANN verbinden (attach).
  // So verhindern wir das wilde Zucken beim Start.
  // Wir setzen alle auf 90 Grad (Mitte/Aufrecht).
  
  servoBase.write(90);
  servoShoulder.write(90);
  servoElbow.write(90);
  servoWrist.write(90);

  // Jetzt verbinden wir die Software mit den Pins
  servoBase.attach(pinBase);
  servoShoulder.attach(pinShoulder);
  servoElbow.attach(pinElbow);
  servoWrist.attach(pinWrist);

  Serial.println("Roboter steht in Grundstellung (90 Grad).");
  delay(2000); // Zeit zum Hineinstellen der Batterie und Finger wegnehmen
}

void loop() {
  // 1. Basis testen (Pin 3)
  Serial.println("Teste Basis (Pin 3)...");
  moveSlowly(servoBase, 90, 130); // Nach links/rechts
  moveSlowly(servoBase, 130, 90); // Zurück zur Mitte
  delay(1000);

  // 2. Schulter testen (Pin 5)
  Serial.println("Teste Schulter (Pin 5)...");
  moveSlowly(servoShoulder, 90, 110); // Leicht anheben/senken
  moveSlowly(servoShoulder, 110, 90); // Zurück
  delay(1000);

  // 3. Ellenbogen testen (Pin 6)
  Serial.println("Teste Ellenbogen (Pin 6)...");
  moveSlowly(servoElbow, 90, 60);     // Anderer Winkel zum Testen
  moveSlowly(servoElbow, 60, 90);     // Zurück
  delay(1000);

  // 4. Handgelenk testen (Pin 9)
  Serial.println("Teste Handgelenk (Pin 9)...");
  moveSlowly(servoWrist, 90, 120);    // Winken
  moveSlowly(servoWrist, 120, 90);    // Zurück
  delay(1000);

  Serial.println("Durchlauf beendet. Pause...");
  delay(3000);
}

// Hilfsfunktion für langsame Bewegungen
// Damit der Arm nicht ruckartig springt und umfällt
void moveSlowly(Servo &myServo, int startAngle, int endAngle) {
  int step = 1;
  int speedDelay = 15; // Je höher, desto langsamer (in Millisekunden)

  if (startAngle < endAngle) {
    for (int pos = startAngle; pos <= endAngle; pos += step) {
      myServo.write(pos);
      delay(speedDelay);
    }
  } else {
    for (int pos = startAngle; pos >= endAngle; pos -= step) {
      myServo.write(pos);
      delay(speedDelay);
    }
  }
}
