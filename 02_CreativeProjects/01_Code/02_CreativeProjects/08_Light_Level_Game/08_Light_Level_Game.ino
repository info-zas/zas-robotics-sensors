/*
  Light-Level Game
  ----------------
  Hardware:
    - LDR Sensor: AO -> A0, VCC->5V, GND->GND
    - Touch Sensor (TTP223): SIG->D8, VCC->5V, GND->GND
    - NeoPixel Ring: DIN->D7 (with ~330Ω resistor recommended), VCC->5V, GND->GND

  Behavior:
    - Reads light level (LDR).
    - Sets challenge difficulty (touches needed).
    - Player taps touch sensor to progress.
    - NeoPixel ring fills LEDs as progress bar.
    - On reaching target, ring flashes rainbow for victory.
*/

#include <Adafruit_NeoPixel.h>

// -------- Pins --------
#define PIN_LDR    A0
#define PIN_TOUCH  8   // ✅ Updated: Touch Sensor SIG on D8
#define PIN_RING   7
#define NUM_PIXELS 12  // adjust for your NeoPixel ring size

// -------- NeoPixel --------
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_RING, NEO_GRB + NEO_KHZ800);

// -------- Game variables --------
int targetTouches = 10;   // will be set by light level
int currentTouches = 0;
int lastTouchState = LOW;

// -------- Helper: Rainbow animation --------
uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85)  return strip.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) { pos -= 85; return strip.Color(0, pos * 3, 255 - pos * 3); }
  pos -= 170;   return strip.Color(pos * 3, 255 - pos * 3, 0);
}

void rainbow(int wait) {
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void setup() {
  pinMode(PIN_TOUCH, INPUT);
  strip.begin();
  strip.show(); // all off

  Serial.begin(9600);
  Serial.println("Light-Level Game started!");
}

void loop() {
  // 1) Read light level
  int lightValue = analogRead(PIN_LDR);

  // 2) Set difficulty based on light
  if (lightValue > 700) {
    targetTouches = 5;   // Bright -> easy
  } else if (lightValue > 400) {
    targetTouches = 10;  // Medium
  } else {
    targetTouches = 15;  // Dark -> hard
  }

  // 3) Read touch input (edge detection)
  int touchNow = digitalRead(PIN_TOUCH);
  if (touchNow == HIGH && lastTouchState == LOW) {
    currentTouches++;
    Serial.print("Touch count: ");
    Serial.println(currentTouches);
  }
  lastTouchState = touchNow;

  // 4) Update NeoPixel progress
  int litPixels = map(currentTouches, 0, targetTouches, 0, NUM_PIXELS);
  strip.clear();
  for (int i = 0; i < litPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 150, 255)); // blue progress
  }
  strip.show();

  // 5) Win condition
  if (currentTouches >= targetTouches) {
    Serial.println("YOU WIN!");
    rainbow(10);
    currentTouches = 0; // reset game
  }

  delay(50);
}
