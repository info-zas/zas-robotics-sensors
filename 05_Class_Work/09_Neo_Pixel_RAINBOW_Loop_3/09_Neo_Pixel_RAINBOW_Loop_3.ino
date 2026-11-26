
#include <Adafruit_NeoPixel.h>

#define PIN 4         
#define NUMPIXELS 12  

int r = 100;
int g = 0;
int b = 0;
int j = 0;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); 
  pixels.setBrightness(100);
  pixels.show();  
}


void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i % 6 == 0) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));    // Red
    } else if (i % 6 == 1) {
      pixels.setPixelColor(i, pixels.Color(255, 100, 0));  // Orange
    } else if (i % 6 == 2) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0));  // Yellow
    } else if (i % 6 == 3) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0));    // Green
    } else if (i % 6 == 4) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255));    // Blue
    } else {
      pixels.setPixelColor(i, pixels.Color(128, 0, 128));  // Purple
    }
  }
pixels.show(); 
delay(1000);
for (int i = 0; i < NUMPIXELS; i++) {
  
    pixels.setPixelColor(i, pixels.Color(r, g, b));
    
  }
  pixels.show(); 
  delay(1000);
}

