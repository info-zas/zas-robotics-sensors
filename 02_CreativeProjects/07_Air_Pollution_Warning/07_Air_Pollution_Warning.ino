/*
  Air Pollution Warning – MQ-135 + OLED
  -------------------------------------
  Hardware:
    MQ-135: AO -> A0, VCC -> 5V, GND -> GND
    OLED SSD1306: SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND

  Behavior:
    - Reads MQ-135 analog value (0..1023) with averaging
    - OLED shows value and status (GOOD / MODERATE / POOR)
    - "NO SENSOR?" message if A0 looks floating
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------- Pins --------
const int PIN_MQ   = A0;  // MQ-135 AO -> A0

// -------- OLED --------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- Tuning --------
const int  GOOD_MAX        = 300;    // <300 -> GOOD
const int  MODERATE_MAX    = 600;    // 300..600 -> MODERATE; >600 -> POOR
const byte AVERAGE_SAMPLES = 16;     // smoothing
const unsigned long REFRESH_MS = 1000;
const unsigned long WARMUP_SECONDS = 30;

// -------- Helpers --------
// Average A0 readings
int readAirAnalog() {
  analogRead(PIN_MQ); // throw away first sample
  long sum = 0;
  for (byte i = 0; i < AVERAGE_SAMPLES; i++) {
    sum += analogRead(PIN_MQ);
    delay(2);
  }
  return (int)(sum / AVERAGE_SAMPLES);
}

// Guess "floating" pin if no sensor connected
bool looksFloating(int current, int last) {
  int delta = abs(current - last);
  return (current > 150 && current < 900 && delta > 50);
}

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed (address 0x3C).");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Air Pollution Monitor");
  display.display();
}

void loop() {
  static unsigned long lastUpdate = 0;
  static int lastValue = 0;

  if (millis() - lastUpdate < REFRESH_MS) return;
  lastUpdate = millis();

  // 1) Read sensor
  int airValue = readAirAnalog();

  // 2) Decide status
  String statusText;
  if (looksFloating(airValue, lastValue)) {
    statusText = "NO SENSOR?";
  } else if (airValue < GOOD_MAX) {
    statusText = "GOOD";
  } else if (airValue < MODERATE_MAX) {
    statusText = "MODERATE";
  } else {
    statusText = "POOR";
  }
  lastValue = airValue;

  // 3) Warm-up hint
  unsigned long secs = millis() / 1000UL;
  bool showWarmup = (secs < WARMUP_SECONDS);

  // 4) Update OLED
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Air Pollution Index");

  display.setTextSize(2);
  display.setCursor(0,16);
  display.print(airValue);

  display.setTextSize(1);
  display.setCursor(0,44);
  display.print("Status: ");
  display.println(statusText);

  if (showWarmup) {
    display.setCursor(0,56);
    display.print("Warming: ");
    display.print(WARMUP_SECONDS - secs);
    display.print("s");
  }
  display.display();

  // 5) Serial debug
  Serial.print("Air=");
  Serial.print(airValue);
  Serial.print("  Status=");
  Serial.print(statusText);
  if (showWarmup) {
    Serial.print(" (Warm-up ");
    Serial.print(WARMUP_SECONDS - secs);
    Serial.print("s)");
  }
  Serial.println();
}
