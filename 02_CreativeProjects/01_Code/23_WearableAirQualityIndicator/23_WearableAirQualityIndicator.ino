/*
  Wearable AQ Indicator: MQ-135 -> NeoPixel Ring + OLED
  - Smooths analog readings
  - Warm-up countdown for MQ-135
  - Color + progress arc on NeoPixel ring
  - Robust OLED init (tries 0x3C, 0x3D; retries, never freezes)
*/

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- Pins ----------------
#define PIN_MQ        A0
#define PIN_NEOPIXEL  6
#define NUMPIXELS     12   // set to your ring size (8/12/16/etc.)

// ---------------- NeoPixel ----------------
#define NP_BRIGHTNESS 110
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool oledOK = false;
uint8_t oledAddr = 0x3C;
unsigned long lastOLEDtry = 0;
const unsigned long OLED_RETRY_MS = 2000;

// ---------------- Timing / Smoothing ----------------
const byte  AVG_SAMPLES      = 12;      // moving average
const unsigned long UI_PERIOD_MS = 1000;
const unsigned long WARMUP_SECONDS = 30;

int   mqBuf[AVG_SAMPLES];
byte  mqIdx = 0;
long  mqSum = 0;

unsigned long tLastUI = 0;

// ---------------- Thresholds (tune for your unit) ----------------
// Raw ADC 0..1023 (these are sensible starting points)
const int RAW_GOOD_MAX       = 300;   // <= 300 -> Good
const int RAW_MODERATE_MAX   = 600;   // 301..600 -> Moderate
const int RAW_UNHEALTHY_MAX  = 800;   // 601..800 -> Unhealthy
// > RAW_UNHEALTHY_MAX -> Hazardous

// ---------------- Helpers ----------------
uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
  return pixels.Color(r, g, b);
}

// Linear interpolate 8-bit
uint8_t lerp8(uint8_t a, uint8_t b, float t) {
  float v = a + (b - a) * t;
  if (v < 0) v = 0; if (v > 255) v = 255;
  return (uint8_t)(v + 0.5f);
}

// Gradient color across ranges: green -> yellow -> orange -> red -> purple
uint32_t colorForRaw(int x) {
  if (x <= RAW_GOOD_MAX) {
    // green (0,255,0) -> yellow (255,255,0)
    float t = (float)x / RAW_GOOD_MAX;
    return color(lerp8(0,255,t), 255, 0);
  } else if (x <= RAW_MODERATE_MAX) {
    // yellow (255,255,0) -> orange (255,128,0)
    float t = (float)(x - RAW_GOOD_MAX) / (RAW_MODERATE_MAX - RAW_GOOD_MAX);
    return color(255, lerp8(255,128,t), 0);
  } else if (x <= RAW_UNHEALTHY_MAX) {
    // orange (255,128,0) -> red (255,0,0)
    float t = (float)(x - RAW_MODERATE_MAX) / (RAW_UNHEALTHY_MAX - RAW_MODERATE_MAX);
    return color(255, lerp8(128,0,t), 0);
  } else {
    // red (255,0,0) -> purple (255,0,128) as it gets very high
    float t = (float)(x - RAW_UNHEALTHY_MAX) / (1023 - RAW_UNHEALTHY_MAX);
    if (t < 0) t = 0; if (t > 1) t = 1;
    return color(255, 0, lerp8(0,128,t));
  }
}

const char* categoryForRaw(int x) {
  if (x <= RAW_GOOD_MAX) return "GOOD";
  if (x <= RAW_MODERATE_MAX) return "MODERATE";
  if (x <= RAW_UNHEALTHY_MAX) return "UNHEALTHY";
  return "HAZARDOUS";
}

const char* adviceForRaw(int x) {
  if (x <= RAW_GOOD_MAX) return "Air OK";
  if (x <= RAW_MODERATE_MAX) return "Ventilate";
  if (x <= RAW_UNHEALTHY_MAX) return "Mask/Move away";
  return "Avoid exposure!";
}

