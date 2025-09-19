#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions
const int IR = 8;


const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

const int SERVO_PIN = 3;

const int MOTOR_LEFT_FORWARD = 7;
const int MOTOR_LEFT_BACKWARD = 8;
const int MOTOR_RIGHT_FORWARD = 9;
const int MOTOR_RIGHT_BACKWARD = 10;

const int ENA = 11;
const int ENB = 12;

const int RGB_RED = 9;
const int RGB_GREEN = A0;
const int RGB_BLUE = A1;

// OLED Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo Object
Servo scanServo;

void setup() {
  // Motor pins
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Sensor pins
  pinMode(IR, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // RGB LED
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Servo
  scanServo.attach(SERVO_PIN);
  scanServo.write(90); // Center position

  Serial.begin(9600);
}

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void setRGB(int r, int g, int b) {
  analogWrite(RGB_RED, r);
  analogWrite(RGB_GREEN, g);
  analogWrite(RGB_BLUE, b);
}

void moveForward() {
  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void stopMoving() {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnLeft() {
  digitalWrite(MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnRight() {
  digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
  digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
  digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

int scanForClearPath() {
  setRGB(0, 0, 255); // Blue during scanning
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Scanning...");

  scanServo.write(45);
  delay(500);
  long leftDistance = readDistance();
  display.print("Left: "); display.println(leftDistance);

  scanServo.write(135);
  delay(500);
  long rightDistance = readDistance();
  display.print("Right: "); display.println(rightDistance);

  scanServo.write(90); // Reset to center
  display.display();

  if (leftDistance > rightDistance)
    return 0; // Left
  else
    return 1; // Right
}

void loop() {
  int ir = digitalRead(IR);
    long distance = readDistance();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Dist: "); display.print(distance); display.println(" cm");
  display.print("IR "); display.print(ir);
  
  display.display();

  if (distance < 15) {
    stopMoving();
    setRGB(255, 0, 0); // Red = obstacle
    int direction = scanForClearPath();

    if (direction == 0) {
      turnLeft();
      delay(700);
    } else {
      turnRight();
      delay(700);
    }

    stopMoving();
    delay(200);
  } else {
    setRGB(0, 255, 0); // Green = clear

    if (ir == LOW ) {
      moveForward();
    }  else if (ir == HIGH) {
      turnRight();
    } else {
      stopMoving();
    }
  }

  delay(100);
}
