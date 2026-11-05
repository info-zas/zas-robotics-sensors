#define tilt 4

void setup() {
  pinMode(tilt,INPUT);
  Serial.begin(9600);
}

void loop() {
  int t = digitalRead(tilt);
  Serial.println(t);
}
