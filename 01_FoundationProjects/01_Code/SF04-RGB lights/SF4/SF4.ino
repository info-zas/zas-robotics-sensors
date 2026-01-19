// Pin definitions
const int redPin = 9;    // Pin connected to the red LED
const int greenPin = 10; // Pin connected to the green LED
const int bluePin = 11;   // Pin connected to the blue LED (yellow is typically a mix of red and green)

// Function to set the color of the RGB LED
void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);   // Set the red brightness
  analogWrite(greenPin, green); // Set the green brightness
  analogWrite(bluePin, blue);   // Set the blue brightness
}

void setup() {
  // Set the LED pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  // Red color
  setColor(255, 0, 0); // Full red
  delay(1000);         // Wait for 1 second

  // Green color
  setColor(0, 255, 0); // Full green
  delay(1000);         // Wait for 1 second

  // Blue color
  setColor(0, 0, 255); // Full blue
  delay(1000);         // Wait for 1 second

  // Yellow color (mix of red and green)
  setColor(255, 255, 0); // Full red and green
  delay(1000);           // Wait for 1 second

  // Cyan color (mix of green and blue)
  setColor(0, 255, 255); // Full green and blue
  delay(1000);           // Wait for 1 second

  // Magenta color (mix of red and blue)
  setColor(255, 0, 255); // Full red and blue
  delay(1000);           // Wait for 1 second

  // White color (mix of red, green, and blue)
  setColor(255, 255, 255); // Full red, green, and blue
  delay(1000);             // Wait for 1 second

  // Off
  setColor(0, 0, 0); // Turn off all colors
  delay(1000);       // Wait for 1 second
}