// Moving-average read
int readMQSmooth() {
  int v = analogRead(PIN_MQ);
  mqSum -= mqBuf[mqIdx];
  mqBuf[mqIdx] = v;
  mqSum += v;
  mqIdx = (mqIdx + 1) % AVG_SAMPLES;
  return (int)(mqSum / AVG_SAMPLES);
}

void oledSplash(const char* line2) {
  if (!oledOK) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);  display.println("Wearable AQ Indicator");
  display.setCursor(0,14); display.println(line2);
  display.display();
}

bool oledBeginAt(uint8_t addr) {
  if (display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    oledAddr = addr;
    display.setRotation(0);
    display.dim(false);
    oledSplash("OLED OK");
    return true;
  }
  return false;
}

void tryInitOLED() {
  if (lastOLEDtry != 0 && (millis() - lastOLEDtry) < OLED_RETRY_MS) return;
  lastOLEDtry = millis();
  oledOK = oledBeginAt(0x3C) || oledBeginAt(0x3D);
}

void ringProgress(uint32_t col, uint8_t count) {
  if (count > NUMPIXELS) count = NUMPIXELS;
  for (uint8_t i = 0; i < NUMPIXELS; i++) {
    if (i < count) pixels.setPixelColor(i, col);
    else           pixels.setPixelColor(i, 0);   // off
  }
  pixels.show();
}

void showOLEDValue(int raw, const char* cat, const char* hint, bool warming, unsigned long secsLeft) {
  if (!oledOK) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);  display.println("Wearable AQ Indicator");

  display.setCursor(0,14);
  display.print("Raw: "); display.println(raw);

  display.setCursor(0,26);
  display.print("Level: "); display.println(cat);

  display.setCursor(0,38);
  display.println(hint);

  if (warming) {
    display.setCursor(0,50);
    display.print("Warming: "); display.print(secsLeft); display.print("s");
  }

  display.display();
}

void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.setBrightness(NP_BRIGHTNESS);
  pixels.clear();
  pixels.show();

  // Prefill smoothing buffer
  for (byte i=0; i<AVG_SAMPLES; i++) {
    int v = analogRead(PIN_MQ);
    mqBuf[i] = v;
    mqSum += v;
    delay(2);
  }

  Wire.begin();
  Wire.setClock(400000);
  tryInitOLED();   // immediate attempt
  oledSplash("Cover sensor to test");
}

void loop() {
  // Keep retrying OLED init in background if needed
  if (!oledOK) tryInitOLED();

  // Read & smooth sensor
  int raw = readMQSmooth();

  // Warm-up status
  unsigned long secs = millis()/1000UL;
  bool warming = (secs < WARMUP_SECONDS);
  unsigned long left = (WARMUP_SECONDS > secs) ? (WARMUP_SECONDS - secs) : 0;

  // Category + color + message
  const char* cat  = categoryForRaw(raw);
  const char* hint = adviceForRaw(raw);
  uint32_t col     = colorForRaw(raw);

  // Progress count (more LEDs as it gets worse)
  uint8_t count = map(raw, 0, 1023, 1, NUMPIXELS);
  if (warming) {
    // During warm-up: gentle pulsing orange to indicate “warming”
    float s = sin(2.0f * PI * (millis() % 2000) / 2000.0f) * 0.5f + 0.5f;
    uint8_t g = (uint8_t)(80 + 60 * s);
    ringProgress(pixels.Color(255, g, 0), count);
  } else {
    ringProgress(col, count);
  }

  // OLED + Serial once per second
  if (millis() - tLastUI > UI_PERIOD_MS) {
    tLastUI = millis();
    showOLEDValue(raw, cat, hint, warming, left);

    Serial.print("MQ135 raw="); Serial.print(raw);
    Serial.print("  cat="); Serial.print(cat);
    Serial.print("  hint="); Serial.println(hint);
  }

  delay(20);
}
