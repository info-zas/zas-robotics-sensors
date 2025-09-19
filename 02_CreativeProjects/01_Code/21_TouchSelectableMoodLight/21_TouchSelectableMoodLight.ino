/*
  Mood Light (self-healing OLED): Touch + DHT11 → RGB LED + OLED
  - Robust OLED init: tries 0x3C then 0x3D and retries every 2 s
  - Serial diagnostics; runs even if OLED is absent
  - Tap touch to cycle moods; LED color depends on temperature band

  Pins (UNO/Nano):
    Touch  -> D4  (TTP223 active-HIGH)
    DHT11  -> D8
    RGB    -> D3=R, D6=G, D5=B  (common-cathode; 220–330 Ω resistors if bare LED)
    OLED   -> SDA=A4, SCL=A5
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------- Pins ----------
const int PIN_TOUCH = 4;
const int PIN_DHT   = 8;
const int PIN_R     = 3;
const int PIN_G     = 6;
const int PIN_B     = 5;

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool oledOK = false;
uint8_t oledAddr = 0x3C;
unsigned long lastOLEDtry = 0;
const unsigned long OLED_RETRY_MS = 2000;

// ---------- DHT ----------
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// ---------- Tuning ----------
const unsigned long READ_MS     = 1000;   // OLED/Serial refresh
const unsigned long DEBOUNCE_MS = 150;    // touch debounce
const float COOL_MAX = 22.0;              // < COOL_MAX  -> COOL
const float WARM_MIN = 28.0;              // > WARM_MIN  -> WARM

// ---------- Mood set ----------
enum Mood : uint8_t { RELAX, FOCUS, ENERGY, SLEEP, MOOD_COUNT };
const char* MOOD_NAMES[MOOD_COUNT] = { "Relax", "Focus", "Energy", "Sleep" };
Mood mood = RELAX;

// ---------- State ----------
unsigned long lastPrint = 0;
unsigned long lastTouchTime = 0;
unsigned long lastBeat = 0;

// ---------- LED helpers (Common-Cathode) ----------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(PIN_R, r ? HIGH : LOW);
  digitalWrite(PIN_G, g ? HIGH : LOW);
  digitalWrite(PIN_B, b ? HIGH : LOW);
}
void ledOff()    { setRGB(false,false,false); }
void ledRed()    { setRGB(true, false, false); }
void ledGreen()  { setRGB(false,true,  false); }
void ledBlue()   { setRGB(false,false,true ); }
void ledYellow() { setRGB(true, true,  false); }
void ledCyan()   { setRGB(false,true,  true ); }
void ledMagenta(){ setRGB(true, false, true ); }
void ledWhite()  { setRGB(true, true,  true ); }

// ---------- Touch edge ----------
bool touchRisingEdge() {
  static int prev = LOW;
  int now = digitalRead(PIN_TOUCH);
  bool edge = (now == HIGH && prev == LOW && (millis() - lastTouchTime) > DEBOUNCE_MS);
  if (edge) lastTouchTime = millis();
  prev = now;
  return edge;
}

// ---------- OLED init helpers ----------
bool beginAt(uint8_t addr) {
  Serial.print(F("Trying SSD1306 @ 0x")); Serial.println(addr, HEX);
  if (display.begin(SSD1306_SWITCHCAPVCC, addr)) {
    oledAddr = addr;
    display.setRotation(0);     // use 2 if your text is upside-down
    display.dim(false);         // full brightness
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);  display.println(F("Mood Light"));
    display.setCursor(0,14); display.print(F("OLED OK @ 0x"));
    display.println(oledAddr, HEX);
    display.display();
    Serial.print(F("OLED initialized @ 0x")); Serial.println(oledAddr, HEX);
    return true;
  }
  Serial.println(F("  begin() failed"));
  return false;
}

void tryInitOLED() {
  // Retry not more often than every OLED_RETRY_MS, but ALWAYS try once at startup
  if (lastOLEDtry != 0 && (millis() - lastOLEDtry) < OLED_RETRY_MS) return;
  lastOLEDtry = millis();

  // Try both common addresses regardless of scan result (matches your working tester)
  oledOK = beginAt(0x3C) || beginAt(0x3D);
  if (!oledOK) Serial.println(F("SSD1306 not found yet (will retry)..."));
}

void showOLED(const char* colorName, float T, float H) {
  if (!oledOK) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);   display.println(F("Mood Light"));
  display.setCursor(0,14);  display.print(F("Mood: ")); display.println(MOOD_NAMES[mood]);

  display.setCursor(0,28);
  if (isnan(T) || isnan(H)) display.println(F("DHT: error"));
  else {
    display.print(F("T: ")); display.print(T, 1); display.print(F(" C   "));
    display.print(F("H: ")); display.print(H, 0); display.println(F(" %"));
  }

  display.setCursor(0,44);  display.print(F("Color: ")); display.println(colorName);
  display.display();
}

// ---------- Color selection ----------
const char* applyMoodColor(float T) {
  enum Band { COOL, NEUTRAL, WARM };
  Band band = isnan(T) ? NEUTRAL : ((T < COOL_MAX) ? COOL : (T > WARM_MIN ? WARM : NEUTRAL));

  switch (mood) {
    case RELAX:
      if (band == COOL)     { ledCyan();    return "Cyan (cool relax)"; }
      if (band == WARM)     { ledMagenta(); return "Magenta (warm relax)"; }
                             ledBlue();     return "Blue (relax)";
    case FOCUS:
      if (band == COOL)     { ledWhite();   return "White (crisp focus)"; }
      if (band == WARM)     { ledYellow();  return "Yellow (warm focus)"; }
                             ledGreen();    return "Green (focus)";
    case ENERGY:
      if (band == COOL)     { ledGreen();   return "Green (fresh)"; }
      if (band == WARM)     { ledRed();     return "Red (high energy)"; }
                             ledYellow();   return "Yellow (energy)";
    case SLEEP:
      if (band == COOL)     { ledBlue();    return "Blue (cool sleep)"; }
      if (band == WARM)     { ledMagenta(); return "Magenta (warm sleep)"; }
                             ledCyan();     return "Cyan (sleep)";
  }
  ledOff(); return "Off";
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_TOUCH, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  ledOff();

  Serial.begin(9600);
  delay(200);
  Serial.println(F("\n[Mood Light] Booting..."));

  Wire.begin();
  Wire.setClock(400000);

  tryInitOLED();    // immediate attempt
  dht.begin();
  Serial.println(F("DHT11: begin()"));
}

void loop() {
  // Heartbeat so you know the sketch is alive
  if (millis() - lastBeat > 500) {
    lastBeat = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  // Keep retrying OLED init if it isn't ready
  if (!oledOK) tryInitOLED();

  // Tap to cycle mood
  if (touchRisingEdge()) {
    mood = (Mood)((mood + 1) % MOOD_COUNT);
    Serial.print(F("Mood -> ")); Serial.println(MOOD_NAMES[mood]);
  }

  // Periodic sensor read + UI
  if (millis() - lastPrint > READ_MS) {
    lastPrint = millis();

    float T = dht.readTemperature();   // may be NaN on very first reads
    float H = dht.readHumidity();

    const char* colorName = applyMoodColor(T);
    showOLED(colorName, T, H);

    Serial.print(F("Mood=")); Serial.print(MOOD_NAMES[mood]);
    Serial.print(F("  T=")); Serial.print(T, 1); Serial.print(F("C"));
    Serial.print(F("  H=")); Serial.print(H, 0); Serial.print(F("%"));
    Serial.print(F("  Color=")); Serial.println(colorName);
  }
}
