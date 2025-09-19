/*
  Interactive NeoPixel Canvas
  Inputs : Touch (D4), LDR (A0)
  Output : NeoPixel Ring (D6)
  Behavior:
    - Touch cycles patterns
    - LDR auto-sets global brightness (brighter room -> brighter LEDs)
    - Smooth animations, debounced touch
*/

#include <Adafruit_NeoPixel.h>

// ------------ User Pins ------------
#define PIN_TOUCH     4
#define PIN_LDR       A0
#define PIN_NEOPIXEL  6

// ------------ Ring Settings ------------
#define NUMPIXELS     12        // set to your ring size
#define NP_BRIGHTNESS 150       // default; will be overridden by LDR mapping

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ------------ LDR Calibration ------------
// With wiring 5V -> LDR -> A0 -> 10k -> GND, A0 is HIGH when BRIGHT.
// Read your Serial values and tune these:
const int LDR_BRIGHT = 900;     // value in a bright room (typ. 800..1023)
const int LDR_DARK   = 120;     // value with sensor covered (typ. 0..200)

// Brightness range to use (0..255). Keep a floor so dark doesn’t go fully off.
const uint8_t BRIGHT_MIN = 60;
const uint8_t BRIGHT_MAX = 255;

// Smoothing window for LDR
const uint8_t LDR_WIN = 12;
int   ldrBuf[LDR_WIN];
uint8_t ldrIdx = 0;
long  ldrSum = 0;

// Timing / debounce
const unsigned long DEBOUNCE_MS = 150;
unsigned long lastTouch = 0;
unsigned long lastFrame = 0;
unsigned long lastPrint = 0;

// Patterns
enum Pattern : uint8_t { SOLID, COMET, THEATER, RAINBOW, SPARKLE, PATTERN_COUNT };
Pattern pattern = SOLID;

uint8_t baseHue = 0;  // evolves over time for SOLID/COMET/SPARKLE
uint16_t frame = 0;   // animation tick

// ---------- Small Helpers ----------
uint8_t clamp8(int v) { if (v<0) return 0; if (v>255) return 255; return (uint8_t)v; }

// map & clamp float 0..1
float norm01(int x, int inMin, int inMax) {
  if (inMax == inMin) return 0.0f;
  float t = (float)(x - inMin) / (float)(inMax - inMin);
  if (t < 0) t = 0; else if (t > 1) t = 1;
  return t;
}

// Wheel -> RGB (0..255)
uint32_t wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return pixels.Color(255 - pos*3, 0, pos*3);
  } else if (pos < 170) {
    pos -= 85;
    return pixels.Color(0, pos*3, 255 - pos*3);
  } else {
    pos -= 170;
    return pixels.Color(pos*3, 255 - pos*3, 0);
  }
}

// Moving-average LDR
int readLDRSmooth() {
  int v = analogRead(PIN_LDR);
  ldrSum -= ldrBuf[ldrIdx];
  ldrBuf[ldrIdx] = v;
  ldrSum += v;
  ldrIdx = (ldrIdx + 1) % LDR_WIN;
  return (int)(ldrSum / LDR_WIN);
}

// Touch rising edge w/ debounce
bool touchRising() {
  static int prev = LOW;
  int now = digitalRead(PIN_TOUCH);
  bool edge = (now == HIGH && prev == LOW && (millis() - lastTouch) > DEBOUNCE_MS);
  if (edge) lastTouch = millis();
  prev = now;
  return edge;
}

// ------------ Patterns ------------
void patSolid() {
  // hue cycles slowly
  uint32_t col = wheel(baseHue);
  for (int i=0; i<NUMPIXELS; i++) pixels.setPixelColor(i, col);
}

