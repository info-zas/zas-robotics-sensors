#include <Wire.h>             // Required for I2C communication with OLED
#include <Adafruit_GFX.h>     // Core graphics library for OLED
#include <Adafruit_SSD1306.h> // Library for SSD1306 OLED displays

// --- Ultrasonic Sensor Definitions ---
#define TRIG_PIN A1 // Trigger pin connected to Analog Pin A1
#define ECHO_PIN A2 // Echo pin connected to Analog Pin A2

// --- Buzzer Definition ---
#define BUZZER_PIN 12 // Buzzer connected to Digital Pin 9

// --- OLED Display Definitions ---
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The I2C address for most SSD1306 OLEDs is 0x3C or 0x3D
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Global Variables ---
long duration;
float distanceCm;

void setup() {
  // --- Serial Communication Setup ---
  Serial.begin(9600);
  Serial.println("Ultrasonic Sensor with OLED and Buzzer");

  // --- Pin Mode Setup ---
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output

  // --- OLED Display Initialization ---
  // If the display doesn't begin, try changing 0x3C to 0x3D or vice-versa
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the display buffer.
  display.clearDisplay();
  display.setTextSize(1);       // Smallest text size
  display.setTextColor(SSD1306_WHITE); // White text on black background
  display.setCursor(0,0);       // Set cursor to top-left corner
  display.println("Initializing...");
  display.display();            // Show initial message
  delay(1000);
}

void loop() {
  // --- Ultrasonic Sensor Reading ---
  // Clear the trigPin by setting it LOW for 2 microseconds
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Set the trigPin HIGH for 10 microseconds to send a pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pin: returns the duration of the pulse in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters
  // Speed of sound is 343 meters/second or 0.0343 cm/microsecond.
  // The sound travels to the object and back, so divide by 2.
  distanceCm = duration * 0.0343 / 2;

  // --- Serial Monitor Output ---
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // --- OLED Display Output ---
  display.clearDisplay();       // Clear the display buffer
  display.setTextSize(2);       // Set text size for distance
  display.setCursor(0,0);       // Set cursor for "Distance:"
  display.print("Distance:");
  
  display.setCursor(0, 30);     // Move cursor down for value
  display.setTextSize(3);       // Larger text for the actual distance
  display.print(distanceCm);
  display.print(" cm");
  display.display();            // Update the OLED display

  // --- Buzzer Control (Example: buzz if less than 20cm) ---
  if (distanceCm < 10 && distanceCm > 0) { // Check distance and ensure it's not a false reading of 0
    tone(BUZZER_PIN, 1000); // Play a tone of 1000 Hz
  } else {
    noTone(BUZZER_PIN); // Stop the tone
  }

  // Add a small delay to avoid excessive readings and flickering
  delay(1000); // Update every 200 milliseconds (0.2 seconds)
}