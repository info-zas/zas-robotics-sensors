#define TRIG_PIN A1
#define ECHO_PIN A2

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Send a 10 microsecond pulse to trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pin
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters
  float distance = duration * 0.034 / 2;

  // Print the distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  //delay(100);
  delay(1000); // Update every 0.5 seconds
}

