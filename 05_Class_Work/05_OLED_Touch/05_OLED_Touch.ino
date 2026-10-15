#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define touchPin 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  pinMode(touchPin, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);  // Fits "Not touched" on one line
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);

  if (digitalRead(touchPin) == HIGH) {
    display.println("Touched");
  } else {
    display.println("Not touched");
  }

  display.display();
  delay(100);
}
