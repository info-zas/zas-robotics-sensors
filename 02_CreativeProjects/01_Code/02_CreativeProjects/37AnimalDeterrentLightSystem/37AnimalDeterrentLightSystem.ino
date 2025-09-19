/*
  Animal Deterrent Light System
  Components: Ultrasonic Sensor, IR Sensor, RGB LED
  Author: [Your Name]
  Date: [Date]
  Description: Detects animals using IR + Ultrasonic sensors and activates RGB LED light patterns to deter them.
*/

#define trigPin A1
#define echoPin A2
#define pirPin 8

#define redPin 9
#define greenPin 10
#define yellowpin 7

long duration;
int distance;
int motionDetected;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowpin, OUTPUT);

  Serial.println("Animal Deterrent Light System Initialized");
}

// Function to set RGB LED color
void setColor(int red, int green, int yellow) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(yellowpin, yellow);
}

// Function to measure distance
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // convert to cm
  return distance;
}

void loop() {
  distance = getDistance();
  motionDetected = digitalRead(pirPin);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Motion: ");
  Serial.println(motionDetected);

  // If animal is detected
  if (motionDetected == HIGH && distance < 150) { // within 1.5m
    // Flash RGB patterns
    setColor(255, 0, 0); delay(200);  // Red
    setColor(0, 255, 0); delay(200);  // Green
    setColor(0, 0, 255); delay(200);  // Blue
    setColor(255, 255, 0); delay(200); // Yellow
    setColor(0, 0, 0); delay(200);    // Off (blink effect)
  } 
  else {
    setColor(0, 0, 0); // LED Off when no animal
  }

  delay(100);
}
