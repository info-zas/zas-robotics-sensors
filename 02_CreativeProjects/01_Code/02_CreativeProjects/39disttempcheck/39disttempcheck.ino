// Distance + Temperature Checker
// Components: Ultrasonic Sensor (HC-SR04), DHT11, OLED (I2C)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic pins
#define TRIG_PIN A1
#define ECHO_PIN A2

// DHT11 setup
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

long duration;
int distance;

void setup() {
  Serial.begin(9600);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found!"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Distance + Temp Checker");
  display.display();
  delay(2000);
}

void loop() {
  // Get distance
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; // cm

  // Get temperature from DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Update OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,5);
  display.println("Contactless Health Check");

  display.setTextSize(1);
  display.setCursor(0,30);
  display.print("Dist: ");
  display.print(distance);
  display.println("cm");

  display.setCursor(0,40);
  display.print("Temp: ");
  display.print(temperature);
  display.print(" C");

  display.display();

  // Debug
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Temp: ");
  Serial.print(temperature);
  Serial.println(" C");

  delay(1000);
}
