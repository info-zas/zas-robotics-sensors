/********* Env Monitor (Adafruit SSD1306 + DHT11 + MQ135 + RGB LED) *********/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1          // Some boards use -1 if no reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --------- Pins ---------
#define DHTPIN   8
#define DHTTYPE  DHT11
#define MQ135PIN A0
const uint8_t LED_R = 9, LED_G = 10, LED_B = 7;
const bool COMMON_ANODE = false;

DHT dht(DHTPIN, DHTTYPE);

int baseline = 0;

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  if (COMMON_ANODE) { r=255-r; g=255-g; b=255-b; }
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void setLEDforAQI(int aqi) {
  if (aqi<=50) setRGB(0,255,0);
  else if (aqi<=100) setRGB(255,180,0);
  else if (aqi<=150) setRGB(255,100,0);
  else if (aqi<=200) setRGB(255,0,0);
  else if (aqi<=300) setRGB(150,0,150);
  else setRGB(220,0,120);
}

int pseudoAQI(int analogMinusBase) {
  int v = max(0, analogMinusBase);
  float scale = 500.0/600.0;
  int aqi = (int)(v*scale);
  if (aqi<0) aqi=0; if (aqi>500) aqi=500;
  return aqi;
}

const char* aqiTxt(int aqi){
  if (aqi<=50) return "Good";
  if (aqi<=100) return "Moderate";
  if (aqi<=150) return "Unhlthy-S";
  if (aqi<=200) return "Unhealthy";
  if (aqi<=300) return "Very Unh.";
  return "Hazardous";
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // 0x3C is most common
    Serial.println(F("SSD1306 not found"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Splash
  display.setTextSize(1);
  display.setCursor(0,10);
  display.println("Env Monitor Station");
  display.setCursor(0,30);
  display.println("Warming MQ135...");
  display.display();
  delay(2000);

  // Quick baseline for MQ135
  long acc=0; int n=0; unsigned long start=millis();
  while(millis()-start < 2000) {
    acc += analogRead(MQ135PIN);
    n++; delay(10);
  }
  baseline = (n>0) ? acc/n : 0;

  display.clearDisplay();
  display.setCursor(0,20);
  display.print("Baseline: ");
  display.println(baseline);
  display.display();
  delay(1500);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) { h=0; t=0; }

  int raw = analogRead(MQ135PIN);
  int aqi = pseudoAQI(raw - baseline);

  setLEDforAQI(aqi);

  // --- Draw to OLED ---
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Env Monitor");

  display.setTextSize(1); // BIG font for temp/hum
  display.setCursor(0,16);
  display.print(t,1); display.println(" C");
  display.setCursor(0,36);
  display.print(h,0); display.println(" %");

  display.setTextSize(1);
  display.setCursor(70,16);
  display.print("AQI:");
  display.setCursor(70,28);
  display.setTextSize(2);
  display.println(aqi);

  display.setTextSize(1);
  display.setCursor(70,50);
  display.print(aqiTxt(aqi));

  display.display();

  // Debug
  Serial.print("T:"); Serial.print(t,1);
  Serial.print("C  H:"); Serial.print(h,0);
  Serial.print("%  raw:"); Serial.print(raw);
  Serial.print("  AQI*:"); Serial.println(aqi);

  delay(1000);
}
