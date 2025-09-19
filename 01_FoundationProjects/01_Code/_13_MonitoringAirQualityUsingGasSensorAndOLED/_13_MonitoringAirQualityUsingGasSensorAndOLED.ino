#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int mq135Pin = A0; // Pin connected to the MQ135 AOUT
float sensorValue = 0;

void setup() {
  Serial.begin(9600);
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check your I2C address (0x3C or 0x3D)
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Initialize the OLED display
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("MQ135 Air Quality");
  display.display();
  delay(2000); // Pause for 2 seconds
}

void loop() {
  // Read the analog value from the MQ135 sensor
  sensorValue = analogRead(mq135Pin);
  
  // Clear the display
  display.clearDisplay();
  
  // Display the sensor value
  display.setCursor(0, 0);
  display.print("Sensor Value: ");
  display.println(sensorValue);
  
  // Optionally, you can convert the sensor value to a more meaningful unit
  // For example, you can implement a conversion based on calibration data
  
  display.display(); // Update the display
  delay(1000); // Update every second
}
