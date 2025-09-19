/*
  IR Occupancy Indicator
  Inputs : IR Sensor on D8 (PIR or IR obstacle)
  Outputs: RGB LED (D3/D4/D5), SSD1306 OLED (I2C)

  Logic:
    - IR present -> OCCUPIED (LED RED, OLED "Occupied")
    - No presence for HOLD_MS -> AVAILABLE (LED GREEN, OLED "Available")
    - Optional PIR warm-up banner for first WARMUP_S seconds
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------- Pins --------
const int PIN_IR = 8;   // IR sensor OUT
const int PIN_R  = 3;   // RED
const int PIN_G  = 4;   // GREEN
const int PIN_B  = 5;   // BLUE or YELLOW pin (not used here)

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- Behavior tuning --------
const bool IR_ACTIVE_HIGH   = true;     // PIR:true, Obstacle sensor (often LOW on detect): false
const unsigned long HOLD_MS = 3000;     // keep OCCUPIED this long after last detection
const unsigned long READ_MS = 1000;     // OLED/Serial refresh
const unsigned long WARMUP_S = 0;       // PIR warm-up seconds (set 30..60 for PIR; keep 0 for obstacle sensor)

// -------- LED helpers (common-cathode). For common-anode, swap HIGH<->LOW. --------
void ledOff()   { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledRed()   { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledGreen() { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
void ledYellow(){ digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }

inline bool presenceNow() {
  int v = digitalRead(PIN_IR);
  return IR_ACTIVE_HIGH ? (v == HIGH) : (v == LOW);
}

void showOLED(const char* statusLine) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);  display.println("Parking / Space Monitor");
  display.setCursor(0,14); display.print("IR: ");
  display.println(IR_ACTIVE_HIGH ? "active-HIGH" : "active-LOW");
  display.setCursor(0,32); display.println(statusLine);
  display.display();
}

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  ledOff();

  Serial.begin(9600);
  Wire.begin(); Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
    while (1) { delay(1000); }
  }

  // Optional warm-up status (useful for PIR modules)
  if (WARMUP_S > 0) {
    unsigned long t0 = millis();
    while ((millis() - t0) < (WARMUP_S * 1000UL)) {
      ledYellow();
      showOLED("Warming up sensor...");
      delay(250);
      ledOff();
      delay(250);
    }
  }
}

void loop() {
  static unsigned long lastSeen = 0;     // last time presence was detected
  static unsigned long lastPrint = 0;

  // --- Presence sensing ---
  if (presenceNow()) {
    lastSeen = millis();                 // refresh hold timer
  }

  bool occupied = (millis() - lastSeen) < HOLD_MS;

  // --- Outputs ---
  if (occupied) {
    ledRed();
    if (millis() - lastPrint > READ_MS) {
      lastPrint = millis();
      showOLED("Status: AVAILABLE");
      Serial.println("Status: AVAILABLE");
    }
  } else {
    ledGreen();
    if (millis() - lastPrint > READ_MS) {
      lastPrint = millis();
      showOLED("Status: OCCUPIED");
      Serial.println("Status: OCCUPIED");
    }
  }
}
