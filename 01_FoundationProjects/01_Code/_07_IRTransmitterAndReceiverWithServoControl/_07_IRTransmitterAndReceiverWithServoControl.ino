#include <Servo.h>

Servo myServo; // Create a Servo object
const int irReceiverPin = 8; // Pin for IR receiver


void setup() {
  myServo.attach(3); // Attach the servo to pin 9
  pinMode(irReceiverPin, INPUT);
  
}

void loop() {
  

  // Check for IR signal
  if (digitalRead(irReceiverPin) == LOW) {
    myServo.write(90); // Move servo to 90 degrees
    delay(1000); // Wait for 1 second
    myServo.write(0); // Move servo back to 0 degrees
  }
}
