/*
  Interactive Distance-Based Game
  Components: Ultrasonic Sensor, Touch Sensor, NeoPixel Ring, OLED
  Author: [Your Name]
  Date: [Date]
  Description: A game where players move their hand to match a target distance.
*/

#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -------------------- OLED Setup --------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------------------- NeoPixel Setup --------------------
#define PIN_NEOPIXEL 7
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// -------------------- Sensor Pins --------------------
#define trigPin A1
#define echoPin A2
#define touchPin 4

// -------------------- Variables --------------------
long duration;
int distance;
int targetDistance;
int score = 0;
bool gameActive = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(touchPin, INPUT);

  // Initialize NeoPixel
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Distance Game Ready");
  display.display();
  delay(2000);
}

// Measure distance from ultrasonic sensor
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // cm
  return distance;
}

// Set NeoPixel color pattern
void setColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void loop() {
  int touchState = digitalRead(touchPin);

  if (touchState == HIGH && !gameActive) {
    // Start new game
    gameActive = true;
    score = 0;
    targetDistance = random(10, 40); // target between 10–40 cm
    setColor(0, 0, 255); // Blue = new round
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Game Started!");
    display.println("Target Distance:");
    display.print(targetDistance);
    display.println(" cm");
    display.display();
    delay(1000);
  }

  if (gameActive) {
    distance = getDistance();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Target: ");
    display.print(targetDistance);
    display.println(" cm");

    display.print("Your: ");
    display.print(distance);
    display.println(" cm");

    display.print("Score: ");
    display.println(score);

    if (abs(distance - targetDistance) <= 3) {
      // Success
      setColor(0, 255, 0); // Green
      display.println("Hit!");
      score++;
      delay(1500);

      // Generate new target
      targetDistance = random(10, 40);
      setColor(0, 0, 255); // Blue = new round
    } else {
      setColor(255, 0, 0); // Red when not matching
    }

    display.display();
  }
}
