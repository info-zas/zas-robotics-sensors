/*
  Automatic Parking Assistant
  Ultrasonic Sensor + OLED + RGB LED
  Author: Your Name
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pins for Ultrasonic
const int trigPin = A1;
const int echoPin = A2;

// Pins for RGB LED
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Parking Assistant");
  display.display();
  delay(1000);
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2; // cm
  return distance;
}

void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void loop() {
  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Display distance on OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print("Dist: ");
  display.print(distance);
  display.println(" cm");
  display.display();

  // Set LED color based on distance
  if (distance > 30) {
    // Safe → Green
    setColor(0, 255, 0);
  } else if (distance > 10 && distance <= 30) {
    // Caution → Yellow
    setColor(0, 0, 255);
  } else {
    // Danger → Red
    setColor(255, 0, 0);
  }

  delay(200);
}
