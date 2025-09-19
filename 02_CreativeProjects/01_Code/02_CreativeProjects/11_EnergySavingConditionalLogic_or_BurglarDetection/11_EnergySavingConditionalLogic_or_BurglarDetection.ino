/*
  Smart Light Control
  Inputs: LDR (A0), IR Motion Sensor (D8)
  Outputs: RGB LED (D3, D4, D5), OLED Display (I2C)

  Functionality:
    - Light turns ON only when it’s dark AND motion is detected
    - OLED displays system status
    - RGB LED indicates the current state
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ------------ Pins ------------
const int PIN_LDR = A0;   // LDR sensor (analog)
const int PIN_IR  = 2;    // IR motion sensor (digital)
const int PIN_R   = 3;    // RED LED
const int PIN_G   = 4;    // GREEN LED
const int PIN_B   = 5;    // BLUE LED

// ------------ Display ------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------------ Thresholds (tune) ------------
const int LDR_THRESHOLD = 400;  // Below this → Dark
// Higher = more light sensitivity

// ------------ LED Helpers ------------
void ledOff()    { digitalWrite(PIN_R, LOW); digitalWrite(PIN_G, LOW); digitalWrite(PIN_B, LOW); }
void ledRed()    { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledGreen()  { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
void ledBlue()   { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, HIGH); }
void ledYellow() { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }

void setup() {
  pinMode(PIN_IR, INPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  ledOff();

  Serial.begin(9600);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Smart Light System");
  display.display();
  delay(700);
}

void loop() {
  int ldrValue = analogRead(PIN_LDR);
  int irValue  = digitalRead(PIN_IR);

  Serial.print("LDR="); Serial.print(ldrValue);
  Serial.print("  IR="); Serial.println(irValue);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Smart Light System");
  display.setCursor(0,14);

  if (ldrValue < LDR_THRESHOLD) {   // Dark
    if (irValue == HIGH) {          // Motion detected
      ledRed();
      display.println("Status: LIGHT ON");
    } else {                        // No motion
      ledYellow();
      display.println("Status: DARK + NO MOTION");
    }
  } else {                          // Bright
    ledGreen();
    display.println("Status: BRIGHT (OFF)");
  }

  display.display();
  delay(500);  // refresh rate
}
