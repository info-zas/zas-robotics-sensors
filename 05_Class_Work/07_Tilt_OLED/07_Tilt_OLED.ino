#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define tiltPin 4

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  pinMode(tiltPin, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  display.clearDisplay();
  display.setCursor(10, 20);

  if (digitalRead(tiltPin) == HIGH) {
    display.println("Tilted");
  } else {
    display.println("Not tilted");
  }

  display.display();
  delay(100);
}
