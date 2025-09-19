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

// Pollution sensor pin
const int pollutionPin = A0;

// RGB LED pins
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

// Visitor detection threshold
const int visitorThreshold = 50; // cm

void setup() {
  Serial.begin(9600);

  // Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // RGB LED pins
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
  // Ultrasonic distance measurement
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  // Pollution sensor reading
  int pollutionValue = analogRead(pollutionPin);

  // Simple air quality mapping (adjust thresholds experimentally)
  String airStatus;
  if (pollutionValue < 200) {
    airStatus = "Good";
    setLED(0, 255, 0); // Green
  } else if (pollutionValue < 400) {
    airStatus = "Moderate";
    setLED(255, 255, 0); // Yellow
  } else {
    airStatus = "Poor";
    setLED(255, 0, 0); // Red
  }

  // Display info on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  if (distance < visitorThreshold) {
    display.print("Visitor Detected!");
    display.setCursor(0, 20);
    display.print("Air: ");
    display.print(airStatus);
    display.setCursor(0, 40);
    display.print("Level: ");
    display.print(pollutionValue);
  } else {
    display.print("No Visitor");
  }

  display.display();

  // Debug
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Pollution: ");
  Serial.println(pollutionValue);

  delay(1000);
}

// Function to control RGB LED
void setLED(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}
