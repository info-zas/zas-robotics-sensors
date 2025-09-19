#define TOUCH_PIN 4
#define LED_PIN 9
#define BUZZER_PIN 12

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  int touchState = digitalRead(TOUCH_PIN);
  
  if (touchState == HIGH) { // If the touch sensor is activated
    digitalWrite(LED_PIN, HIGH); // Turn on the LED
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    delay(1000); // Keep them on for 1 second
    digitalWrite(LED_PIN, LOW); // Turn off the LED
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
  }
}
