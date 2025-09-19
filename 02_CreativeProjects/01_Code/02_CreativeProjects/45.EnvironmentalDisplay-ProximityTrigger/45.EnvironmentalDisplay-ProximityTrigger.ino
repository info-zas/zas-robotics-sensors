/*
 * Project: Environmental Display with Proximity Trigger
 * Components: Arduino Uno, HC-SR04, DHT11, MQ135, OLED Display
 * Function: Displays temperature, humidity, and air quality only when someone is near
 * Author: [Your Name]
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <DHT.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic Sensor
const int trigPin = A1;
const int echoPin = A2;

// DHT11 Sensor
#define DHTPIN 8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Pollution Sensor
const int mq135Pin = A0;

// Variables
long duration;
int distance;
float temperature, humidity;
int airQuality;

void setup() {
  Serial.begin(9600);

  // Sensor setup
  dht.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println("System Ready...");
  display.display();
  delay(2000);
}

void loop() {
  // --- Ultrasonic distance measurement ---
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // cm

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- Check if person is nearby (within 50 cm) ---
  if (distance > 0 && distance <= 50) {
    // Read environmental sensors
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    airQuality = analogRead(mq135Pin);

    // Display data on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Env. Data:");

    display.setCursor(0, 15);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.setCursor(0, 30);
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    display.setCursor(0, 45);
    display.print("Air Q: ");
    display.print(airQuality);
    display.println(" ppm");

    display.display();
  } else {
    // Turn off / idle display
    display.clearDisplay();
    display.setCursor(20, 25);
    display.println("Idle Mode...");
    display.display();
  }

  delay(1000); // update every second
}
