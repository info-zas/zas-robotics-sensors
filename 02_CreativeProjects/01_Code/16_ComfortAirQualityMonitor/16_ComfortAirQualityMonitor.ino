/*
  Comfort + Air-Quality Monitor
  DHT11 (D8) + MQ-135 (A0) + OLED (I2C) + RGB LED (D3/D4/D5) + Touch (D7)

  Behavior:
    - If air is POOR or T/H outside comfort => RED alert (blink), OLED suggests ventilation, alert is LATCHED.
    - Touch button acknowledges (clears) the alert. If conditions are still bad, it will re-latch automatically.
    - If air OK but T/H not comfy => YELLOW "ADJUST (T/H)".
    - If all good => GREEN "GOOD".

  Wiring quick ref:
    DHT11 -> D8, VCC 5V, GND GND
    MQ-135 AO -> A0, VCC 5V, GND GND
    RGB LED (common cathode): D3=R, D4=G, D5=B/Y  (use 220-330Ω resistors if discrete LEDs)
    Touch (TTP223) -> D7 (HIGH on touch)
    SSD1306 I2C: SDA A4, SCL A5, addr 0x3C
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- Pins ----------------
const int PIN_DHT   = 8;
const int PIN_MQ    = A0;
const int PIN_R     = 3;
const int PIN_G     = 6;
const int PIN_B     = 5;      // use as Blue or Yellow pin on traffic-light module
const int PIN_TOUCH = 4;

// ---------------- Display ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Sensors ----------------
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// ---------------- Thresholds (tune) ----------------
const float COMFY_TEMP_MIN = 20.0;   // °C
const float COMFY_TEMP_MAX = 28.0;   // °C
const float COMFY_HUM_MIN  = 30.0;   // %
const float COMFY_HUM_MAX  = 60.0;   // %

const int AIR_GOOD_MAX      = 300;   // raw MQ-135 ADC
const int AIR_MODERATE_MAX  = 600;   // >600 => POOR (alert)

const byte MQ_AVG_SAMPLES   = 16;    // smoothing for MQ-135
const unsigned long READ_PERIOD_MS   = 1000;
const unsigned long MQ_WARMUP_SECONDS = 30;

const unsigned long BLINK_MS = 350;  // RED blink period

// --------------- Alert latch ---------------
bool alertLatched = false;

// --------------- LED helpers (COMMON CATHODE) ---------------
// For COMMON ANODE: swap HIGH <-> LOW below.
void ledOff()    { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledRed()    { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, LOW);  digitalWrite(PIN_B, LOW); }
void ledGreen()  { digitalWrite(PIN_R, LOW);  digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }
void ledYellow() { digitalWrite(PIN_R, HIGH); digitalWrite(PIN_G, HIGH); digitalWrite(PIN_B, LOW); }

// --------------- Read & Smooth MQ-135 ---------------
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
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_TOUCH, INPUT);  // TTP223 is usually active-HIGH
  ledOff();

  Serial.begin(9600);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed (0x3C)");
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Comfort + AQ Monitor");
  display.display();
  delay(700);
}

void loop() {
  static unsigned long lastRead = 0;
  static unsigned long lastBlink = 0;
  static bool blinkOn = true;

  if (millis() - lastRead < READ_PERIOD_MS) return;
  lastRead = millis();

  // 1) Read sensors
  float hum = dht.readHumidity();
  float tem = dht.readTemperature();
  int   air = readAirAnalog();

  bool dhtOK = !(isnan(hum) || isnan(tem));

  // 2) Categorize air
  enum AirCat { AIR_GOOD, AIR_MODERATE, AIR_POOR };
  AirCat airCat = (air < AIR_GOOD_MAX)     ? AIR_GOOD :
                  (air < AIR_MODERATE_MAX) ? AIR_MODERATE :
                                              AIR_POOR;

  // 3) Comfort check (Temp/Humidity)
  bool comfy = false;
  if (dhtOK) {
    comfy = (tem >= COMFY_TEMP_MIN && tem <= COMFY_TEMP_MAX &&
             hum >= COMFY_HUM_MIN  && hum <= COMFY_HUM_MAX);
  }

  // 4) Determine conditions
  bool badAir = (airCat == AIR_POOR);
  bool badTH  = dhtOK ? !comfy : false;
  bool badNow = (badAir || badTH);

  // 5) Latch logic + touch acknowledge
  if (badNow) alertLatched = true;
  if (digitalRead(PIN_TOUCH) == HIGH) {   // acknowledge/reset
    alertLatched = false;
    delay(300); // debounce
  }

  // 6) Status/LEDs
  String status;
  if (!dhtOK) {
    status = "SENSOR ERR";
    ledOff();
  } else if (alertLatched) {
    // Blink RED while latched
    if (millis() - lastBlink > BLINK_MS) {
      lastBlink = millis();
      blinkOn = !blinkOn;
    }
    if (blinkOn) ledRed(); else ledOff();

    if (badAir && badTH)      status = "ALERT: HOT + AIR!";
    else if (badAir)          status = "UNHEALTHY AIR! Vent";
    else /*badTH only*/       status = "UNCOMFY T/H! Vent";
  } else if (!badNow && !alertLatched) {
    if (airCat == AIR_GOOD && comfy) {
      status = "GOOD";
      ledGreen();
    } else {
      status = "ADJUST (T/H)";
      ledYellow();
    }
  }

  // 7) Warm-up hint for MQ-135
  unsigned long secs = millis() / 1000UL;
  bool warming = (secs < MQ_WARMUP_SECONDS);

  // 8) Update OLED (small font everywhere)
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Comfort + AQ Monitor");

  display.setCursor(0,14);
  if (dhtOK) {
    display.print("T: "); display.print(tem, 1); display.println(" C");
    display.print("H: "); display.print(hum, 0); display.println(" %");
  } else {
    display.println("T/H: ERR");
  }
  display.print("Air: "); display.println(air);
  if (warming) { display.println("(MQ warming)"); }

  display.setCursor(0,44);
  display.println(status);
  display.display();

  // 9) Serial debug
  Serial.print("T="); Serial.print(tem,1);
  Serial.print("C  H="); Serial.print(hum,0);
  Serial.print("%  Air="); Serial.print(air);
  Serial.print("  AirCat=");
  Serial.print(airCat==AIR_GOOD?"GOOD":airCat==AIR_MODERATE?"MODERATE":"POOR");
  Serial.print("  comfy="); Serial.print(comfy?"Y":"N");
  Serial.print("  latched="); Serial.print(alertLatched?"Y":"N");
  if (warming) { Serial.print("  (MQ warming)"); }
  Serial.println();
}
