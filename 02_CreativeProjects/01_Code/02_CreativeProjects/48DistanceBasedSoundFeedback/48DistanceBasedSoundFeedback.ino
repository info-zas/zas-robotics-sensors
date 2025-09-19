#define TRIG_PIN A1
#define ECHO_PIN A2
#define BUZZER_PIN 12
#define RED_PIN 5
#define GREEN_PIN 6
#define YELLOW_PIN 7

// Target distance in cm
const int targetDistance = 20;
const int tolerance = 3; // +/- range allowed

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);

  Serial.begin(9600);
}

void setColor(int r, int g, int b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(YELLOW_PIN, b);
}

void playTone(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}

void loop() {
  long distance = readDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance >= (targetDistance - tolerance) && distance <= (targetDistance + tolerance)) {
    // Correct distance
    setColor(0, 255, 0);   // Green
    playTone(1000, 200);   // High tone
  } 
  else if (distance < (targetDistance - tolerance)) {
    // Too close
    setColor(0, 0, 255);   // Blue
    playTone(500, 200);    // Low tone
  } 
  else if (distance > (targetDistance + tolerance)) {
    // Too far
    setColor(255, 0, 0);   // Red
    playTone(300, 200);    // Lower tone
  }

  delay(300);
}
