// Proximity-Based Lighting
// Ultrasonic Sensor + RGB LED
// Smart lighting system demo

// Pins
const int trigPin = A1;
const int echoPin = A2;
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

void setup() {
  Serial.begin(9600);

  // Pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

// Function to measure distance
long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2; // cm
  return distance;
}

// Function to set RGB LED color
void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void loop() {
  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance < 10) {
    // Very close → Red, max brightness
    setColor(255, 0, 0);
  } else if (distance < 20) {
    // Medium range → Green
    setColor(0, 255, 0);
  } else if (distance < 40) {
    // Farther → Blue
    setColor(0, 0, 255);
  } else {
    // No presence → LED off
    setColor(0, 0, 0);
  }

  delay(200);
}
