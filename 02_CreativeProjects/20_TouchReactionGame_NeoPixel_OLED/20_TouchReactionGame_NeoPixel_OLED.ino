/*
  Touch-Reaction Game: NeoPixel + OLED
  ------------------------------------
  Hardware:
    - Touch sensor (TTP223) on D7 (active HIGH)
    - NeoPixel ring on D6 (WS2812)
    - SSD1306 OLED 128x64 (I2C A4/A5, addr 0x3C)

  Game flow:
    READY : "Tap to arm"     -> single tap -> ARMED
    ARMED : wait random 1–3s -> light random pixel -> GO
    GO    : wait for tap     -> measure reaction ms -> RESULT
    RESULT: show time ~2s    -> back to READY
    False start during ARMED -> "Too Soon!" message

  Non-blocking (millis), clean rising-edge touch detection.
*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ------- Pins -------
const int PIN_TOUCH    = 4;     // TTP223 OUT (active HIGH)
const int PIN_NEOPIXEL = 6;     // WS2812 DIN

// ------- NeoPixel -------
#define NUMPIXELS 8             // set to your ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ------- OLED -------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------- Game tuning -------
const unsigned long RAND_MIN_MS   = 1000;   // min random wait
const unsigned long RAND_MAX_MS   = 3000;   // max random wait
const unsigned long RESULT_HOLDMS = 2000;   // show result this long
const unsigned long OLED_REFRESH  = 300;    // throttle OLED updates
const unsigned long DEBOUNCE_MS   = 120;    // touch debounce
const uint8_t NP_BRIGHTNESS       = 120;    // 0..255 NeoPixel global brightness

// ------- State machine -------
enum GameState : uint8_t { READY, ARMED, GO, RESULT };
GameState state = READY;

// ------- Timing & scores -------
unsigned long tStateStart = 0;   // when we entered current state
unsigned long tGoLit      = 0;   // when GO pixel lit (start of timing)
unsigned long reactionMs  = 0;   // last reaction time
unsigned long bestMs      = 0xFFFFFFFF; // best (lower is better)
unsigned long nextOLED    = 0;

// ------- Other vars -------
int goPixel = 0;                 // which NeoPixel index is "GO"
unsigned long randWait = 0;      // ARMED random delay
unsigned long lastTouchTime = 0; // debounce

// ------- Small helpers -------
void npClear() { pixels.clear(); pixels.show(); }
void npAll(uint32_t c) { for (int i=0;i<NUMPIXELS;i++) pixels.setPixelColor(i,c); pixels.show(); }

bool touchRisingEdge() {
  static int last = LOW;
  int now = digitalRead(PIN_TOUCH);
  bool edge = (now == HIGH && last == LOW && (millis() - lastTouchTime) > DEBOUNCE_MS);
  if (edge) lastTouchTime = millis();
  last = now;
  return edge;
}

void oledLines(const char* l1, const char* l2 = "", const char* l3 = "") {
  if (millis() < nextOLED) return;
  nextOLED = millis() + OLED_REFRESH;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);  display.println("Reaction Game");
  display.setCursor(0,16); display.println(l1);
  if (*l2) { display.setCursor(0,28); display.println(l2); }
  if (*l3) { display.setCursor(0,40); display.println(l3); }
  if (bestMs != 0xFFFFFFFF) {
    display.setCursor(0,56);
    display.print("Best: "); display.print(bestMs); display.print(" ms");
  }
  display.display();
}

void toState(GameState s) {
  state = s;
  tStateStart = millis();
  // Per-state entry actions
  switch (state) {
    case READY:
      npClear();
      oledLines("Tap to ARM", "Get ready...");
      break;
    case ARMED:
      npAll(pixels.Color(12, 12, 12));      // dim ring while waiting
      randWait = random(RAND_MIN_MS, RAND_MAX_MS + 1);
      oledLines("Armed...", "Wait for light");
      break;
    case GO:
      goPixel = random(NUMPIXELS);
      pixels.clear();
      pixels.setPixelColor(goPixel, pixels.Color(0, 255, 0)); // bright GREEN
      pixels.show();
      tGoLit = millis();                    // start timing
      oledLines("GO! Tap now");
      break;
    case RESULT:
      // Simple blink on the GO pixel while showing result
      break;
  }
}

void setup() {
  pinMode(PIN_TOUCH, INPUT);
  Serial.begin(9600);

  // Seed RNG with floating analog pin
  randomSeed(analogRead(A0));

  pixels.begin();
  pixels.setBrightness(NP_BRIGHTNESS);
  pixels.clear();
  pixels.show();

  Wire.begin(); Wire.setClock(400000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1) { /* OLED required for this demo */ }
  }

  toState(READY);
}

void loop() {
  // ---- State machine ----
  switch (state) {
    case READY:
      if (touchRisingEdge()) {
        toState(ARMED);
      }
      break;

    case ARMED:
      // False start: touched before GO
      if (touchRisingEdge()) {
        npAll(pixels.Color(255, 0, 0));  // red flash
        oledLines("Too soon!", "Penalty: 0 ms");
        reactionMs = 0;
        delay(300);
        toState(RESULT);
        break;
      }
      // Time to light GO?
      if (millis() - tStateStart >= randWait) {
        toState(GO);
      }
      break;

    case GO:
      if (touchRisingEdge()) {
        reactionMs = millis() - tGoLit;
        if (reactionMs < bestMs) bestMs = reactionMs;

        // Flash success (blue) briefly
        pixels.setPixelColor(goPixel, pixels.Color(0, 0, 255));
        pixels.show();

        char line2[24];
        snprintf(line2, sizeof(line2), "Time: %lu ms", reactionMs);
        oledLines("Nice!", line2);
        toState(RESULT);
      }
      break;

    case RESULT:
      // Blink the chosen pixel while we show result text
      if (((millis() / 200) % 2) == 0) {
        pixels.setPixelColor(goPixel, pixels.Color(255, 255, 0)); // yellow
      } else {
        pixels.setPixelColor(goPixel, pixels.Color(0, 0, 0));
      }
      pixels.show();

      // After hold period, return to READY
      if (millis() - tStateStart > RESULT_HOLDMS) {
        toState(READY);
      }
      break;
  }

  // Optional serial debug once per second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    Serial.print("State="); Serial.print(state);
    Serial.print("  Best="); Serial.print(bestMs == 0xFFFFFFFF ? 0 : bestMs);
    Serial.println(" ms");
  }
}
