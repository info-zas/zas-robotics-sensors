/*
  Room Comfort Index (DHT11 on D8, LDR on A0, OLED I2C, RGB on D3/D4/D5)
  ----------------------------------------------------------------------
  - Shows Temp (°C), Humidity (%), Light (0..1023) on OLED
  - Computes comfort status:
      COMFY (green), COMFY but DARK (blue), ADJUST (yellow), UNCOMFY (red)
  - Fully student-documented and easy to tweak
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// -------- Pin Assignments (UPDATED) --------
const int PIN_DHT   = 8;     // DHT11 data now on D8
const int PIN_LDR   = A0;    // LDR AO -> A0
const int PIN_R     = 3;     // RGB Red
const int PIN_G     = 4;     // RGB Green
const int PIN_B     = 5;     // RGB Blue

// -------- Sensor & Display Setup --------
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 = no reset pin

// -------- Tunable Thresholds --------
const float COMFY_TEMP_MIN = 20.0;     // °C
const float COMFY_TEMP_MAX = 26.0;     // °C
const float COMFY_HUM_MIN  = 30.0;     // %
const float COMFY_HUM_MAX  = 60.0;     // %
const int   DARK_LIGHT_MAX = 350;      // light <= this => "too dark"
const unsigned long READ_EVERY_MS = 1000; // ms between sensor reads

// -------- RGB helpers (common cathode) --------
void rgbOff()    { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void rgbRed()    { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void rgbGreen()  { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
void rgbBlue()   { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, HIGH); }
void rgbYellow() { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
// For COMMON ANODE LEDs, swap HIGH<->LOW in the five functions above.

void setup() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  rgbOff();

  Serial.begin(9600);
  dht.begin();

  // OLED init (I2C at 0x3C is common)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed (screen will stay blank)."));
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(F("Room Comfort Index"));
  display.display();
  delay(700);
}

void loop() {
  static unsigned long lastRead = 0;
  if (millis() - lastRead < READ_EVERY_MS) return;
  lastRead = millis();

  // 1) Read sensors
  float h = dht.readHumidity();
  float t = dht.readTemperature();   // Celsius
  int lightRaw = analogRead(PIN_LDR);

  bool dhtOK = !(isnan(h) || isnan(t));

  // 2) Decide status
  enum Status { COMFY, COMFY_DARK, ADJUST, UNCOMFY, SENSOR_ERROR };
  Status status;

  if (!dhtOK) {
    status = SENSOR_ERROR;
  } else {
    bool tempComfy = (t >= COMFY_TEMP_MIN && t <= COMFY_TEMP_MAX);
    bool humComfy  = (h >= COMFY_HUM_MIN  && h <= COMFY_HUM_MAX);

    if (tempComfy && humComfy) {
      status = (lightRaw <= DARK_LIGHT_MAX) ? COMFY_DARK : COMFY;
    } else {
      bool tempFar = (t < COMFY_TEMP_MIN - 3) || (t > COMFY_TEMP_MAX + 3);
      bool humFar  = (h < COMFY_HUM_MIN  -10) || (h > COMFY_HUM_MAX  +10);
      status = (tempFar || humFar) ? UNCOMFY : ADJUST;
    }
  }

  // 3) RGB LED feedback
  switch (status) {
    case COMFY:       rgbGreen();  break;
    case COMFY_DARK:  rgbBlue();   break;
    case ADJUST:      rgbYellow(); break;
    case UNCOMFY:     rgbRed();    break;
    default:          rgbOff();    break; // SENSOR_ERROR
  }

  // 4) OLED update
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(F("Room Comfort Index"));

  display.setCursor(0,14);
  if (dhtOK) {
    display.print(F("Temp: ")); display.print(t,1); display.println(F(" C"));
    display.print(F("Hum : ")); display.print(h,0); display.println(F(" %"));
  } else {
    display.println(F("Temp/Hum: ERR"));
  }
  display.print(F("Light: ")); display.println(lightRaw);

  display.setCursor(0,44);
  display.setTextSize(2);
  switch (status) {
    case COMFY:       display.println(F("COMFY"));       break;
    case COMFY_DARK:  display.println(F("COMFY-DARK"));  break;
    case ADJUST:      display.println(F("ADJUST"));      break;
    case UNCOMFY:     display.println(F("UNCOMFY"));     break;
    case SENSOR_ERROR:display.println(F("SENSOR ERR"));  break;
  }
  display.display();

  // 5) Serial debug
  Serial.print(F("T=")); Serial.print(t,1);
  Serial.print(F("C  H=")); Serial.print(h,0);
  Serial.print(F("%  L=")); Serial.print(lightRaw);
  Serial.print(F("  Status="));
  switch (status) {
    case COMFY: Serial.println(F("COMFY")); break;
    case COMFY_DARK: Serial.println(F("COMFY_DARK")); break;
    case ADJUST: Serial.println(F("ADJUST")); break;
    case UNCOMFY: Serial.println(F("UNCOMFY")); break;
    default: Serial.println(F("SENSOR_ERR")); break;
  }
}
