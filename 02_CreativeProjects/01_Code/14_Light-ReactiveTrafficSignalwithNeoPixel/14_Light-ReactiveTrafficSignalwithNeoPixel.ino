/*
  LDR → NeoPixel Traffic-Signal Display (stable)
  - Reads ambient light with an LDR (A0)
  - Uses moving average + hysteresis (no flicker)
  - Shows traffic-signal colors on a NeoPixel ring (D6):
      BRIGHT  -> GREEN (steady)
      MEDIUM  -> YELLOW (steady)
      DARK    -> RED (gentle blink)

  Wiring:
    LDR + 10kΩ voltage divider -> A0  (LDR to 5V, 10k to GND, junction to A0)
    NeoPixel VCC -> 5V, GND -> GND, DIN -> D6
*/

#include <Adafruit_NeoPixel.h>

// ---------- Pins ----------
const int PIN_LDR      = A0;
const int PIN_NEOPIXEL = 6;

// ---------- NeoPixel ----------
#define NUMPIXELS 8  // adjust to your ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------- Smoothing (moving average) ----------
const uint8_t AVG_WIN = 10;     // number of samples in the moving average
int   avgBuf[AVG_WIN];
uint8_t avgIdx = 0;
long  avgSum = 0;

// ---------- Hysteresis thresholds ----------
const int DARK_MAX    = 150;    // <150 definitely dark
const int BRIGHT_MIN  = 700;    // >700 definitely bright
const int HYST_MARGIN = 80;     // leave band only after crossing by this margin

// ---------- Band state ----------
enum Band { DARK, MEDIUM, BRIGHT };
Band band = MEDIUM;

// ---------- Helpers ----------
void fillAll(uint32_t c) {
  for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, c);
  pixels.show();
}

void setup() {
  Serial.begin(9600);

  pixels.begin();
  pixels.setBrightness(120); // 0..255 — raise/lower to taste
  pixels.clear();
  pixels.show();

  // Prefill moving-average buffer for a stable start
  for (uint8_t i = 0; i < AVG_WIN; i++) {
    int v = analogRead(PIN_LDR);
    avgBuf[i] = v;
    avgSum += v;
    delay(2);
  }

  Serial.println("LDR → NeoPixel Traffic-Signal ready");
}

void loop() {
  // ---- Moving average ----
  int raw = analogRead(PIN_LDR);        // 0..1023
  avgSum -= avgBuf[avgIdx];
  avgBuf[avgIdx] = raw;
  avgSum += raw;
  avgIdx = (avgIdx + 1) % AVG_WIN;
  int ldr = avgSum / AVG_WIN;           // smoothed value

  // ---- Hysteresis banding ----
  switch (band) {
    case DARK:
      if (ldr > DARK_MAX + HYST_MARGIN) band = MEDIUM;
      break;
    case MEDIUM:
      if (ldr < DARK_MAX)            band = DARK;
      else if (ldr > BRIGHT_MIN)     band = BRIGHT;
      break;
    case BRIGHT:
      if (ldr < BRIGHT_MIN - HYST_MARGIN) band = MEDIUM;
      break;
  }

  // ---- Traffic-signal colors ----
  static uint32_t lastBlink = 0;
  static bool blinkOn = true;

  if (band == BRIGHT) {
    // GREEN (steady)
    fillAll(pixels.Color(0, 255, 0));
  } else if (band == MEDIUM) {
    // YELLOW (steady)
    fillAll(pixels.Color(255, 180, 0));
  } else { // DARK
    // RED (gentle blink for strong visibility)
    if (millis() - lastBlink > 350) {   // ~3 Hz
      lastBlink = millis();
      blinkOn = !blinkOn;
    }
    fillAll(blinkOn ? pixels.Color(255, 0, 0)
                    : pixels.Color(12, 0, 0));  // dim between blinks
  }

  // ---- Debug ----
  Serial.print("LDR raw="); Serial.print(raw);
  Serial.print(" avg="); Serial.print(ldr);
  Serial.print(" band=");
  Serial.println(band == BRIGHT ? "BRIGHT" : band == MEDIUM ? "MEDIUM" : "DARK");

  delay(100);
}
