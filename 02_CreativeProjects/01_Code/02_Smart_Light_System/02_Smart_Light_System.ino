/*
  Smart Light System (LDR Module + Touch + Digital RGB)
  -----------------------------------------------------
  Hardware:
    - LDR module AO  -> A0  (VCC->5V, GND->GND)
    - Touch sensor   -> D8  (VCC->5V, GND->GND, SIG->D8)
    - RGB LED (common cathode):
        R anode -> D3 through 220Ω
        G anode -> D4 through 220Ω
        B anode -> D5 through 220Ω
        Cathode -> GND

  Behavior:
    - Read analog light value from AO (0..1023).
    - If value < lightThreshold (dark)  -> show selected color.
    - If value >= lightThreshold (bright) -> LED off.
    - Each tap on touch sensor cycles to the next color.

  Notes:
    - We use D3, D4, D5 as DIGITAL (no PWM dimming), so colors are ON/OFF mixes.
    - If your RGB LED is COMMON ANODE (common to 5V), invert digital writes
      (HIGH->LOW, LOW->HIGH) in setRGB()/applyColor().
*/

const int PIN_LDR   = A0;   // LDR module AO -> A0
const int PIN_TOUCH = 8;    // TTP223 touch sensor SIG -> D8

// RGB LED digital pins
const int PIN_R = 3;        // Red channel
const int PIN_G = 4;        // Green channel
const int PIN_B = 5;        // Blue channel

// ---- User-tunable settings ----
int lightThreshold = 550;   // Pick after calibration using Serial Monitor
int printIntervalMs = 250;  // How often to print debug info
// -------------------------------

// Color table: each entry {R,G,B} uses 0 (OFF) or 1 (ON)
byte colors[][3] = {
  {1,0,0}, // 0: Red
  {0,1,0}, // 1: Green
  {0,0,1}, // 2: Blue
  {1,1,0}, // 3: Yellow
  {0,1,1}, // 4: Cyan
  {1,0,1}, // 5: Magenta
  {1,1,1}, // 6: White
  {0,0,0}  // 7: Off
};
const int NUM_COLORS = sizeof(colors) / sizeof(colors[0]);
int colorIndex = 0;         // current color state

// Touch edge detection
bool lastTouch = false;

void setup() {
  pinMode(PIN_TOUCH, INPUT);  // TTP223 outputs HIGH when touched (default mode)

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  setRGB(0,0,0);              // start with LED off

  Serial.begin(9600);
  Serial.println("Smart Light System (LDR Module + Touch + Digital RGB)");
  Serial.println("Open Serial Monitor @9600, note LDR values, then tune lightThreshold.");
}

void loop() {
  // 1) Read LDR analog value
  int lightValue = analogRead(PIN_LDR);  // 0..1023

  // 2) Touch tap detection (rising edge)
  bool touchNow = (digitalRead(PIN_TOUCH) == HIGH);
  if (touchNow && !lastTouch) {
    colorIndex = (colorIndex + 1) % NUM_COLORS;  // next color
    Serial.print("Touch detected -> colorIndex = ");
    Serial.println(colorIndex);
  }
  lastTouch = touchNow;

  // 3) Light-based control
  if (lightValue < lightThreshold) {
    applyColor(colorIndex);   // dark -> show chosen color
  } else {
    setRGB(0,0,0);            // bright -> turn off
  }

  // 4) Periodic debug print for calibration
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= (unsigned long)printIntervalMs) {
    lastPrint = millis();
    Serial.print("LDR=");
    Serial.print(lightValue);
    Serial.print("  threshold=");
    Serial.print(lightThreshold);
    Serial.print("  color=");
    Serial.println(colorIndex);
  }

  delay(10); // small delay to ease CPU/serial load
}

// -------- Helper functions --------

// Set channels directly: 1=ON, 0=OFF (for common cathode)
// For COMMON ANODE LEDs, swap HIGH<->LOW in the digitalWrite lines.
void setRGB(byte rOn, byte gOn, byte bOn) {
  digitalWrite(PIN_R, rOn ? HIGH : LOW);
  digitalWrite(PIN_G, gOn ? HIGH : LOW);
  digitalWrite(PIN_B, bOn ? HIGH : LOW);
}

// Apply a color by index from the table
void applyColor(int idx) {
  byte r = colors[idx][0];
  byte g = colors[idx][1];
  byte b = colors[idx][2];
  setRGB(r, g, b);
}
