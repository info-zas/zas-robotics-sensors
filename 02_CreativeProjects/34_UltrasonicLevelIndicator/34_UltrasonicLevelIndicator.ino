#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic Sensor pins
const int trigPin = A1;
const int echoPin = A2;

// Tank parameters (in cm) – adjust for your tank
const int tankHeight = 30;   // Total tank depth
const int minDistance = 2;   // Distance when tank is full (sensor to liquid)
const int maxDistance = 28;  // Distance when tank is empty

void setup() {
  Serial.begin(9600);

  // Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  // Measure distance
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // Convert to cm

  // Map distance to level %
  int levelPercent = map(distance, maxDistance, minDistance, 0, 100);
  if (levelPercent < 0) levelPercent = 0;
  if (levelPercent > 100) levelPercent = 100;

  // Print to Serial
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm  | Level: ");
  Serial.print(levelPercent);
  Serial.println(" %");

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Ultrasonic Level");
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(levelPercent);
  display.print("%");
  display.setCursor(0, 50);
  display.setTextSize(1);
  display.print("Dist: ");
  display.print(distance);
  display.print(" cm");

  display.display();

  delay(1000);
}
