#define BLYNK_TEMPLATE_ID " " // Enter Your BLYNK_TEMPLATE_ID 
#define BLYNK_TEMPLATE_NAME " "// Enter your BLYNK_TEMPLATE _NAME
#define BLYNK_AUTH_TOKEN " " //Enter your BLYNK_AUTH_TOKEN

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

#define DHTPIN 32         // Pin where the DHT sensor is connected
#define DHTTYPE DHT22     // Type of the DHT sensor
#define TDSPIN 36    // Pin where the TDS sensor is connected

// Your WiFi credentials
char ssid[] = ""; // Enter your wifi name
char pass[] = ""; // Enter password

DHT dht(DHTPIN, DHTTYPE);

int relayPin = 12; // Pin connected to the relay module, change if necessary

// Moving average settings
#define NUM_SAMPLES 10  // Number of samples to average
int sensorValues[NUM_SAMPLES]; // Array to store sensor values
int sensorIndex = 0; // Index for the circular buffer

BLYNK_WRITE(V2) {
  digitalWrite(relayPin, param.asInt()); // Write the value from the Blynk app to the relay pin
}

// Function to calculate moving average
float calculateMovingAverage(int newValue) {
  sensorValues[sensorIndex] = newValue;
  sensorIndex = (sensorIndex + 1) % NUM_SAMPLES; // Update circular buffer index
  
  // Calculate average
  float average = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    average += sensorValues[i];
  }
  average /= NUM_SAMPLES;
  
  return average;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Ensure relay is initially off
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();
  
  // Read temperature and humidity
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Send temperature and humidity to Blynk
  Blynk.virtualWrite(V0, humidity);
  Blynk.virtualWrite(V1, temperature);

  // Read TDS sensor value
  int sensorValue = analogRead(TDSPIN);
  float smoothedSensorValue = calculateMovingAverage(sensorValue);
  float ppm = map(smoothedSensorValue, 0, 4095, 0, 5000); // Example conversion, adjust as needed

  // Print and send TDS value to Blynk
  Serial.print("PPM: ");
  Serial.println(ppm);
  
  Blynk.virtualWrite(V2, ppm);

  // Calculate electrical conductivity
  float voltage = sensorValue * (5.0 / 4095.0); // Assuming 5V Arduino
  float conductivity = 10 * voltage; // Example formula, adjust according to your calibration

  // Print and send electrical conductivity to Blynk
  Serial.print("Electrical Conductivity: ");
  Serial.print(conductivity);
  Serial.println(" µS/cm"); // Microsiemens per centimeter
  Blynk.virtualWrite(V3, conductivity);

  delay(1000); // Adjust delay time as needed for stability
}
