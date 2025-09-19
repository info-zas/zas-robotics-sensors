#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display width and height
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The I2C address for most SSD1306 OLEDs is 0x3C or 0x3D
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // Initialize OLED display with I2C address 0x3C
  // You might need to change 0x3C to 0x3D depending on your display.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display buffer.
  display.clearDisplay();

  // Set text size (1 is smallest)
  display.setTextSize(1);
  // Set text color (WHITE is default for monochrome OLEDs)
  display.setTextColor(SSD1306_WHITE);
  // Set cursor position (x, y)
  display.setCursor(0, 0);
  // Print text
  display.println("Hello, Arduino!");

  // Display everything on the screen
  display.display();

  delay(2000); // Pause for 2 seconds

  // Clear display and show some more text
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("ZAS");
  display.println("Robotics!");
  display.display();
}

void loop() {
  // Nothing to do in the loop for this simple example
}