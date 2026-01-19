#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_Sensor.h> // Required for DHT library

// DHT sensor definitions
#define DHTPIN 8     // DHT sensor connected to digital pin 8
#define DHTTYPE DHT11 // Type of DHT sensor (DHT11 or DHT22)

// OLED display definitions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The I2C address for most SSD1306 OLEDs is 0x3C or 0x3D
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT11 & OLED Test"));

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check your I2C address (0x3C or 0x3D)
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Initializing...");
  display.display();
  delay(1000);

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Read temperature and humidity.
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature(false);
  // Read temperature as Fahrenheit (is Fahrenheit is true)
  // float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Failed to read");
    display.println("DHT sensor!");
    display.display();
    return;
  }

  // Compute heat index in Celsius (or Fahrenheit)
  // float hic = dht.computeHeatIndex(t, h, false);
  // float hif = dht.computeHeatIndex(f, h);

  Serial.print(F("Humidity:"));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));

  // Clear display before writing new values
  display.clearDisplay();

  // Display Humidity
  display.setTextSize(2); // Larger text for main values
  display.setCursor(0,0);
  display.print("Hum:");
  display.print(h);
  display.println("%");

  // Display Temperature
  display.setCursor(0, 32); // Move cursor down
  display.print("Temp:");
  display.print(t);
  display.println((char)247); // Degree symbol
  display.println("C");

  // Update the OLED display
  display.display();
}