void patComet() {
  int head = (frame / 2) % NUMPIXELS;           // speed
  // fade all pixels a bit
  for (int i=0; i<NUMPIXELS; i++) {
    uint32_t c = pixels.getPixelColor(i);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >>  8) & 0xFF;
    uint8_t b =  c        & 0xFF;
    r = (uint8_t)(r * 0.75); g = (uint8_t)(g * 0.75); b = (uint8_t)(b * 0.75);
    pixels.setPixelColor(i, r, g, b);
  }
  // bright head with wheel color
  pixels.setPixelColor(head, wheel(baseHue));
}

void patTheater() {
  // classic theater chase (every 3rd pixel)
  uint8_t phase = (frame / 3) % 3;
  uint32_t col = wheel(baseHue);
  for (int i=0; i<NUMPIXELS; i++) {
    if (i % 3 == phase) pixels.setPixelColor(i, col);
    else                pixels.setPixelColor(i, 0);
  }
}

void patRainbow() {
  for (int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, wheel((i*256/NUMPIXELS + frame) & 0xFF));
  }
}

void patSparkle() {
  // faint base, random sparkles pop
  uint32_t base = wheel(baseHue);
  // dim base
  uint8_t br = ((base >> 16) & 0xFF) / 8;
  uint8_t bg = ((base >>  8) & 0xFF) / 8;
  uint8_t bb = ( base        & 0xFF) / 8;
  for (int i=0; i<NUMPIXELS; i++) pixels.setPixelColor(i, br, bg, bb);
  // a few bright specks
  for (int s=0; s<2; s++) {
    int p = random(NUMPIXELS);
    pixels.setPixelColor(p, wheel(random(256)));
  }
}

// ------------ Main ------------
void setup() {
  pinMode(PIN_TOUCH, INPUT);
  pixels.begin();
  pixels.setBrightness(NP_BRIGHTNESS);
  pixels.clear(); pixels.show();

  Serial.begin(9600);
  delay(200);
  Serial.println(F("Interactive NeoPixel Canvas"));

  // Prefill LDR smoothing
  for (uint8_t i=0; i<LDR_WIN; i++) {
    int v = analogRead(PIN_LDR);
    ldrBuf[i] = v;
    ldrSum += v;
    delay(2);
  }

  // Power-on wipe
  for (int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, wheel(i*256/NUMPIXELS));
    pixels.show(); delay(30);
  }
}

void loop() {
  // 1) Touch -> next pattern
  if (touchRising()) {
    pattern = (Pattern)((pattern + 1) % PATTERN_COUNT);
    Serial.print(F("Pattern -> "));
    switch(pattern){
      case SOLID:   Serial.println(F("SOLID")); break;
      case COMET:   Serial.println(F("COMET")); break;
      case THEATER: Serial.println(F("THEATER")); break;
      case RAINBOW: Serial.println(F("RAINBOW")); break;
      case SPARKLE: Serial.println(F("SPARKLE")); break;
    }
  }

  // 2) LDR -> global brightness (bright room => bright LEDs)
  int ldr = readLDRSmooth();
  float bright01 = norm01(ldr, LDR_DARK, LDR_BRIGHT);  // 0 dark .. 1 bright
  uint8_t b = (uint8_t)(BRIGHT_MIN + bright01 * (BRIGHT_MAX - BRIGHT_MIN));
  pixels.setBrightness(b);

  // 3) Animate at ~50 FPS
  if (millis() - lastFrame > 20) {
    lastFrame = millis();
    baseHue++;           // slow hue drift for some patterns
    frame++;

    switch(pattern) {
      case SOLID:   patSolid();   break;
      case COMET:   patComet();   break;
      case THEATER: patTheater(); break;
      case RAINBOW: patRainbow(); break;
      case SPARKLE: patSparkle(); break;
    }
    pixels.show();
  }

  // 4) Debug once per second
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print(F("LDR=")); Serial.print(ldr);
    Serial.print(F("  bright01=")); Serial.print(bright01, 2);
    Serial.print(F("  setBrightness=")); Serial.println(b);
  }
}
