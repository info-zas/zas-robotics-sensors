/*
  Touch-Controlled NeoPixel Scenes with OLED
  ------------------------------------------
  Inputs : TTP223 Touch Sensor on D7 (active HIGH)
  Outputs: NeoPixel Ring on D6, SSD1306 OLED (I2C A4/A5)

  Scenes (cycle on each touch):
    0 OFF
    1 Solid RED
    2 Solid GREEN
    3 Solid BLUE
    4 Rainbow Cycle
    5 Theater Chase
    6 Breathing White
    7 Comet (moving dot with trail)

  Design notes:
    - Debounced rising-edge detection for touch
    - Non-blocking animations (millis timers)
    - OLED shows current scene name
*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ---------------- Pins ----------------
const int PIN_TOUCH    = 4;   // TTP223 output (active HIGH on touch)
const int PIN_NEOPIXEL = 6;   // WS2812 DIN

// ---------------- NeoPixel ----------------
#define NUMPIXELS 8           // set to your ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Timing ----------------
const unsigned long DEBOUNCE_MS = 200;

// Per-scene step intervals (tunable)
const unsigned long STEP_RAINBOW_MS = 20;
const unsigned long STEP_CHASE_MS   = 80;
const unsigned long STEP_BREATHE_MS = 18;
const unsigned long STEP_COMET_MS   = 40;
const unsigned long OLED_REFRESH_MS = 400;

// ---------------- Scene State ----------------
enum Scene : uint8_t {
  SCN_OFF = 0,
  SCN_RED,
  SCN_GREEN,
  SCN_BLUE,
  SCN_RAINBOW,
  SCN_CHASE,
  SCN_BREATHE,
  SCN_COMET,
  SCN_COUNT
};

Scene scene = SCN_OFF;
const char* SCENE_NAMES[SCN_COUNT] = {
  "OFF",
  "Solid RED",
  "Solid GREEN",
  "Solid BLUE",
  "Rainbow",
  "Theater Chase",
  "Breathing",
  "Comet"
};

// Animation internals
unsigned long tLastStep = 0;     // generic step timer
unsigned long tLastOLED = 0;     // OLED throttle
unsigned long tLastTouch = 0;    // debounce
int animStep = 0;                // generic animation counter
int cometPos = 0;                // comet head position
int chasePhase = 0;              // theater-chase phase
int breatheLevel = 0;            // 0..255
int breatheDir = +1;             // +1/-1

// ---------------- Helpers ----------------
void showSceneOLED() {
  if (millis() - tLastOLED < OLED_REFRESH_MS) return;
  tLastOLED = millis();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Touch Scene Player");

  display.setCursor(0,16);
  display.print("Scene: ");
  display.println(SCENE_NAMES[scene]);

  display.display();
}

void setAll(uint8_t r, uint8_t g, uint8_t b) {
  for (int i=0; i<NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(r,g,b));
  pixels.show();
}

// Color wheel helper (0..255)
uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if(pos < 85)  return pixels.Color(255 - pos*3, 0, pos*3);
  if(pos < 170) { pos -= 85; return pixels.Color(0, pos*3, 255 - pos*3); }
  pos -= 170;   return pixels.Color(pos*3, 255 - pos*3, 0);
}

// Reset per-scene animation variables
void resetAnimation() {
  tLastStep = 0;
  animStep = 0;
  chasePhase = 0;
  breatheLevel = 0;
  breatheDir = +1;
  cometPos = 0;
}

// Handle one animation frame (non-blocking)
void runScene() {
  switch (scene) {
    case SCN_OFF:
      // Draw once when entering
      if (animStep == 0) { setAll(0,0,0); animStep = 1; }
      break;

    case SCN_RED:
      if (animStep == 0) { setAll(255,0,0); animStep = 1; }
      break;

    case SCN_GREEN:
      if (animStep == 0) { setAll(0,255,0); animStep = 1; }
      break;

    case SCN_BLUE:
      if (animStep == 0) { setAll(0,0,255); animStep = 1; }
      break;

    case SCN_RAINBOW:
      if (millis() - tLastStep >= STEP_RAINBOW_MS) {
        tLastStep = millis();
        for (int i=0; i<NUMPIXELS; i++) {
          pixels.setPixelColor(i, wheel((i*256/NUMPIXELS + animStep) & 255));
        }
        pixels.show();
        animStep = (animStep + 1) & 255;
      }
      break;

    case SCN_CHASE:  // theater chase (on 1 of every 3 LEDs, phase shifted)
      if (millis() - tLastStep >= STEP_CHASE_MS) {
        tLastStep = millis();
        for (int i=0; i<NUMPIXELS; i++) {
          if ((i + chasePhase) % 3 == 0) pixels.setPixelColor(i, pixels.Color(255, 180, 0)); // yellow
          else                           pixels.setPixelColor(i, 0);
        }
        pixels.show();
        chasePhase = (chasePhase + 1) % 3;
      }
      break;

    case SCN_BREATHE:  // white breathing by modulating brightness
      if (millis() - tLastStep >= STEP_BREATHE_MS) {
        tLastStep = millis();
        breatheLevel += breatheDir * 4;   // speed
        if (breatheLevel >= 255) { breatheLevel = 255; breatheDir = -1; }
        if (breatheLevel <= 0)   { breatheLevel = 0;   breatheDir = +1; }
        setAll(breatheLevel, breatheLevel, breatheLevel);
      }
      break;

    case SCN_COMET: { // bright head with fading tail
      if (millis() - tLastStep >= STEP_COMET_MS) {
        tLastStep = millis();
        // fade all pixels slightly
        for (int i=0; i<NUMPIXELS; i++) {
          uint32_t c = pixels.getPixelColor(i);
          uint8_t r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
          r = (r * 60) / 100; g = (g * 60) / 100; b = (b * 60) / 100; // 40% fade
          pixels.setPixelColor(i, pixels.Color(r,g,b));
        }
        // draw head
        pixels.setPixelColor(cometPos, pixels.Color(0, 255, 255)); // cyan head
        pixels.show();
        cometPos = (cometPos + 1) % NUMPIXELS;
      }
    } break;
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_TOUCH, INPUT);
  pixels.begin();
  pixels.setBrightness(110); // 0..255
  pixels.clear(); pixels.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
  }
  display.clearDisplay(); display.display();

  showSceneOLED();
  resetAnimation();
}

void loop() {
  // ---- Debounced rising-edge detection for touch ----
  static int prevTouch = LOW;
  int now = digitalRead(PIN_TOUCH);
  if (now == HIGH && prevTouch == LOW && millis() - tLastTouch > DEBOUNCE_MS) {
    tLastTouch = millis();
    // next scene
    scene = (Scene)((scene + 1) % SCN_COUNT);
    Serial.print("Touch! Scene -> "); Serial.println(SCENE_NAMES[scene]);
    showSceneOLED();
    resetAnimation();
  }
  prevTouch = now;

  // ---- Run current scene animation ----
  runScene();

  // ---- Update OLED periodically (keeps it responsive if animations are static) ----
  showSceneOLED();
}
