/*
 * Project: Proximity-Based Hand Sanitizer Reminder
 * Components: Arduino Uno, HC-SR04 Ultrasonic Sensor, OLED Display, Buzzer
 * Function: Detects when someone comes close and reminds them to sanitize
 * Author: [Your Name]
 */

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic Sensor pins
const int trigPin = A1;
const int echoPin = A2;

// Buzzer pin
const int buzzer = 12;

// Variables
long duration;
int distance;

void setup() {
  // Initialize serial monitor
  Serial.begin(9600);

  // Initialize pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 25);
  display.println("System Ready...");
  display.display();
  delay(2000);
}

void loop() {
  // Trigger ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // Convert to cm

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance <= 30) { // If someone is within 30 cm
    // Activate buzzer
    digitalWrite(buzzer, HIGH);

    // Show message on OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("Please Sanitize");
    display.setCursor(0, 35);
    display.println("Your Hands!");
    display.display();
  } 
  else {
    // Turn off buzzer
    digitalWrite(buzzer, LOW);

    // Clear display
    display.clearDisplay();
    display.setCursor(20, 25);
    display.println("Waiting...");
    display.display();
  }

  delay(500);
}
