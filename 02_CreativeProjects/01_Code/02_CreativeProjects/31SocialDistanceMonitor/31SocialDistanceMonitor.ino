#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic pins
const int trigPin = A1;
const int echoPin = A2;

// RGB LED pins
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

void setup() {
  Serial.begin(9600);

  // Pin setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

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
  // Trigger ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure echo
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2; // cm

  // Clear OLED for new output
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance);
  display.println(" cm");

  // Set LED color based on distance
  if (distance > 100) {   // Safe
    setColor(0, 255, 0);  // Green
    display.println("Status: SAFE");
  } else if (distance > 60 && distance <= 100) { // Warning
    setColor(0, 0, 255); // Yellow
    display.println("Status: WARNING");
  } else if (distance <= 60 && distance > 2) { // Too close
    setColor(255, 0, 0);  // Red
    display.println("Status: TOO CLOSE!");
  } else {
    setColor(0, 0, 0); // Off if no detection
    display.println("No Object");
  }

  display.display();

  // Debug info
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  delay(500);
}

// Function to set RGB LED color
void setColor(int redVal, int greenVal, int blueVal) {
  analogWrite(redPin, redVal);
  analogWrite(greenPin, greenVal);
  analogWrite(bluePin, blueVal);
}
