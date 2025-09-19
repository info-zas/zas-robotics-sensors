// Touch-Free Liquid Dispenser
// Components: Ultrasonic Sensor (HC-SR04), Servo Motor, optional Touch Sensor

#include <Servo.h>

#define TRIG_PIN A1
#define ECHO_PIN A2
#define SERVO_PIN 3
#define TOUCH_PIN 4  // Optional

Servo dispenserServo;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT);
  dispenserServo.attach(SERVO_PIN);
  dispenserServo.write(0); // Initial position
}

void loop() {
  // Measure distance
  long duration, distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm

  // Read touch sensor (if connected)
  bool touchDetected = digitalRead(TOUCH_PIN);

  // If hand detected within 10 cm OR touch sensor pressed
  if ((distance > 0 && distance < 10) || touchDetected == HIGH) {
    Serial.println("Hand Detected! Dispensing...");
    dispenserServo.write(90); // Move servo to press pump
    delay(1000);              // Hold press
    dispenserServo.write(0);  // Reset position
    delay(2000);              // Wait before next use
  }
  else {
    dispenserServo.write(0); // Stay idle
  }

  delay(200);
}
