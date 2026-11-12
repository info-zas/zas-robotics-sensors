#include <Servo.h>

Servo myServo;

void setup() {
  myServo.attach(3);
}

void loop() {
  // Move from 0 to 180 degrees
  for (int angle = 0; angle <= 180; angle += 10) {
    myServo.write(angle);
    delay(200);
  }

  // Move back from 180 to 0 degrees
  for (int angle = 180; angle >= 0; angle -= 10) {
    myServo.write(angle);
    delay(200);
  }
}
