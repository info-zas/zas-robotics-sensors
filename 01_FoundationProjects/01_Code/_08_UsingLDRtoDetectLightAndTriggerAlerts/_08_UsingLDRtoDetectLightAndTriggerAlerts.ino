const int ldrPin = A0; // Pin for LDR
const int ledPin = 9;  // Pin for LED
const int buzzerPin = 12; // Pin for Buzzer
int ldrValue = 0; // Variable to store LDR value

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600); // Start serial communication for debugging
}
void loop() {
  ldrValue = analogRead(ldrPin); // Read the value from the LDR
  Serial.println(ldrValue); // Print the LDR value to the Serial Monitor

  if (ldrValue > 500) { // Threshold for light level (adjust as needed)
    digitalWrite(ledPin, HIGH); // Turn on the LED
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(ledPin, LOW); // Turn off the LED
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
  }
  delay(100); // Short delay for stability
}
