#include <Servo.h>

#define tiltPin 4
#define servoPin 3

Servo myServo;

void setup() {
  pinMode(tiltPin, INPUT);
  myServo.attach(servoPin);
  Serial.begin(9600);
}

void loop() {
  int t = digitalRead(tiltPin);
  Serial.println(t);

  if (t == HIGH) {
    myServo.write(90);  // Open
  } else {
    myServo.write(0);   // Close
  }

  delay(100);
}
