#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic pins
const int trigPin = A1;
const int echoPin = A2;

// Touch sensor pin
const int touchPin = 4;

void setup() {
  Serial.begin(9600);

  // Pin setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(touchPin, INPUT);

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
  // Measure distance from ultrasonic
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  // Read touch sensor
  int touchState = digitalRead(touchPin);

  // Clear display for new data
  display.clearDisplay();
  display.setCursor(0, 0);

  // If visitor is detected
  if (distance > 2 && distance < 150) {
    display.print("Visitor Detected!");
    display.setCursor(0, 20);
    display.print("Distance: ");
    display.print(distance);
    display.print(" cm");

    // If touch sensor is pressed
    if (touchState == HIGH) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Someone at the Door!");
    }
  } else {
    display.print("No Visitor");
  }

  display.display();

  // Debug info
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Touch: ");
  Serial.println(touchState);

  delay(500);
}
