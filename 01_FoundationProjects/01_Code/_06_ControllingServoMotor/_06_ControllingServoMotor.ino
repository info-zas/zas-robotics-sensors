#include <Servo.h>

Servo myServo; // Create a Servo object

void setup() {
  myServo.attach(3); // Attach the servo to pin 9
}

void loop() {
  // Move the servo to 0 degrees
  myServo.write(0);
  delay(1000); // Wait for 1 second

  // Move the servo to 90 degrees
  myServo.write(90);
  delay(1000); // Wait for 1 second

  // Move the servo to 180 degrees
  myServo.write(180);
  delay(1000); // Wait for 1 second
}
