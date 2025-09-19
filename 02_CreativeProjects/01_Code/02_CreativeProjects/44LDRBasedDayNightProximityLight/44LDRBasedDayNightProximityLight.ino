// LDR-Based Day-Night Proximity Light
// Components: Ultrasonic Sensor, LDR, RGB LED

// Ultrasonic Sensor Pins
const int trigPin = 9;
const int echoPin = 10;

// LDR Pin
const int ldrPin = A0;

// RGB LED Pins
const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;

// Thresholds
const int lightThreshold = 500;  // Adjust based on environment (lower = darker)
const int distanceThreshold = 30; // cm (if person is closer than 30 cm, trigger LED)

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Initially turn off LED
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void loop() {
  // Read LDR value
  int ldrValue = analogRead(ldrPin);
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // Measure distance using Ultrasonic
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Control logic: Night + Proximity = Turn ON LED
  if (ldrValue < lightThreshold && distance < distanceThreshold) {
    // Nighttime and person detected nearby → Turn ON LED (white light)
    analogWrite(redPin, 255);
    analogWrite(greenPin, 255);
    analogWrite(bluePin, 255);
  } else {
    // Otherwise, turn OFF LED
    analogWrite(redPin, 0);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 0);
  }

  delay(500);
}
