#include <DHT.h>

#define DHTPIN 8     // Pin where the DHT11 is connected
#define DHTTYPE DHT11   // Define the type of DHT sensor

DHT dht(DHTPIN, DHTTYPE); // Create a DHT object

void setup() {
  Serial.begin(9600); // Start serial communication
  dht.begin(); // Initialize the DHT sensor
}

void loop() {
  // Wait a few seconds between measurements
  delay(2000);

  // Read temperature as Celsius
  float temperature = dht.readTemperature(true);
  // Read humidity
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print the results to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
