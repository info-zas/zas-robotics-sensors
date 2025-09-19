/*
  Smart Shelf Alert System
  Components: Ultrasonic Sensor, RGB LED, OLED Display
  Author: [Your Name]
  Date: [Date]
  Description: Detects missing items or person proximity using distance thresholds.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic sensor pins
#define trigPin A1
#define echoPin A2

// RGB LED pins
#define redPin 9
#define greenPin 10
#define bluePin 7

long duration;
int distance;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Smart Shelf System");
  display.display();
  delay(2000);
}

// Function to set RGB LED color
void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

// Function to measure distance
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // convert to cm
  return distance;
}

void loop() {
  distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  display.clearDisplay();
  display.setCursor(0,0);

  // Thresholds (example values, adjust as needed)
  if (distance > 30) {
    // Shelf item missing
    setColor(255, 0, 0); // Red LED
    display.println("Alert: Item Missing!");
  }
  else if (distance < 10) {
    // Person too close
    setColor(0, 0, 255); // Yellow LED
    display.println("Warning: Too Close!");
  }
  else {
    // Normal condition
    setColor(0, 255, 0); // Green LED
    display.println("Shelf OK");
  }

  display.display();
  delay(500);
}
