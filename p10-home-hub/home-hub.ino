#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h> // Library for BMP180
#include <ArduinoJson.h>

// --- Configuration ---
// --- Wokwi Specific Wi-Fi Configuration ---
const char* ssid = "Wokwi-GUEST"; // Do NOT change this name
const char* password = "";        // Leave this completely blank ""

// --- Public Broker for Simulation ---
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// --- Pin Definitions ---
#define DHTPIN 23
#define DHTTYPE DHT22
#define LDR_PIN 35

// HC-SR04 Ultrasonic Pins
#define TRIG_PIN 12
#define ECHO_PIN 13

const int relayPins[4] = {25, 26, 27, 14};
bool relayStates[4] = {false, false, false, false};

// --- Object Initialization ---
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp; // BMP180 on I2C (Pins 21 & 22)
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WebServer server(80);

// --- Global Variables ---
unsigned long lastMsgTime = 0;
const long interval = 5000; // Telemetry interval (5 seconds)

// --- Function to Measure Distance (HC-SR04) ---
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure the bounce back time in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5 meters max)
  
  if (duration == 0) return -1.0; // Out of range or error
  
  // Calculate distance: Speed of sound is ~343 m/s or 0.0343 cm/us
  float distance = (duration * 0.0343) / 2;
  return distance;
}

// --- MQTT Callback (Incoming Relay Commands) ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  
  if (doc.containsKey("relay") && doc.containsKey("state")) {
    int relayNum = doc["relay"].as<int>() - 1;
    bool state = doc["state"].as<bool>();
    
    if (relayNum >= 0 && relayNum < 4) {
      relayStates[relayNum] = state;
      digitalWrite(relayPins[relayNum], state ? LOW : HIGH); // Active Low
      mqttClient.publish("home/hub/relay/status", state ? "ON" : "OFF");
    }
  }
}

// --- HTTP Server Route ---
void handleRoot() {
  StaticJsonDocument<500> doc;
  doc["temperature"] = dht.readTemperature();
  doc["humidity"] = dht.readHumidity();
  doc["pressure_hpa"] = bmp.readPressure() / 100.0F;
  doc["light_level"] = analogRead(LDR_PIN);
  doc["distance_cm"] = getDistanceCM();
  
  JsonArray relays = doc.createNestedArray("relays");
  for(int i=0; i<4; i++) relays.add(relayStates[i]);

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// --- Network Setup ---
void setupNetworks() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  
  server.on("/api/status", HTTP_GET, handleRoot);
  server.begin();
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    if (WiFi.status() != WL_CONNECTED) WiFi.begin(ssid, password);
    if (mqttClient.connect("ESP32_Home_Hub")) {
      mqttClient.subscribe("home/hub/relay/cmd");
    } else {
      delay(5000);
    }
  }
}

// --- Core Setup ---
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();
  
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }
  
  pinMode(LDR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Keep relays off initially
  }
  
  setupNetworks();
}

// --- Core Loop ---
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();
  server.handleClient();
  
  // Timed Telemetry Publishing over MQTT
  unsigned long now = millis();
  if (now - lastMsgTime > interval) {
    lastMsgTime = now;
    
    StaticJsonDocument<300> doc;
    doc["temp"] = dht.readTemperature();
    doc["hum"] = dht.readHumidity();
    doc["press"] = bmp.readPressure() / 100.0F; // Pa to hPa
    doc["lux"] = analogRead(LDR_PIN);
    doc["distance"] = getDistanceCM();
    
    char buffer[256];
    serializeJson(doc, buffer);
    mqttClient.publish("home/hub/telemetry", buffer);
  }
}
