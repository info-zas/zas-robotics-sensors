#include <Adafruit_NeoPixel.h>

#define PIN 8 // Pin connected to the NeoPixel ring
#define NUMPIXELS 8 // Number of pixels in the NeoPixel ring

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); // Initialize the NeoPixel ring
}

void loop() {
  // Cycle through colors
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Red
    pixels.show();
    delay(500);
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green
    pixels.show();
    delay(500);
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // Blue
    pixels.show();
    delay(500);
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Off
  }

  // Rainbow effect
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    delay(50);
  }
}

// Function to generate rainbow colors across 0-255 positions
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
