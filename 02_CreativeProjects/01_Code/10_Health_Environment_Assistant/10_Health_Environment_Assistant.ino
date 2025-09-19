/*
  Health Environment Assistant
  DHT11 (D8) + MQ-135 (A0) + OLED (I2C) + RGB LED (D3/D4/D5)

  Shows: Temperature (°C), Humidity (%), Air value (0..1023), and overall status.
  LED Colors:
    - GREEN  = Air Good AND Temp/Humidity Comfy
    - YELLOW = Air Good/Moderate but Temp/Humidity needs adjustment
    - RED    = Poor Air (overrides everything)
    - OFF    = Sensor error
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ------------ Pins ------------
const int PIN_DHT = 8;   // DHT11 data pin
const int PIN_MQ  = A0;  // MQ-135 analog output
const int PIN_R   = 3;   // RED LED
const int PIN_G   = 4;   // GREEN LED
const int PIN_Y   = 5;   // YELLOW LED

// ------------ Display ------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------------ Sensors ------------
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// ------------ Thresholds (tune if needed) ------------
const float COMFY_TEMP_MIN = 20.0;   // °C
const float COMFY_TEMP_MAX = 26.0;   // °C
const float COMFY_HUM_MIN  = 30.0;   // %
const float COMFY_HUM_MAX  = 60.0;   // %

const int AIR_GOOD_MAX      = 300;   // MQ-135 raw ADC
const int AIR_MODERATE_MAX  = 600;   // >600 -> POOR

const byte MQ_AVG_SAMPLES   = 16;    // smoothing
const unsigned long READ_PERIOD_MS = 1000;
const unsigned long MQ_WARMUP_SECONDS = 30;

// ------------ RGB helpers (COMMON CATHODE) ------------
// For COMMON ANODE LEDs, swap HIGH<->LOW
void ledOff()    { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_Y, LOW); }
void ledRed()    { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_Y, LOW); }
void ledGreen()  { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_Y, LOW); }
void ledYellow() { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_Y, HIGH); }

// ------------ MQ average reading ------------
int readAirAnalog() {
  analogRead(PIN_MQ); // throw away first sample after mux switch
  long sum = 0;
  for (byte i = 0; i < MQ_AVG_SAMPLES; i++) {
    sum += analogRead(PIN_MQ);
    delay(2);
  }
  return (int)(sum / MQ_AVG_SAMPLES);
}

void setup() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_Y, OUTPUT);
  ledOff();

  Serial.begin(9600);
  dht.begin();

  // OLED init (common address 0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed (0x3C)");
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Health Env Assistant");
  display.display();
  delay(700);
}

void loop() {
  static unsigned long lastRead = 0;
  if (millis() - lastRead < READ_PERIOD_MS) return;
  lastRead = millis();

  // 1) Read sensors
  float hum = dht.readHumidity();
  float tem = dht.readTemperature(); // Celsius
  int air   = readAirAnalog();       // 0..1023

  bool dhtOK = !(isnan(hum) || isnan(tem));

  // 2) Air category
  enum AirCat { AIR_GOOD, AIR_MODERATE, AIR_POOR };
  AirCat airCat = (air < AIR_GOOD_MAX)     ? AIR_GOOD :
                  (air < AIR_MODERATE_MAX) ? AIR_MODERATE :
                                              AIR_POOR;

  // 3) Comfort check
  bool comfy = false;
  if (dhtOK) {
    comfy = (tem >= COMFY_TEMP_MIN && tem <= COMFY_TEMP_MAX &&
             hum >= COMFY_HUM_MIN  && hum <= COMFY_HUM_MAX);
  }

  // 4) Status + LED
  String status;
  if (!dhtOK) {
    status = "SENSOR ERR";
    ledOff();
  } else if (airCat == AIR_POOR) {
    status = "UNHEALTHY (AIR)";
    ledRed();
  } else if (comfy) {
    status = "GOOD";
    ledGreen();
  } else {
    status = "ADJUST (T/H)";
    ledYellow();
  }

  // 5) Warm-up countdown
  unsigned long secs = millis() / 1000UL;
  bool warming = (secs < MQ_WARMUP_SECONDS);

  // 6) OLED update (status SMALL font)
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Health Env Assistant");

  display.setCursor(0,14);
  if (dhtOK) {
    display.print("T: "); display.print(tem, 1); display.println(" C");
    display.print("H: "); display.print(hum, 0); display.println(" %");
  } else {
    display.println("T/H: ERR");
  }
  display.print("Air: "); display.println(air);

  display.setCursor(0,44);
  display.println(status);  // <-- small font, same size as others
  display.display();

  // 7) Serial debug
  Serial.print("T="); Serial.print(tem,1);
  Serial.print("C  H="); Serial.print(hum,0);
  Serial.print("%  Air="); Serial.print(air);
  Serial.print("  Status="); Serial.print(status);
  if (warming) {
    Serial.print("  (MQ warm ");
    Serial.print(MQ_WARMUP_SECONDS - secs);
    Serial.print("s)");
  }
  Serial.println();
}
