#include <Adafruit_NeoPixel.h>

#define TRIG_PIN A1
#define ECHO_PIN A2
#define TOUCH_PIN 4
#define NEOPIXEL_PIN 6
#define NUM_PIXELS 8

Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

long duration;
int distance;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT);

  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2; // Convert to cm
}

void rainbowCycle(uint8_t wait) {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
    }
    pixels.show();
    delay(wait);
  }
}

void touchPattern() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 255)); // Purple
    pixels.show();
    delay(50);
  }
  delay(300);
  pixels.clear();
  pixels.show();
}

void distancePattern(int d) {
  int brightness = map(d, 5, 50, 255, 10);
  brightness = constrain(brightness, 10, 255);
  
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, brightness, brightness)); // Cyan gradient
  }
  pixels.show();
}

void loop() {
  int touchValue = digitalRead(TOUCH_PIN);
  int distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Touch: ");
  Serial.println(touchValue);

  if (touchValue == HIGH) {
    touchPattern(); // Touch triggers color flash
  } else if (distance <= 30) {
    distancePattern(distance); // Distance controls color brightness
  } else {
    rainbowCycle(10); // Idle animation
  }

  delay(100);
}
