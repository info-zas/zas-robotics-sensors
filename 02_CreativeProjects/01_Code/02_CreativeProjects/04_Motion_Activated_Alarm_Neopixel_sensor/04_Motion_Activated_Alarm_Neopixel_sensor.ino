/*
  Motion-Activated Alarm (IR Sensor + NeoPixel Ring)
  --------------------------------------------------
  Hardware:
    IR Sensor:
      - VCC -> 5V
      - GND -> GND
      - OUT -> D8  (many modules pull OUT LOW when motion/obstacle is detected)

    NeoPixel Ring (WS2812 / SK6812 compatible):
      - DIN -> D6   (through 330-470Ω resistor recommended)
      - 5V  -> 5V   (large rings may need external 5V supply)
      - GND -> GND  (MUST share ground with Arduino)

  Behavior:
    - If motion detected -> flash RED (alarm)
    - If no motion       -> steady GREEN (safe)
    - Serial Monitor prints current state for debugging

  Library:
    - Install "Adafruit NeoPixel" from Library Manager (Sketch > Include Library > Manage Libraries)
*/

#include <Adafruit_NeoPixel.h>

// ----------- USER SETTINGS (change these if needed) -----------
#define PIN_IR          8          // IR sensor digital output pin
#define PIN_NEOPIXEL    6          // NeoPixel ring DIN pin

#define NUM_PIXELS      12         // <-- set to your ring size (8, 12, 16, etc.)
#define BRIGHTNESS      80         // 0..255 overall brightness limit

#define IR_ACTIVE_LOW   1          // most obstacle IR modules: LOW = detected
// If your sensor gives HIGH when motion is detected, set IR_ACTIVE_LOW to 0.
// ---------------------------------------------------------------

// Create the NeoPixel strip object
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Small helper to convert (r,g,b) to a 32-bit color
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) { return strip.Color(r, g, b); }

// Fill the whole ring with one color and show it
void fillRing(uint32_t color) {
  for (int i = 0; i < NUM_PIXELS; i++) strip.setPixelColor(i, color);
  strip.show();
}

// Blink the whole ring (color ON -> OFF) once with given on/off times (ms)
void flashAll(uint32_t color, uint16_t onMs, uint16_t offMs) {
  fillRing(color);
  delay(onMs);
  strip.clear();
  strip.show();
  delay(offMs);
}

void setup() {
  pinMode(PIN_IR, INPUT);     // digital input from IR module
  strip.begin();              // init NeoPixels
  strip.setBrightness(BRIGHTNESS);
  strip.clear();
  strip.show();

  Serial.begin(9600);
  Serial.println(F("Motion-Activated Alarm (IR + NeoPixel) ready."));
  Serial.println(F("Tip: If alarm never triggers, flip IR_ACTIVE_LOW."));
}

void loop() {
  // Read the IR sensor
  int raw = digitalRead(PIN_IR);

  // Normalize to "motionDetected = true/false" regardless of active level
  bool motionDetected = IR_ACTIVE_LOW ? (raw == LOW) : (raw == HIGH);

  if (motionDetected) {
    Serial.println(F("Motion DETECTED -> ALARM"));
    // Alarm effect: flash red a few times
    for (int i = 0; i < 2; i++) {                 // number of flashes per detection
      flashAll(rgb(255, 0, 0), 250, 250);         // red on/off 250ms
    }
  } else {
    // Safe state: steady green (or choose 'off' by using strip.clear())
    Serial.println(F("No motion -> SAFE"));
    fillRing(rgb(0, 180, 0));                     // steady green
    delay(150);                                   // small delay to reduce serial spam
  }
}
