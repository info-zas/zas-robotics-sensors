/******************************************************************************************
  Project: Traffic Light Control with Pedestrian Touch Button + OLED Status
  Board  : Arduino Uno / Nano (ATmega328P class)
  Inputs : Touch sensor (TTP223) on D8
  Outputs: Traffic light LEDs (RED=D3, YELLOW=D4, GREEN=D5), SSD1306 I²C OLED 128x64

  What it does (real-world model, small scale):
  - Cars are allowed to go (GREEN LED on) by default.
  - A pedestrian taps the touch sensor to request crossing.
  - System safely switches to YELLOW (warning), then RED (cars stop).
  - While RED is on, the OLED shows a live countdown for crossing time.
  - After the countdown, it returns to normal (GREEN).

  Libraries you need (install from Arduino Library Manager):
  - Adafruit GFX Library         by Adafruit
  - Adafruit SSD1306             by Adafruit

  Wiring (typical):
  - Touch sensor (TTP223):
      VCC -> 5V
      GND -> GND
      SIG -> D8
  - LEDs (use ~220Ω resistors if discrete LEDs):
      RED    -> D3  (through resistor to LED -> GND)
      YELLOW -> D4  (through resistor to LED -> GND)
      GREEN  -> D5  (through resistor to LED -> GND)
    (If using a 3-LED traffic module, follow its pin labels; if it's "active-LOW", see notes below.)
  - OLED SSD1306 I²C (0x3C is common):
      VCC -> 5V
      GND -> GND
      SDA -> A4 (Uno)
      SCL -> A5 (Uno)

  Notes:
  - This sketch uses NON-BLOCKING timing (millis()) so the OLED and logic stay responsive.
  - Touch is debounced in software (no chattering).
  - If your touch module is ACTIVE-LOW (reads LOW when pressed), see the "CONFIG SECTION" to flip it.
******************************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* -------------------- OLED SETUP -------------------- */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// -1 means "no reset pin" (common for SSD1306 breakout boards)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* -------------------- PINS (CHANGE HERE IF NEEDED) -------------------- */
const uint8_t PIN_TOUCH  = 8;  // TTP223 touch sensor signal pin
const uint8_t PIN_RED    = 3;  // Red traffic LED
const uint8_t PIN_YELLOW = 4;  // Yellow traffic LED
const uint8_t PIN_GREEN  = 5;  // Green traffic LED

/* -------------------- CONFIG SECTION -------------------- */
// Durations (in milliseconds) – tweak to your liking
const unsigned long YELLOW_DURATION_MS = 2000;  // how long YELLOW stays on
const unsigned long RED_DURATION_MS    = 6000;  // how long pedestrians can cross
const unsigned long RETURN_BUFFER_MS   = 1000;  // brief buffer before going back to GREEN

// Touch input behavior (set these to match your module):
// - If your TTP223 outputs HIGH when touched (most common), keep as below.
const uint8_t TOUCH_INPUT_MODE   = INPUT;  // or INPUT_PULLUP for active-low boards
const int     TOUCH_ACTIVE_LEVEL = HIGH;   // HIGH means "pressed"

// Debounce time for touch (ignore changes faster than this)
const unsigned long TOUCH_DEBOUNCE_MS = 120;

/* -------------------- STATE MACHINE -------------------- */
// We make states with clear names so the logic is easy to read
enum State {
  IDLE_GREEN,     // Default: cars go (GREEN on)
  REQUESTED,      // Touch detected, show "wait" briefly before changing
  YELLOW_PHASE,   // Warning phase
  RED_PHASE,      // Pedestrians crossing (RED on + OLED countdown)
  RETURN_PHASE    // Optional transition back to GREEN
};
State state = IDLE_GREEN;       // start here
unsigned long stateStartedAt = 0;  // when we entered the current state (millis)

/* -------------------- TOUCH DEBOUNCE VARIABLES -------------------- */
bool lastRawTouchLevel   = !TOUCH_ACTIVE_LEVEL;  // last raw reading from the pin
bool logicalPressLatched = false;                // becomes true once a "press" edge is confirmed
unsigned long lastLevelChangeAt = 0;             // time when the raw level last changed

/* -------------------- HELPER: LED CONTROL -------------------- */
void setLights(bool red, bool yellow, bool green) {
  // If your traffic module is "active-LOW" (LED turns on when pin is LOW),
  // change HIGH/LOW below to invert logic, e.g.:
  //   digitalWrite(PIN_RED, red ? LOW : HIGH);
  digitalWrite(PIN_RED,    red    ? HIGH : LOW);
  digitalWrite(PIN_YELLOW, yellow ? HIGH : LOW);
  digitalWrite(PIN_GREEN,  green  ? HIGH : LOW);
}

/* -------------------- HELPER: OLED TEXT -------------------- */
void oledPrint3(const String& l1,
                const String& l2 = "",
                const String& l3 = "") {
  display.clearDisplay();
  display.setTextSize(1);                 // small font so we fit multiple lines
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(l1);
  if (l2.length()) display.println(l2);
  if (l3.length()) display.println(l3);
  display.display();
}

