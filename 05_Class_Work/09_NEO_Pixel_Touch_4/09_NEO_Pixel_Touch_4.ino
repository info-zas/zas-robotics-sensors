#include <Adafruit_NeoPixel.h>

#define PIN 8
#define NUMPIXELS 8

Adafruit_NeoPixel pixels(NUMPIXELS,PIN,NEO_GRB+NEO_KHZ800);

void setup() {
  pinMode(4,INPUT); //touch sensor
  pixels.begin();
  pixels.setBrightness(50);
  pixels.show();
  Serial.begin(9600);
}

void loop() {

  int touch = digitalRead(4);
  Serial.println(touch);

  if(touch == 1){
        for (int i = 0; i<NUMPIXELS; i = i+1){
        pixels.setPixelColor(i,pixels.Color(0,255,255)); //cyan
      }
      pixels.show();
      delay(100);
  }


else{
      for (int i = 0; i<NUMPIXELS; i = i+1){
      pixels.setPixelColor(i,pixels.Color(255,0,0)); //red
    }
    pixels.show();
    delay(100);
}

}
