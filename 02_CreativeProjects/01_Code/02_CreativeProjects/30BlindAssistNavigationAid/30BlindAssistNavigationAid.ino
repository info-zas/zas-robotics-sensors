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

// Buzzer pin
const int buzzerPin = 12;

void setup() {
  Serial.begin(9600);

  // Pin setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

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

  // Display distance on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Distance: ");
  display.print(distance);
  display.println(" cm");
  display.display();

  // Buzzer feedback
  if (distance > 2 && distance <= 30) {
    // Very close → fast beeping
    tone(buzzerPin, 1000);
    delay(100);
    noTone(buzzerPin);
    delay(100);
  } else if (distance > 30 && distance <= 100) {
    // Medium distance → slower beeping
    tone(buzzerPin, 800);
    delay(300);
    noTone(buzzerPin);
    delay(300);
  } else if (distance > 100 && distance <= 200) {
    // Far but detected → very slow beeping
    tone(buzzerPin, 600);
    delay(600);
    noTone(buzzerPin);
    delay(600);
  } else {
    // No obstacle nearby → no sound
    noTone(buzzerPin);
  }

  // Debug info
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}
