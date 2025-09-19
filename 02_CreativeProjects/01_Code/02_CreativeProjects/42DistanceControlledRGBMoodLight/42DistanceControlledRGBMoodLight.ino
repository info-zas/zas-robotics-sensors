// Distance-Controlled RGB Mood Light
// Components: Ultrasonic Sensor, Touch Sensor, RGB LED

// Ultrasonic pins
const int trigPin = A1;
const int echoPin = A2;

// Touch Sensor pin
const int touchPin = 4;

// RGB LED pins
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 7;

// Variables
long duration;
int distance;
int mode = 0; // 0 = Calm, 1 = Alert, 2 = Rainbow
bool lastTouchState = LOW;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(touchPin, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Turn off LED initially
  setColor(0, 0, 0);
}

void loop() {
  // Read touch input
  bool touchState = digitalRead(touchPin);
  if (touchState == HIGH && lastTouchState == LOW) {
    mode = (mode + 1) % 3; // Cycle through modes
    Serial.print("Mode changed to: ");
    Serial.println(mode);
    delay(300); // debounce
  }
  lastTouchState = touchState;

  // Measure distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // cm

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Apply behavior based on mode
  if (mode == 0) {
    calmMode(distance);
  } else if (mode == 1) {
    alertMode(distance);
  } else if (mode == 2) {
    rainbowMode();
  }

  delay(100);
}

// ====== Modes ======

// Calm Mode: Soft colors based on distance
void calmMode(int dist) {
  if (dist < 10) setColor(50, 0, 100);   // Purple (close)
  else if (dist < 20) setColor(0, 50, 100); // Cyan (medium)
  else if (dist < 30) setColor(0, 100, 50); // Greenish (far)
  else setColor(0, 0, 0); // Off
}

// Alert Mode: Strong colors based on distance
void alertMode(int dist) {
  if (dist < 10) setColor(255, 0, 0);   // Red (danger)
  else if (dist < 20) setColor(255, 150, 0); // Orange (warning)
  else if (dist < 30) setColor(255, 255, 0); // Yellow (safe)
  else setColor(0, 0, 0); // Off
}

// Rainbow Mode: Cycles through colors
void rainbowMode() {
  for (int r = 0; r < 256; r++) {
    setColor(r, 255 - r, (r * 2) % 255);
    delay(10);
  }
}

// ====== Helper Function ======
void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}
