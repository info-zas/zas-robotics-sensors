// Safe Entry Counter System (Single IR + Ultrasonic)
// Components: Ultrasonic Sensor, 1x IR Sensor, OLED Display

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor pins
const int irSensorPin = 8;     // Single IR sensor
const int trigPin = A1;         // Ultrasonic trig
const int echoPin = A2;        // Ultrasonic echo

long duration;
int distance;
int prevDistance = 0;
int peopleCount = 0;
const int maxDistance = 120;   // cm detection zone
const int minDistance = 10;    // ignore very close noise

void setup() {
  Serial.begin(9600);

  pinMode(irSensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found!"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Safe Entry Counter");
  display.display();
  delay(2000);
}

void loop() {
  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // in cm

  // Check IR sensor trigger
  if (digitalRead(irSensorPin) == LOW && distance > minDistance && distance < maxDistance) {
    if (prevDistance == 0) {
      prevDistance = distance;  // store first detection distance
    } else {
      // Compare new distance to previous
      if (distance < prevDistance) {
        peopleCount++; // Person moving closer = entering
        Serial.println("Person Entered");
      } else if (distance > prevDistance) {
        if (peopleCount > 0) {
          peopleCount--; // Person moving away = exiting
          Serial.println("Person Exited");
        }
      }
      prevDistance = 0; // reset after decision
      updateDisplay();
      delay(800); // debounce
    }
  }
}

// Function to update OLED
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Safe Entry Counter");

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("Count: ");
  display.println(peopleCount);

  display.display();
}
