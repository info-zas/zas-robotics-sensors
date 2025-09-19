/*
  Smart Street Light
  ------------------
  Sensors:
    - LDR module AO -> A0 (VCC->5V, GND->GND)
    - IR obstacle/motion sensor OUT -> D8 (VCC->5V, GND->GND)
  Output:
    - RGB LED (common CATHODE):
        R -> D3 (220Ω), G -> D4 (220Ω), B -> D5 (220Ω), cathode -> GND

  Logic:
    - Day (bright): LED OFF
    - Night (dark) + no motion: BLUE steady
    - Night (dark) + motion: WHITE blink (R+G+B)

  Notes:
    - Many IR modules are ACTIVE-LOW (LOW = detected). Flip IR_ACTIVE_LOW if yours is different.
    - D4 is not PWM; we use simple ON/OFF colors (no dimming).
*/

//// -------- Settings you can tune --------
const int LIGHT_THRESHOLD = 500;       // < threshold = night; >= threshold = day
const bool IR_ACTIVE_LOW  = true;      // true if IR outputs LOW when motion detected
const int BLINK_ON_MS     = 300;       // how long "streetlight ON" stays on
const int BLINK_OFF_MS    = 200;       // off time during blink
const unsigned long PRINT_EVERY_MS = 400; // Serial debug rate
//// --------------------------------------

//// -------- Pins --------
const int PIN_LDR = A0;
const int PIN_IR  = 8;  // IR sensor OUT
const int PIN_R   = 3;  // RGB LED pins
const int PIN_G   = 4;
const int PIN_B   = 5;

//// -------- Helper functions (COMMON CATHODE LED) --------
void ledOff()        { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledBlue()       { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, HIGH); }
void ledWhite()      { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, HIGH); }
// If you have a COMMON ANODE LED, swap HIGH<->LOW in the three functions above.

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  ledOff();

  Serial.begin(9600);
  Serial.println("Smart Street Light starting...");
}

void loop() {
  // 1) Read sensors
  int lightRaw = analogRead(PIN_LDR);      // 0..1023
  int irRaw    = digitalRead(PIN_IR);      // HIGH/LOW
  bool motion  = IR_ACTIVE_LOW ? (irRaw == LOW) : (irRaw == HIGH);
  bool isNight = (lightRaw < LIGHT_THRESHOLD);

  // 2) Decide & act
  if (!isNight) {
    // Daytime -> street light OFF
    ledOff();
  } else {
    // Nighttime
    if (motion) {
      // Motion detected -> blink white a couple times
      for (int i = 0; i < 2; i++) {
        ledWhite();
        delay(BLINK_ON_MS);
        ledOff();
        delay(BLINK_OFF_MS);
      }
      // After blink, keep BLUE standby
      ledBlue();
    } else {
      // No motion -> BLUE standby
      ledBlue();
    }
  }

  // 3) Print some debug info for tuning
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= PRINT_EVERY_MS) {
    lastPrint = millis();
    Serial.print("LDR=");
    Serial.print(lightRaw);
    Serial.print("  isNight=");
    Serial.print(isNight ? "YES" : "NO");
    Serial.print("  motion=");
    Serial.println(motion ? "YES" : "NO");
  }

  delay(20); // small pause
}
