/*
  Motion-Activated Alarm (IR Sensor + RGB LED)
  --------------------------------------------
  Hardware:
    - IR Sensor OUT -> D8
    - RGB LED (common cathode):
        R -> D3 (with 220Ω resistor)
        G -> D4 (with 220Ω resistor)
        B -> D5 (with 220Ω resistor)
        Common cathode -> GND

  Behavior:
    - If motion detected -> Blink RED alarm
    - If no motion -> Show GREEN safe light

  Notes:
    - Some IR modules output LOW when motion/obstacle is detected.
    - Adjust code if your sensor outputs inverted logic.
*/

//// ----- Pin Setup -----
const int PIN_IR = 8;  // IR sensor signal pin
const int PIN_R  = 3;  // Red channel of RGB LED
const int PIN_G  = 4;  // Green channel of RGB LED
const int PIN_B  = 5;  // Blue channel of RGB LED

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  Serial.begin(9600);
  Serial.println("Motion-Activated Alarm started...");
}

void loop() {
  // 1) Read IR sensor
  int motion = digitalRead(PIN_IR);

  // 2) Decide what to do
  if (motion == LOW) {  
    // Motion detected (for most modules LOW = detected)
    Serial.println("Motion detected! Alarm ON.");
    blinkRed();
  } else {
    // No motion
    Serial.println("No motion. Safe mode.");
    showGreen();
  }
}

// --- Helper Functions ---

// Blink red (alarm)
void blinkRed() {
  digitalWrite(PIN_R, HIGH);   // Red ON
  digitalWrite(PIN_G, LOW);
  digitalWrite(PIN_B, LOW);
  delay(300);

  digitalWrite(PIN_R, LOW);    // Red OFF
  delay(300);
}

// Show green (safe)
void showGreen() {
  digitalWrite(PIN_R, LOW);
  digitalWrite(PIN_G, HIGH);   // Green ON
  digitalWrite(PIN_B, LOW);
}
