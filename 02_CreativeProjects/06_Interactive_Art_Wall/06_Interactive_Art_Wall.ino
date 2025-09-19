/*
  Interactive Art Wall
  --------------------
  Touch sensor (SIG -> D6) changes NeoPixel patterns.
  NeoPixel ring (DIN -> D7) shows animations.
  OLED displays the current pattern name.

  Hardware:
    - Touch (TTP223): VCC->5V, GND->GND, SIG->D6
    - NeoPixel Ring:  DIN->D7 (with ~330Ω recommended), 5V->5V, GND->GND
    - OLED SSD1306:   SDA->A4, SCL->A5, VCC->5V, GND->GND
*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// -------- Pins (UPDATED) --------
#define PIN_TOUCH     6    // Touch SIG
#define PIN_NEOPIXEL  7    // NeoPixel data
#define NUM_PIXELS    12   // change to your ring size
#define BRIGHTNESS    80
#define TOUCH_COOLDOWN_MS 250

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// NeoPixel setup
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Pattern state
enum Pattern { SOLID, RAINBOW, THEATER, WIPE, BREATHE, PATTERN_COUNT };
int patternIndex = SOLID;

unsigned long lastTouchTime = 0;
int lastTouchState = LOW;

// --- OLED helper ---
void oledShow(const char* title, const char* hint) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(title);

  display.setTextSize(1);
  display.setCursor(0, 36);
  display.println(hint);
  display.setCursor(0, 52);
  display.println("Tap to change");
  display.display();
}

// --- Color helper ---
uint32_t rgb(byte r, byte g, byte b) { return strip.Color(r, g, b); }

// --- Patterns ---
void patternSolid() {
  static byte which = 0;
  const uint32_t palette[] = {
    rgb(255,0,0), rgb(0,255,0), rgb(0,0,255),
    rgb(255,255,0), rgb(255,0,255), rgb(0,255,255), rgb(255,255,255)
  };
  for (int i=0; i<NUM_PIXELS; i++) strip.setPixelColor(i, palette[which]);
  strip.show();
  delay(600);
  which = (which + 1) % (sizeof(palette)/sizeof(palette[0]));
}

uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if(pos < 85) return strip.Color(255 - pos*3, 0, pos*3);
  if(pos < 170) { pos -= 85; return strip.Color(0, pos*3, 255 - pos*3); }
  pos -= 170; return strip.Color(pos*3, 255 - pos*3, 0);
}
void patternRainbow() {
  static byte j=0;
  for (int i=0; i<NUM_PIXELS; i++) strip.setPixelColor(i, wheel((i+j)&255));
  strip.show();
  delay(20);
  j++;
}

void patternTheaterChase() {
  static byte q=0;
  uint32_t c = rgb(255,80,0);
  strip.clear();
  for (int i=q; i<NUM_PIXELS; i+=3) strip.setPixelColor(i, c);
  strip.show();
  delay(120);
  q = (q+1)%3;
}

void patternWipe() {
  static int i=0;
  static uint32_t c = rgb(0,150,255);
  strip.setPixelColor(i, c);
  strip.show();
  i++;
  if(i>=NUM_PIXELS) { i=0; c=rgb(random(256),random(256),random(256)); strip.clear(); }
  delay(50);
}

void patternBreathe() {
  static int b=0; static int dir=1;
  uint32_t c = rgb(80,0,160);
  for(int i=0;i<NUM_PIXELS;i++) strip.setPixelColor(i,c);
  strip.setBrightness(map(b,0,255,10,BRIGHTNESS));
  strip.show();
  b += dir*4;
  if(b>=255 || b<=0) dir=-dir;
  delay(20);
}

// --- Run pattern ---
void runPattern(int idx) {
  switch(idx) {
    case SOLID:   patternSolid(); break;
    case RAINBOW: patternRainbow(); break;
    case THEATER: patternTheaterChase(); break;
    case WIPE:    patternWipe(); break;
    case BREATHE: patternBreathe(); break;
  }
}

void showTitleFor(int idx) {
  switch(idx) {
    case SOLID:   oledShow("SOLID","Calm color loops"); break;
    case RAINBOW: oledShow("RAINBOW","Flowing spectrum"); break;
    case THEATER: oledShow("CHASE","Retro marquee"); break;
    case WIPE:    oledShow("WIPE","Paint the ring"); break;
    case BREATHE: oledShow("BREATHE","Soft pulse"); break;
  }
}

void setup() {
  pinMode(PIN_TOUCH, INPUT);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.clear(); strip.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.begin(9600);
    Serial.println("OLED failed, LEDs still run.");
  } else {
    oledShow("SOLID","Calm color loops");
  }

  Serial.begin(9600);
  Serial.println("Interactive Art Wall ready. Tap sensor to change.");
}

void loop() {
  int touchNow = digitalRead(PIN_TOUCH);
  if (touchNow == HIGH && lastTouchState == LOW &&
      (millis()-lastTouchTime) > TOUCH_COOLDOWN_MS) {
    patternIndex = (patternIndex+1) % PATTERN_COUNT;
    lastTouchTime = millis();
    showTitleFor(patternIndex);
    Serial.print("Pattern -> "); Serial.println(patternIndex);
    strip.clear(); strip.show();
  }
  lastTouchState = touchNow;

  runPattern(patternIndex);
}
