
#include <Adafruit_NeoPixel.h>

#define PIN 8         
#define NUMPIXELS 8  

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); 
  pixels.setBrightness(100);
  pixels.show();  
}


void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i % 3 == 0) {
      pixels.setPixelColor(i, pixels.Color(100, 0, 0));  // Red
    } else if (i % 3 == 1) {
      pixels.setPixelColor(i, pixels.Color(0, 100, 0));  // Green
    } else {
      pixels.setPixelColor(i, pixels.Color(0, 0, 100));  // Blue
    }
  }
  pixels.show(); 
  delay(1000); 
}