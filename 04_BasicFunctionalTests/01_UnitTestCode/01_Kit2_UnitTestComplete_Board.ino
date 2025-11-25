#include <Servo.h>
#include <Stepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>


#define BUTTON_PIN A2
#define BUZZER_PIN A1


// ========== OLED ==========
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ========== Servo ==========
Servo myServo;
#define SERVO_PIN 11
#define POT_PIN A0

// ========== Stepper ==========
#define STEPS 32
Stepper myStepper(STEPS, 8, 10, 9, 7);
volatile bool rotateCW = false, rotateCCW = false;

// ========== Rotary Encoder ==========
#define ENCODER_CLK 2
#define ENCODER_DT A3

volatile int encoderCount = 0;

//String encoderDir = "";

// ========== DC Motor with Encoder ==========
#define PWMA 5
#define AIN1 6
#define AIN2 4
#define ENCA 3
volatile long ticks = 0;

// ========== GPS ==========
TinyGPSPlus gps;

// ========== MPU6050 ==========
#define MPU_ADDR 0x68
int16_t ax, ay, az;

// ========== Setup ==========
void setup() {
  // Serial for GPS
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Internal pull-up resistor
  pinMode(BUZZER_PIN, OUTPUT);


  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Servo
  myServo.attach(SERVO_PIN);

  // Stepper
  myStepper.setSpeed(500);  // Adjust speed as needed

  // Rotary Encoder
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);

  // DC Motor
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(ENCA, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), countTicks, RISING);

  // MPU6050 init
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

// ========== Loop ==========
void loop() {
  // (a) Potentiometer controls Servo
  int pot = analogRead(POT_PIN);
  myServo.write(map(pot, 0, 1023, 0, 180));

  // (b) Rotary Encoder controls Stepper
  if (rotateCW) {
    myStepper.step(10); rotateCW = false;
  } else if (rotateCCW) {
    myStepper.step(-10); rotateCCW = false;
  }

  // (c) MPU6050 Raw Data
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();

  // (d) GPS
  while (Serial.available()) gps.encode(Serial.read());

  // (e) Run DC Motor until 10,000 ticks
  if (ticks < 5000) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 180);
  } else {
    analogWrite(PWMA, 0);  // Stop motor
  }

  // OLED Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Ax:"); display.print(ax);
  display.print(" Ay:"); display.print(ay);
  display.print(" Az:"); display.println(az);

  display.print("Enc: "); 
 // display.print(encoderDir);
  display.print(" ");
  display.println(encoderCount);


  // Add encoder pulse count
  display.print("Ticks: "); display.println(ticks);  // <-- ADDED LINE

  if (gps.location.isValid()) {
    display.print("Lat:"); display.println(gps.location.lat(), 4);
    display.print("Lon:"); display.println(gps.location.lng(), 4);
  }

  if (gps.date.isValid()) {
    display.print("Date:"); display.print(gps.date.day()); display.print("/");
    display.print(gps.date.month()); display.print("/");
    display.println(gps.date.year());
  }

  if (gps.time.isValid()) {
    display.print("Time:");
    display.print(gps.time.hour()); display.print(":");
    display.print(gps.time.minute()); display.print(":");
    display.println(gps.time.second());
  }

  
  // (f) - Play buzzer on button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200); // short buzz
    digitalWrite(BUZZER_PIN, LOW);
    delay(200); // debounce delay
  }
  display.display();
  delay(200);
}

// ========== Interrupts ==========
void readEncoder() {
  if (digitalRead(ENCODER_DT) != digitalRead(ENCODER_CLK)) {
    encoderCount++;
    rotateCW = true;
    //encoderDir = "CW";
  } else {
    encoderCount--;
    rotateCCW = true;
    //encoderDir = "CCW";
  }
}


void countTicks() {
  ticks++;
}
