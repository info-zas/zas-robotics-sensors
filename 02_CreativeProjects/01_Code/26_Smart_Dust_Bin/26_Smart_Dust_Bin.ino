/*
  Smart Dustbin (demo-friendly)
  Ultrasonic -> Servo with hysteresis + hold + robust ranging

  Pins (as you used):
    HC-SR04: TRIG=A1, ECHO=A2   (UNO analog pins work as digital I/O)
    Servo  : D3
*/

#include <Servo.h>

// -------- Pins --------
const uint8_t PIN_TRIG  = A1;
const uint8_t PIN_ECHO  = A2;
const uint8_t PIN_SERVO = 3;

// -------- Demo Tuning --------
const int  OPEN_CM   = 1;        // <= open
const int  CLOSE_CM  = 10;        // >= eligible to close
const unsigned long HOLD_MS = 500; // stay open at least this long
const unsigned long MEAS_PERIOD_MS = 90;  // ~11 Hz updates

// Valid/usable range for demo (filters crazy readings)
const int MIN_CM = 4;             // HC-SR04 unreliable below ~2–3 cm
const int MAX_CM = 150;           // plenty for a bin

// Servo angles (adjust to your hinge)
const int ANGLE_CLOSED = 5;       // small offset avoids stalling against lid
const int ANGLE_OPEN   = 95;

// -------- Internals --------
Servo lid;
unsigned long lastMeasure = 0;
unsigned long lastSeen    = 0;

enum LidState { LID_CLOSED, LID_OPEN };
LidState state = LID_CLOSED;

// ---------- Ranging helpers ----------
unsigned long echoOnceUS() {
  // 10 µs trigger
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(4);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Clean up any stale level then measure HIGH pulse
  pulseIn(PIN_ECHO, HIGH, 1000UL);
  pulseIn(PIN_ECHO, LOW,  1000UL);
  return pulseIn(PIN_ECHO, HIGH, 40000UL);   // up to ~6.8 m
}

int readDistanceCm() {
  // median of 5 for robustness
  int cm[5]; uint8_t m = 0;
  for (uint8_t i=0; i<5; i++) {
    unsigned long us = echoOnceUS();
    if (us > 0) {
      int d = (int)(us / 58.0);             // convert to cm
      if (d >= MIN_CM && d <= MAX_CM) cm[m++] = d;
    }
    delay(5);
  }
  if (m == 0) return 999;
  // insertion sort (m <= 5)
  for (uint8_t i=1; i<m; i++) {
    int key = cm[i], j = i - 1;
    while (j >= 0 && cm[j] > key) { cm[j+1] = cm[j]; j--; }
    cm[j+1] = key;
  }
  return cm[m/2];  // median
}

// ---------- Servo helpers ----------
void moveEase(int fromA, int toA, int step = 3, int ms = 6) {
  if (toA > fromA) {
    for (int a = fromA; a <= toA; a += step) { lid.write(a); delay(ms); }
  } else {
    for (int a = fromA; a >= toA; a -= step) { lid.write(a); delay(ms); }
  }
  lid.write(toA);
}

void goOpen() {
  moveEase(ANGLE_CLOSED, ANGLE_OPEN);
  state = LID_OPEN;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(F("STATE -> OPEN"));
}

void goClosed() {
  moveEase(ANGLE_OPEN, ANGLE_CLOSED);
  state = LID_CLOSED;
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("STATE -> CLOSED"));
}

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  lid.attach(PIN_SERVO);
  lid.write(ANGLE_CLOSED);

  Serial.begin(9600);
  delay(300);
  Serial.println(F("Smart Dustbin (demo) starting..."));

  // quick sweep to show it’s alive
  moveEase(ANGLE_CLOSED, ANGLE_OPEN, 4, 5);
  moveEase(ANGLE_OPEN, ANGLE_CLOSED, 4, 5);
}

void loop() {
  if (millis() - lastMeasure < MEAS_PERIOD_MS) return;
  lastMeasure = millis();

  int dist = readDistanceCm();

  Serial.print(F("Distance(cm)=")); Serial.print(dist);
  Serial.print(F("  State=")); Serial.println(state == LID_OPEN ? "OPEN" : "CLOSED");

  // --- State machine with hysteresis + hold ---
  if (state == LID_CLOSED) {
    if (dist != 999 && dist <= OPEN_CM) {
      lastSeen = millis();
      goOpen();
    }
  } else { // LID_OPEN
    if (dist != 999 && dist <= OPEN_CM) {
      lastSeen = millis();   // refresh hold while something is near
    }
    bool holdOver   = (millis() - lastSeen) >= HOLD_MS;
    bool farEnough  = (dist == 999) || (dist >= CLOSE_CM);
    if (holdOver && farEnough) {
      goClosed();
    }
  }
}
