// Tilt -> LED + Buzzer
// Tilt switch on D8 (to GND), LED on D9, Buzzer on D12

const int TILT_PIN   = 8;   // Tilt sensor (active LOW with INPUT_PULLUP)
const int LED_PIN    = 9;   // On-board red LED
const int BUZZER_PIN = 12;  // On-board buzzer (active type)

void setup() {
  pinMode(TILT_PIN, INPUT_PULLUP);  // No external resistor needed
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // Reads LOW when tilted/triggered
  bool tilted = (digitalRead(TILT_PIN) == LOW);

  if (tilted) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);   // For passive buzzer, use: tone(BUZZER_PIN, 1000);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);    // For passive buzzer with tone(): noTone(BUZZER_PIN);
  }

  delay(20); // small debounce
}
