#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>
#include <DHT.h>

// Replace with your network credentials
const char *ssid = "Unavailable";
const char *password = "*****";

// Replace with your ThingSpeak channel details
const char *thingSpeakApiKey = "I974NAASKC5K4K4V";
const unsigned long channelNumber = 2359132;

// Define sensor pins
#define tempSensorPin 14   // GPIO14 on ESP32 for DS18B20
#define tdsPin 34           // Analog pin for TDS sensor
#define DHTPIN 26           // GPIO26 on ESP32 for DHT sensor
#define DHTTYPE DHT22       // DHT sensor type
#define turbidityPin 35     // Analog pin for turbidity sensor

// Create a OneWire object for DS18B20
OneWire oneWire(tempSensorPin);

// Pass OneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

// Declare ThingSpeak client
WiFiClient client;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak client
  ThingSpeak.begin(client);

  // Start the Dallas Temperature sensor
  sensors.begin();

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // --- Read temperature from DS18B20 ---
  sensors.requestTemperatures();
  float temperatureDS18B20 = sensors.getTempCByIndex(0);

  // --- Read TDS value ---
  int tdsValue = analogRead(tdsPin);
  float tds = map(tdsValue, 0, 1023, 0, 100);

  // --- Read temperature and humidity from DHT sensor ---
  float tempDHT = dht.readTemperature();
  float humDHT = dht.readHumidity();

  // --- Read turbidity value ---
  int turbidityValue = analogRead(turbidityPin);
  float turbidity = map(turbidityValue, 0, 1023, 0, 100);

  // Print all values on a single line
  Serial.print("DS18 Temperature: ");
  Serial.print(temperatureDS18B20);
  Serial.print(" °C, DHT Temperature: ");
  Serial.print(tempDHT);
  Serial.print(" °C, Humidity: ");
  Serial.print(humDHT);
  Serial.print("%, TDS: ");
  Serial.print(tds);
  Serial.print(", Turbidity: ");
  Serial.println(turbidity);

  // Update ThingSpeak
  ThingSpeak.setField(1, tempDHT);   // DHT11 temperature
  ThingSpeak.setField(2, humDHT);    // DHT11 humidity
  ThingSpeak.setField(3, tds);       // TDS
  ThingSpeak.setField(4, turbidity); // Turbidity
  ThingSpeak.setField(5, temperatureDS18B20); // DS18B20 water temperature

  int statusCode = ThingSpeak.writeFields(channelNumber, thingSpeakApiKey);

  if (statusCode == 200) {
    Serial.println("Channel update successful");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(statusCode));
  }

  delay(20000);  // Delay for 20 seconds before updating ThingSpeak again
}
