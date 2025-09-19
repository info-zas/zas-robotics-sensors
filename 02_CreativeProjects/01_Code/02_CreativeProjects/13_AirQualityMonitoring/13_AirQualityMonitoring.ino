/*
  Air Quality Monitor with MQ-135 + OLED + NeoPixel
  Inputs: MQ-135 (A0)
  Outputs: OLED (I2C), NeoPixel Ring (D6)

  Functionality:
    - Reads air quality from MQ-135
    - Shows raw value on OLED
    - Lights NeoPixel in color bands:
        Green = Good
        Yellow = Moderate
        Red = Poor
        Purple = Very Poor
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// ------------ Pins ------------
const int PIN_MQ = A0;        // MQ-135 sensor analog pin
const int PIN_NEOPIXEL = 6;   // NeoPixel data pin

// ------------ Display Setup ------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------------ NeoPixel Setup ------------
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ------------ Thresholds (example ranges) ------------
const int AQ_GOOD_MAX      = 200;
const int AQ_MODERATE_MAX  = 400;
const int AQ_POOR_MAX      = 700;
// Above 700 = Very Poor

// ------------ Helper Function ------------
void setAllPixels(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void setup() {
  Serial.begin(9600);
  pixels.begin();
  pixels.clear();
  pixels.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed!");
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Air Quality Monitor");
  display.display();
  delay(1000);
}

void loop() {
  int airValue = analogRead(PIN_MQ);

  // 1) Categorize Air Quality
  String status;
  if (airValue < AQ_GOOD_MAX) {
    status = "Good";
    setAllPixels(0, 255, 0);    // Green
  } 
  else if (airValue < AQ_MODERATE_MAX) {
    status = "Moderate";
    setAllPixels(255, 150, 0);  // Yellow/Orange
  } 
  else if (airValue < AQ_POOR_MAX) {
    status = "Poor";
    setAllPixels(255, 0, 0);    // Red
  } 
  else {
    status = "Very Poor";
    setAllPixels(128, 0, 128);  // Purple
  }

  // 2) Show on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Air Quality Monitor");

  display.setCursor(0,16);
  display.print("Value: ");
  display.println(airValue);

  display.setCursor(0,32);
  display.print("Status: ");
  display.println(status);

  display.display();

  // 3) Debug Serial
  Serial.print("AirValue="); Serial.print(airValue);
  Serial.print("  Status="); Serial.println(status);

  delay(1000);
}
