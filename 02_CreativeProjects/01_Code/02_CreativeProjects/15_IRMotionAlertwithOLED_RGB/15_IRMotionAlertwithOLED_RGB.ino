/*
  IR Motion Alert with OLED + RGB LED
  -----------------------------------
  Inputs : IR sensor on D8 (PIR active-HIGH by default)
  Outputs: RGB LED on D3/D4/D5, SSD1306 OLED (I2C)

  Behavior:
    - On motion: blink RED, OLED "MOTION DETECTED", latch for ALERT_HOLD_MS
    - No motion: GREEN, OLED "No motion"
    - Works with both PIR (HIGH on motion) and IR obstacle (LOW on detect)

  Wiring:
    IR VCC->5V, GND->GND, OUT->D8
    RGB LED (common cathode) or traffic-light:
      D3->RED, D4->GREEN, D5->BLUE or YELLOW  (use series resistors if bare LEDs)
    OLED I2C: SDA->A4, SCL->A5, VCC->5V, GND->GND (addr 0x3C)

  Tuning:
    - Set IR_ACTIVE_HIGH=true for PIR; false for IR obstacle sensor
    - ALERT_HOLD_MS keeps the alert visible after last motion
    - BLINK_MS sets RED blink rate
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------- Pins --------
const int PIN_IR  = 8;   // IR sensor output
const int PIN_R   = 3;   // RED LED
const int PIN_G   = 4;   // GREEN LED
const int PIN_B   = 5;   // BLUE (or YELLOW module pin)

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- Behavior tuning --------
const bool IR_ACTIVE_HIGH = true;      // true=PIR (HIGH on motion), false=IR obstacle (LOW on detect)
const unsigned long ALERT_HOLD_MS = 5000; // hold alert for 5 s after last motion
const unsigned long READ_PERIOD_MS = 1000;
const unsigned long BLINK_MS = 350;    // RED blink period

// -------- LED helpers (common-cathode) --------
// If you use a common-ANODE module, swap HIGH <-> LOW here.
void ledOff()    { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledRed()    { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledGreen()  { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
void ledBlue()   { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, HIGH); }
void ledYellow() { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }

bool motionNow() {
  int v = digitalRead(PIN_IR);
  return IR_ACTIVE_HIGH ? (v == HIGH) : (v == LOW);
}

void showOLED(const char* line1, const char* line2) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("IR Motion Alert");
  display.setCursor(0,14);
  display.println(line1);
  if (line2 && line2[0]) {
    display.setCursor(0,26);
    display.println(line2);
  }
  display.display();
}

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  ledOff();

  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed (0x3C)");
  }
  showOLED("Waiting for sensor", IR_ACTIVE_HIGH ? "Mode: PIR (HIGH)" : "Mode: IR obstacle (LOW)");
  delay(800);
}

void loop() {
  static unsigned long alertUntil = 0;      // time until which alert stays active
  static unsigned long lastPrint  = 0;
  static unsigned long lastBlink  = 0;
  static bool blinkOn = true;

  // Update alert latch
  if (motionNow()) {
    alertUntil = millis() + ALERT_HOLD_MS;  // extend alert window
  }

  bool alertActive = (millis() < alertUntil);

  // LED behavior
  if (alertActive) {
    // blink RED
    if (millis() - lastBlink > BLINK_MS) {
      lastBlink = millis();
      blinkOn = !blinkOn;
    }
    if (blinkOn) ledRed(); else ledOff();
  } else {
    ledGreen(); // idle state
  }

  // OLED + Serial each second
  if (millis() - lastPrint > READ_PERIOD_MS) {
    lastPrint = millis();

    if (alertActive) {
      showOLED("MOTION DETECTED!", "Alert active");
      Serial.println("Status: MOTION DETECTED (alert holding)");
    } else {
      showOLED("No motion", IR_ACTIVE_HIGH ? "PIR idle" : "Obstacle idle");
      Serial.println("Status: No motion");
    }
  }
}
