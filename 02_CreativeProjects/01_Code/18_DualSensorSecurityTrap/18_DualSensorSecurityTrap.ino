/*
  Dual-Sensor Security Trap  —  Simplified
  Inputs : LDR (A0) for beam break, IR sensor (D8) for motion
  Output : SSD1306 OLED (I2C)

  Keeps:
   - 2 s LDR auto-calibration
   - Light EMA smoothing
   - IR motion + LDR beam-break logic
   - Latched alert hold
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Pins ----------
const int PIN_LDR = A0;   // LDR divider junction (LDR->5V, 10k->GND, junction->A0)
const int PIN_IR  = 8;    // IR sensor OUT

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Tuning ----------
const bool IR_ACTIVE_HIGH        = true;     // PIR:true (HIGH on motion). IR obstacle:false (LOW on detect)
const unsigned long CAL_MS       = 2000;     // LDR calibration time
const unsigned long HOLD_MS      = 4000;     // alert hold time
const unsigned long REFRESH_MS   = 1000;     // OLED/Serial refresh
const int  TRIP_DELTA            = 120;      // |LDR-Base| above this => beam broken
const float EMA_ALPHA            = 0.20f;    // smoothing factor (0..1)

// ---------- State ----------
bool calibrated = false;
int  ldrBase = 0;               // calibrated baseline
float ldrEMA = 0;               // filtered LDR
unsigned long tCalStart = 0;
unsigned long tHoldUntil = 0;
unsigned long tLastPrint = 0;

enum Event { EVT_NONE, EVT_BEAM, EVT_MOTION };
Event lastEvent = EVT_NONE;

// ---------- Small helpers ----------
inline bool motionNow() {
  int v = digitalRead(PIN_IR);
  return IR_ACTIVE_HIGH ? (v == HIGH) : (v == LOW);
}

void showOLED(const char* title, int ldr, int base, int ir, const char* status) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);  display.println(title);
  display.setCursor(0,14); display.print("LDR: "); display.print(ldr);
                           display.print("  Base: "); display.println(base);
  display.print("IR : ");  display.println(ir);
  display.setCursor(0,36); display.println(status);
  display.display();
}

void setup() {
  pinMode(PIN_IR, INPUT);

  Serial.begin(9600);
  Wire.begin(); Wire.setClock(400000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed (0x3C)");
    while (1) { delay(1000); }
  }

  // Start calibration
  int first = analogRead(PIN_LDR);
  ldrEMA = first;                 // seed EMA
  tCalStart = millis();
  showOLED("Security Trap (Calibrating)", (int)ldrEMA, 0, digitalRead(PIN_IR), "Align beam & wait...");
}

void loop() {
  // --- Read + smooth LDR ---
  int ldrRaw = analogRead(PIN_LDR);
  ldrEMA = (1.0f - EMA_ALPHA) * ldrEMA + EMA_ALPHA * ldrRaw;
  int ldr = (int)ldrEMA;

  // --- Calibration window ---
  if (!calibrated) {
    if (millis() - tCalStart >= CAL_MS) {
      ldrBase = ldr;              // set baseline from EMA
      calibrated = true;
      Serial.print("Calibration done. LDR base = "); Serial.println(ldrBase);
    } else if (millis() - tLastPrint > 300) {
      tLastPrint = millis();
      showOLED("Security Trap (Calib...)", ldr, 0, digitalRead(PIN_IR), "Hold beam steady");
    }
    return;
  }

  // --- Detection ---
  bool motion = motionNow();
  bool beamBroken = (abs(ldr - ldrBase) > TRIP_DELTA);

  Event evt = EVT_NONE;
  if (beamBroken) evt = EVT_BEAM; else if (motion) evt = EVT_MOTION;
  if (evt != EVT_NONE) { lastEvent = evt; tHoldUntil = millis() + HOLD_MS; }

  // --- Status text (with latch) ---
  const char* status;
  if (millis() < tHoldUntil && lastEvent != EVT_NONE) {
    status = (lastEvent == EVT_BEAM) ? "ALERT: BEAM BROKEN!" : "ALERT: MOTION!";
  } else if (beamBroken) {
    status = "Beam unstable - check alignment";
  } else {
    status = "ARMED / Idle";
  }

  // --- Periodic UI/Serial ---
  if (millis() - tLastPrint > REFRESH_MS) {
    tLastPrint = millis();
    int irRaw = digitalRead(PIN_IR);

    showOLED("Security Trap", ldr, ldrBase, irRaw, status);

    Serial.print("LDR="); Serial.print(ldr);
    Serial.print("  Base="); Serial.print(ldrBase);
    Serial.print("  IRraw="); Serial.print(irRaw);
    Serial.print("  Motion="); Serial.print(motion ? 1 : 0);
    Serial.print("  BeamBroken="); Serial.print(beamBroken ? 1 : 0);
    Serial.print("  Status="); Serial.println(status);
  }
}
