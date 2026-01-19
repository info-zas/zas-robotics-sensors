#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2);       // Font size
  display.setTextColor(WHITE);  // Text color
  display.setCursor(10, 20);    // X,Y position
  display.println("Hello!");
  display.display();            // Show text
}

void loop() {
}
