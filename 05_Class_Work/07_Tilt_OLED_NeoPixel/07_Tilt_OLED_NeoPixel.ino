#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define tiltPin 4
#define ringPin 8

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel ring(8, ringPin, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(tiltPin, INPUT);
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  ring.begin();
  ring.setBrightness(30);
}

void loop() {
  int t = digitalRead(tiltPin);
  Serial.println(t);

  display.clearDisplay();
  display.setCursor(10, 20);

  if (t == HIGH) {
    display.println("Tilted");
    ring.fill(ring.Color(255, 0, 0));
  } else {
    display.println("Not tilted");
    ring.clear();
  }

  display.display();
  ring.show();
  delay(100);
}
