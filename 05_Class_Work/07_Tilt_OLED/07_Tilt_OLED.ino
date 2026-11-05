#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define tiltPin 4

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  pinMode(tiltPin, INPUT);
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  int t = digitalRead(tiltPin);
  Serial.println(t);

  display.clearDisplay();
  display.setCursor(10, 20);

  if (t == HIGH) {
    display.println("Tilted");
  } else {
    display.println("Not tilted");
  }

  display.display();
  delay(100);
}