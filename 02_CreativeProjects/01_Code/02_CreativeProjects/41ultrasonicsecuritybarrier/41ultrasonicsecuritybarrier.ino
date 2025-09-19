// Ultrasonic Security Barrier
// Components: Ultrasonic Sensor, Buzzer, RGB LED

// Ultrasonic pins
const int trigPin = A1;
const int echoPin = A2;

// Buzzer pin
const int buzzerPin = 12;

// RGB LED pins
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

// Variables
long duration;
int distance;
const int threshold = 30; // cm

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Turn off outputs initially
  digitalWrite(buzzerPin, LOW);
  setColor(0, 0, 0);
}

void loop() {
  // Measure distance using Ultrasonic
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

  // Security check
  if (distance > 0 && distance < threshold) {
    // Trigger alarm
    digitalWrite(buzzerPin, HIGH);

    // Flashing RGB warning
    setColor(255, 0, 0);  // Red
    delay(200);
    setColor(0, 0, 255);  // Blue
    delay(200);
  } else {
    // No alarm
    digitalWrite(buzzerPin, LOW);
    setColor(0, 255, 0); // Green = Safe
  }

  delay(100);
}

// ====== Helper Function ======
void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}
