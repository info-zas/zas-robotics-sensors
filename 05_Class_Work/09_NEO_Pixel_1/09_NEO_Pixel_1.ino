#include <Adafruit_NeoPixel.h>

#define PIN 8
#define NUMPIXELS 8

Adafruit_NeoPixel pixels(NUMPIXELS,PIN,NEO_GRB+NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.setBrightness(100);
  pixels.show();
}

void loop() {

  pixels.setPixelColor(0,pixels.Color(0,0,255));

  pixels.show();
  delay(100);
}

