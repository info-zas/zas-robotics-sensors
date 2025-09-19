#define BUZZER_PIN 12

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
  delay(1000);                    // Wait for 1 second
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer
  delay(1000);                    // Wait for 1 second
}
