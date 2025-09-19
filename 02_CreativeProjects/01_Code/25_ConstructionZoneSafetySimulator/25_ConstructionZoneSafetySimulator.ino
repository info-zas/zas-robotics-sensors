/*
  Construction-Zone Safety Simulator
  Inputs : IR sensor (D8), MQ-135 (A0)
  Outputs: Traffic light (D3=R, D4=Y, D5=G), OLED (I2C)

  Priority:
    1) If air is UNHEALTHY/HAZARDOUS -> RED (override)
    2) Else if motion detected       -> YELLOW (caution)
    3) Else                           -> GREEN (clear)

  Notes:
    - MQ-135 uses raw ADC as a relative indicator (not calibrated ppm).
    - Includes warm-up and moving-average smoothing.
    - OLED init is resilient (tries 0x3C then 0x3D; retries in background).
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Pins ----------
const int PIN_IR   = 8;   // IR/PIR digital output
const int PIN_MQ   = A0;  // MQ-135 analog output
const int PIN_RED  = 3;
const int PIN_YEL  = 4;
const int PIN_GRN  = 5;

// ---------- Config ----------
const bool IR_ACTIVE_HIGH = false; // Obstacle sensor: LOW=detect -> false. PIR: HIGH=detect -> true.
const byte AVG_SAMPLES    = 16;    // smoothing window
const unsigned long UI_MS = 1000;  // OLED/Serial refresh period
const unsigned long WARMUP_S = 30; // MQ-135 warm-up seconds

// Raw ADC thresholds (tune for your unit/room)
const int RAW_GOOD_MAX      = 300;   // <=300   -> Good
const int RAW_MODERATE_MAX  = 600;   // 301-600 -> Moderate
const int RAW_UNHEALTHY_MAX = 800;   // 601-800 -> Unhealthy
// >800 -> Hazardous

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool oledOK = false;
unsigned long lastOLEDtry = 0;
const unsigned long OLED_RETRY_MS = 2000;

// ---------- Smoothing ----------
int   mqBuf[AVG_SAMPLES];
byte  mqIdx = 0;
long  mqSum = 0;

// ---------- Timing ----------
unsigned long lastUI = 0;

// ---------- LED helpers (active-HIGH) ----------
void ledsOff()     { digitalWrite(PIN_RED, LOW); digitalWrite(PIN_YEL, LOW); digitalWrite(PIN_GRN, LOW); }
void ledRed()      { digitalWrite(PIN_RED, HIGH); digitalWrite(PIN_YEL, LOW);  digitalWrite(PIN_GRN, LOW); }
void ledYellow()   { digitalWrite(PIN_RED, LOW);  digitalWrite(PIN_YEL, HIGH); digitalWrite(PIN_GRN, LOW); }
void ledGreen()    { digitalWrite(PIN_RED, LOW);  digitalWrite(PIN_YEL, LOW);  digitalWrite(PIN_GRN, HIGH); }
// For common-anode modules: swap HIGH<->LOW above.

// ---------- Helpers ----------
bool motionNow() {
  int v = digitalRead(PIN_IR);
  return IR_ACTIVE_HIGH ? (v == HIGH) : (v == LOW);
}

int readMQSmooth() {
  int v = analogRead(PIN_MQ);
  mqSum -= mqBuf[mqIdx];
  mqBuf[mqIdx] = v;
  mqSum += v;
  mqIdx = (mqIdx + 1) % AVG_SAMPLES;
  return (int)(mqSum / AVG_SAMPLES);
}

const char* categoryForRaw(int x) {
  if (x <= RAW_GOOD_MAX)      return "GOOD";
  if (x <= RAW_MODERATE_MAX)  return "MODERATE";
  if (x <= RAW_UNHEALTHY_MAX) return "UNHEALTHY";
  return "HAZARDOUS";
}

const char* adviceForRaw(int x) {
  if (x <= RAW_GOOD_MAX)      return "Air OK";
  if (x <= RAW_MODERATE_MAX)  return "Ventilate";
  if (x <= RAW_UNHEALTHY_MAX) return "Limit exposure";
  return "STOP! Avoid dust";
}

bool oledBeginTry(uint8_t addr) {
  if (display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    display.setRotation(0);        // use 2 if your text is upside-down
    display.dim(false);
    oledOK = true;
    return true;
  }
  return false;
}

void tryInitOLED() {
  if (lastOLEDtry != 0 && (millis() - lastOLEDtry) < OLED_RETRY_MS) return;
  lastOLEDtry = millis();
  oledOK = oledBeginTry(0x3C) || oledBeginTry(0x3D);
}

void showOLED(int raw, const char* cat, const char* hint, bool warming, unsigned long left, const char* light) {
  if (!oledOK) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0,0);   display.println("Construction Zone");
  display.setCursor(0,14);  display.print("Air: "); display.print(raw);
  display.print("  ");       display.println(cat);

  display.setCursor(0,26);  display.print("Light: "); display.println(light);
  display.setCursor(0,38);  display.println(hint);

  if (warming) {
    display.setCursor(0,50);
    display.print("Warming: "); display.print(left); display.print("s");
  }
  display.display();
}

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YEL, OUTPUT);
  pinMode(PIN_GRN, OUTPUT);
  ledsOff();

  Serial.begin(9600);
  delay(200);
  Serial.println(F("Construction-Zone Safety Simulator"));

  // Prefill smoothing buffer
  for (byte i=0; i<AVG_SAMPLES; i++) {
    int v = analogRead(PIN_MQ);
    mqBuf[i] = v;
    mqSum += v;
    delay(2);
  }

  Wire.begin();
  Wire.setClock(400000);
  tryInitOLED();  // first attempt
}

void loop() {
  // Retry OLED in background if needed
  if (!oledOK) tryInitOLED();

  // Read sensors
  int  raw     = readMQSmooth();
  bool motion  = motionNow();

  // Warm-up handling
  unsigned long secs = millis()/1000UL;
  bool warming = (secs < WARMUP_S);
  unsigned long left = (WARMUP_S > secs) ? (WARMUP_S - secs) : 0;

  // Determine category and advice
  const char* cat  = categoryForRaw(raw);
  const char* hint = adviceForRaw(raw);

  // Decide traffic light
  const char* light;
  if (!warming && (raw > RAW_UNHEALTHY_MAX || raw > RAW_MODERATE_MAX && !motion)) {
    // If hazardous, or generally high even without motion -> RED
    ledRed();  light = "RED";
  } else if (!warming && raw > RAW_GOOD_MAX && motion) {
    // Moderate dust + motion -> YELLOW
    ledYellow(); light = "YELLOW";
  } else if (warming) {
    // During warm-up: caution
    ledYellow(); light = "YELLOW (warm-up)";
  } else if (motion) {
    // Motion but air good -> caution
    ledYellow(); light = "YELLOW";
  } else {
    // Clear
    ledGreen(); light = "GREEN";
  }

  // UI update (OLED + Serial) once per second
  if (millis() - lastUI > UI_MS) {
    lastUI = millis();
    showOLED(raw, cat, hint, warming, left, light);

    Serial.print("raw="); Serial.print(raw);
    Serial.print("  cat="); Serial.print(cat);
    Serial.print("  motion="); Serial.print(motion ? "1" : "0");
    Serial.print("  light="); Serial.println(light);
  }

  delay(20);
}
