/*
  Weather Indicator with DHT11 + RGB/NeoPixel
  Inputs: DHT11 (D8), Touch Sensor (D7 optional)
  Outputs: RGB LED or NeoPixel (D6)

  Modes:
    - Temperature Mode (default)
      Blue = Cold, Green = Comfy, Red = Hot
    - Humidity Mode (toggle by touch)
      Yellow = Dry, Green = Comfy, Cyan = Humid
*/

#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// ------------ Pins ------------
const int PIN_DHT = 8;       // DHT11 Data pin
const int PIN_TOUCH = 4;     // Touch sensor
const int PIN_NEOPIXEL = 6;  // NeoPixel control pin

// ------------ DHT11 Setup ------------
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// ------------ NeoPixel Setup ------------
#define NUMPIXELS 8  // NeoPixel ring with 8 LEDs
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ------------ Variables ------------
bool humidityMode = false;  // false=Temperature, true=Humidity

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(PIN_TOUCH, INPUT);

  pixels.begin();
  pixels.clear();
  pixels.show();

  Serial.println("Weather Indicator Ready");
}

void loop() {
  // Read sensors
  float hum = dht.readHumidity();
  float tem = dht.readTemperature();

  // Check DHT11 valid
  if (isnan(hum) || isnan(tem)) {
    Serial.println("DHT11 ERROR");
    return;
  }

  // Toggle mode if touch detected
  if (digitalRead(PIN_TOUCH) == HIGH) {
    humidityMode = !humidityMode;
    delay(500); // debounce
  }

  pixels.clear();

  // ---------- Mode Logic ----------
  if (!humidityMode) {
    // Temperature Mode
    if (tem < 20) setAllPixels(0, 0, 255);      // Blue
    else if (tem <= 31) setAllPixels(0, 255, 0); // Green
    else setAllPixels(255, 0, 0);               // Red

    Serial.print("Temperature: "); Serial.print(tem); Serial.println(" *C");

  } else {
    // Humidity Mode
    if (hum < 30) setAllPixels(255, 255, 0);     // Yellow
    else if (hum <= 60) setAllPixels(0, 255, 0); // Green
    else setAllPixels(0, 255, 255);             // Cyan

    Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
  }

  pixels.show();
  delay(1000);
}

// ------------ Helper Function ------------
void setAllPixels(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
}
