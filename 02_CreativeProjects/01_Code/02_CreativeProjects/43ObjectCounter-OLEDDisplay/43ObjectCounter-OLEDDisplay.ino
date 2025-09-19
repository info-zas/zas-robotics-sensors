// Object Counter with OLED Display
// Components: Ultrasonic Sensor, OLED (SSD1306 I2C)

// Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic pins
const int trigPin = A1;
const int echoPin = A2;

// Variables
long duration;
int distance;
int count = 0;
const int threshold = 15; // cm (object closer than this is counted)
bool objectDetected = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Halt if display not found
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Counter");
  display.display();
  delay(1000);
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
  distance = duration * 0.034 / 2; // cm

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Check if object detected within threshold
  if (distance > 0 && distance < threshold) {
    if (!objectDetected) {
      count++;
      objectDetected = true;

      // Update display
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println("Count:");
      display.setTextSize(3);
      display.setCursor(0,30);
      display.println(count);
      display.display();
    }
  } else {
    objectDetected = false;
  }

  delay(200);
}
