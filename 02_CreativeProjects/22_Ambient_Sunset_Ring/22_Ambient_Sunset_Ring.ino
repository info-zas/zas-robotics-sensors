/*
  Ambient “Sunset” Ring
  LDR (A0) -> NeoPixel ring (D6)
  As the room darkens, color transitions: White -> Orange -> Red
  Includes moving-average smoothing for calm, time-based transitions.
*/

#include <Adafruit_NeoPixel.h>

// -------- User settings --------
#define PIN_LDR        A0
#define PIN_NEOPIXEL   6
#define NUMPIXELS      12          // set to your ring size (8/12/16/etc.)
#define NP_BRIGHTNESS  120         // 0..255 global brightness

// Quick calibration (use Serial to fine-tune):
// Readings seen in BRIGHT room and in DARK cover; adjust these bounds.
const int LDR_BRIGHT = 900;        // LDR value in bright light (typical 800..1023)
const int LDR_DARK   = 100;        // LDR value in darkness (typical 0..200)

// Smoothing (time-based). Larger window = smoother/slower response.
const uint8_t SMOOTH_WINDOW = 12;  // samples in the moving average
const bool    ENABLE_BREATH = true; // subtle breathing (aesthetic)

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Moving average buffer for LDR
int   ldrBuf[SMOOTH_WINDOW];
uint8_t ldrIdx = 0;
long  ldrSum = 0;

unsigned long lastPrint = 0;

// -------- Helpers --------

// Linear map and clamp 0..1
float norm01(int x, int inMin, int inMax) {
  if (inMax == inMin) return 0.0f;
  float t = (float)(x - inMin) / (float)(inMax - inMin);
  if (t < 0) t = 0; else if (t > 1) t = 1;
  return t;
}

// Lerp between two bytes
uint8_t lerp8(uint8_t a, uint8_t b, float t) {
  float v = a + (b - a) * t;
  if (v < 0) v = 0; if (v > 255) v = 255;
  return (uint8_t)(v + 0.5f);
}

// Map darkness (0..1) to color: 0..0.5 White->Orange, 0.5..1 Orange->Red
uint32_t colorFromDarkness(float d) {
  // White (255,255,255), Orange (255,165,0), Red (255,0,0)
  uint8_t r, g, b;
  if (d <= 0.5f) {
    float t = d / 0.5f;               // 0..1
    r = lerp8(255, 255, t);
    g = lerp8(255, 165, t);
    b = lerp8(255,   0, t);
  } else {
    float t = (d - 0.5f) / 0.5f;      // 0..1
    r = lerp8(255, 255, t);
    g = lerp8(165,   0, t);
    b = lerp8(  0,   0, t);
  }
  return pixels.Color(r, g, b);
}

// Moving-average read of LDR
int readLDRSmooth() {
  int v = analogRead(PIN_LDR);
  ldrSum -= ldrBuf[ldrIdx];
  ldrBuf[ldrIdx] = v;
  ldrSum += v;
  ldrIdx = (ldrIdx + 1) % SMOOTH_WINDOW;
  return (int)(ldrSum / SMOOTH_WINDOW);
}

void setup() {
  Serial.begin(9600);
  pixels.begin();
  pixels.setBrightness(NP_BRIGHTNESS);
  pixels.clear();
  pixels.show();

  // Prefill smoothing buffer
  for (uint8_t i = 0; i < SMOOTH_WINDOW; i++) {
    int v = analogRead(PIN_LDR);
    ldrBuf[i] = v;
    ldrSum += v;
    delay(2);
  }

  Serial.println(F("Ambient Sunset Ring started"));
  Serial.println(F("Cover/uncover the LDR and watch color change."));
  Serial.println(F("Tweak LDR_BRIGHT/LDR_DARK if needed."));
}

void loop() {
  // 1) Read LDR with smoothing
  int ldr = readLDRSmooth();

  // 2) Convert to brightness (0..1), then to darkness (0..1)
  float bright = norm01(ldr, LDR_DARK, LDR_BRIGHT); // 0 dark .. 1 bright
  float dark   = 1.0f - bright;                    // 0 bright .. 1 dark

  // 3) Optional gentle breathing (very subtle, independent of LDR)
  float breath = 1.0f;
  if (ENABLE_BREATH) {
    // sine between 0.85 and 1.00 over ~3 seconds
    float s = sin(2.0f * PI * (millis() % 3000) / 3000.0f);
    breath = 0.925f + 0.075f * (s * 0.5f + 0.5f);
  }

  // 4) Choose target color based on darkness and apply optional breath
  uint32_t col = colorFromDarkness(dark);

  // Apply "breath" by scaling the final color (simple per-pixel dimming)
  uint8_t r = (uint8_t)( ((col >> 16) & 0xFF) * breath );
  uint8_t g = (uint8_t)( ((col >>  8) & 0xFF) * breath );
  uint8_t b = (uint8_t)( ( col        & 0xFF) * breath );
  uint32_t colBreath = pixels.Color(r, g, b);

  // 5) Show on ring (all pixels same color)
  for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, colBreath);
  pixels.show();

  // 6) Debug once per second
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print(F("LDR=")); Serial.print(ldr);
    Serial.print(F("  bright=")); Serial.print(bright, 2);
    Serial.print(F("  dark=")); Serial.print(dark, 2);
    Serial.print(F("  color RGB=(")); Serial.print(r); Serial.print(",");
    Serial.print(g); Serial.print(","); Serial.print(b); Serial.println(")");
  }

  delay(20);  // ~50 FPS update
}
