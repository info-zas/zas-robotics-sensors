//Traffic Light Sequence Code
// Pin definitions
const int redPin = 9;    // Pin connected to the red LED
const int yellowPin = 7; // Pin connected to the yellow LED
const int greenPin = 10; // Pin connected to the green LED

void setup() {
  // Set the LED pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  // Red light on
  digitalWrite(redPin, HIGH);   // Turn on red light
  digitalWrite(yellowPin, LOW);  // Turn off yellow light
  digitalWrite(greenPin, LOW);   // Turn off green light
  delay(5000);                   // Keep red light on for 5 seconds

  // Green light on
  digitalWrite(redPin, LOW);     // Turn off red light
  digitalWrite(yellowPin, LOW);  // Turn off yellow light
  digitalWrite(greenPin, HIGH);  // Turn on green light
  delay(5000);                   // Keep green light on for 5 seconds

  // Yellow light on
  digitalWrite(redPin, LOW);     // Turn off red light
  digitalWrite(yellowPin, HIGH); // Turn on yellow light
  digitalWrite(greenPin, LOW);   // Turn off green light
  delay(2000);                   // Keep yellow light on for 2 seconds
}
