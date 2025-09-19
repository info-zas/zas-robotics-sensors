const int tiltPin = 8; // Pin for tilt sensor
const int ledPin = 9;  // Pin for LED
const int buzzerPin = 12; // Pin for Buzzer

void setup() {
  pinMode(tiltPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  int tiltState = digitalRead(tiltPin); // Read the state of the tilt sensor
  
  if (tiltState == LOW) { // If the tilt sensor is activated
    digitalWrite(ledPin, HIGH); // Turn on the LED
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(ledPin, LOW); // Turn off the LED
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
  }
  delay(100); // Short delay for stability
}