/* -------------------- HELPER: ENTER A NEW STATE -------------------- */
void enterState(State next) {
  state = next;
  stateStartedAt = millis();

  switch (state) {
    case IDLE_GREEN:
      // Cars go
      setLights(false, false, true);
      oledPrint3("Cars: GO", "Tap to cross", "(Touch sensor)");
      // Once we return to GREEN, we clear any old "press" so next tap will be fresh
      logicalPressLatched = false;
      break;

    case REQUESTED:
      // Still GREEN for a short user-feedback delay
      setLights(false, false, true);
      oledPrint3("Request received", "Please wait...", "Preparing to stop");
      break;

    case YELLOW_PHASE:
      // Warn cars to slow down
      setLights(false, true, false);
      oledPrint3("Warning", "Cars: SLOW", "Yellow light");
      break;

    case RED_PHASE:
      // Cars stop; pedestrians can cross
      setLights(true, false, false);
      // The countdown text is updated continuously in loop()
      break;

    case RETURN_PHASE:
      // Brief transition (can be yellow or all-off)
      setLights(false, true, false);
      oledPrint3("Get ready", "Resuming traffic", "...");
      break;
  }
}

/* -------------------- TOUCH HANDLING (WITH DEBOUNCE) -------------------- */
bool wasHighStable = false;  // remembers if touch was stably "pressed" before

// Returns true exactly once per press (edge-triggered), thanks to latch
void updateTouchDebounce() {
  // Read the current raw level from the pin
  bool rawLevel = digitalRead(PIN_TOUCH);

  // If your sensor is ACTIVE-LOW, you can invert here instead:
  // rawLevel = !rawLevel;

  unsigned long now = millis();

  // Detect level change and start debounce timing
  if (rawLevel != lastRawTouchLevel) {
    lastRawTouchLevel = rawLevel;
    lastLevelChangeAt = now;
  }

  // After the level stays stable past the debounce time, we trust it
  if ((now - lastLevelChangeAt) >= TOUCH_DEBOUNCE_MS) {
    bool isPressed = (rawLevel == TOUCH_ACTIVE_LEVEL);

    // Rising edge detect: went from not pressed to pressed
    if (isPressed && !wasHighStable) {
      wasHighStable = true;
      logicalPressLatched = true;   // this flags a "new press" for the main logic
    }
    // Falling edge: released
    if (!isPressed && wasHighStable) {
      wasHighStable = false;
    }
  }
}

/* -------------------- SETUP -------------------- */
void setup() {
  // Configure pins
  pinMode(PIN_TOUCH, TOUCH_INPUT_MODE);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);

  // Start I²C + OLED
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If OLED fails, we continue headless (no display),
    // but in class it's good to check wiring/I2C address if nothing shows.
  }

  // Quick self-test so you can verify LED wiring visually
  setLights(true, false, false);  delay(600);  // RED
  setLights(false, true, false);  delay(600);  // YELLOW
  setLights(false, false, true);  delay(600);  // GREEN
  setLights(false, false, false);              // all off

  // Start in normal driving mode
  enterState(IDLE_GREEN);
}

/* -------------------- LOOP (MAIN PROGRAM) -------------------- */
void loop() {
  // Keep checking the touch sensor with debounce
  updateTouchDebounce();

  // Current time for non-blocking timing checks
  unsigned long now = millis();

  // Main state machine
  switch (state) {

    case IDLE_GREEN:
      // Wait for a fresh press (latched),
      // then acknowledge the request and proceed
      if (logicalPressLatched) {
        enterState(REQUESTED);
      }
      break;

    case REQUESTED:
      // Small "please wait" period so the user gets feedback
      if (now - stateStartedAt >= 800) {  // 0.8 seconds
        enterState(YELLOW_PHASE);
      }
      break;

    case YELLOW_PHASE:
      // When the yellow duration expires, go to RED
      if (now - stateStartedAt >= YELLOW_DURATION_MS) {
        enterState(RED_PHASE);
      }
      break;

    case RED_PHASE: {
      // While RED is active, we show a live countdown on the OLED
      unsigned long elapsed = now - stateStartedAt;

      if (elapsed >= RED_DURATION_MS) {
        // Done crossing; time to return to normal
        enterState(RETURN_PHASE);
      } else {
        // Compute how many whole seconds remain (friendly display)
        int secondsLeft = (int)((RED_DURATION_MS - elapsed) / 1000UL) + 1;

        // Draw status & countdown (kept simple and readable)
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Cars: STOP");
        display.println("Pedestrians: CROSS");
        display.print("Time left: ");
        display.print(secondsLeft);
        display.println(" s");
        display.display();
      }
      break;
    }

    case RETURN_PHASE:
      // After a short buffer, go back to GREEN (normal operation)
      if (now - stateStartedAt >= RETURN_BUFFER_MS) {
        enterState(IDLE_GREEN);
      }
      break;
  }

  // NOTE:
  // We never use delay() in the main logic (except for the startup self-test),
  // so everything stays responsive while timers are running.
}

/* ---------------------------------------------------------------------------------------
   HOW TO ADAPT IF THINGS LOOK "INVERTED"
   - If the TOUCH seems "pressed" all the time or never:
       * Change TOUCH_INPUT_MODE to INPUT_PULLUP
       * Change TOUCH_ACTIVE_LEVEL to LOW
       * Or simply invert the "rawLevel" inside updateTouchDebounce().
   - If your traffic module LEDs turn ON when you write LOW:
       * In setLights(), swap HIGH/LOW as commented to make "true" mean "on".
----------------------------------------------------------------------------------------*/
