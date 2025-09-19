/*
 * Project: Distance-Based Fan Speed Controller (Servo Motor Demo)
 * Components: Arduino Uno, HC-SR04, SG90 Servo Motor
 * Function: Simulates fan speed by rotating servo angle based on distance
 * Author: [Your Name]
 */

#include <Servo.h>

const int trigPin = A1;
const int echoPin = A2;
const int servoPin = 3;

Servo fanServo;
long duration;
int distance;
int servoAngle;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  fanServo.attach(servoPin);
  fanServo.write(0);
  Serial.println("System Ready...");
}

void loop() {
  // Trigger ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Map distance to servo angle (0–180 degrees)
  if (distance > 10 && distance <= 100) {
    servoAngle = map(distance, 10, 100, 180, 30);
  } else if (distance <= 10) {
    servoAngle = 180; // max angle (fastest demo speed)
  } else {
    servoAngle = 0;   // off
  }

  fanServo.write(servoAngle);

  Serial.print("Servo Angle: ");
  Serial.println(servoAngle);

  delay(200);
}
