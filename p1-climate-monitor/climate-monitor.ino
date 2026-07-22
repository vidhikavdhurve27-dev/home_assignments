#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT Configuration
#define DHTPIN 13     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// Buzzer Pin
const int buzzerPin = 18;

void setup() {
  Serial.begin(115200);
  
  // Initialize DHT
  dht.begin();
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Initialize Buzzer
  pinMode(buzzerPin, OUTPUT);
  
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  delay(2000); // DHT11 reads every 2 seconds

  // Reading temperature and humidity
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius

  // Check if valid
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Display on Serial Monitor
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  // Update OLED Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.println("ROOM CLIMATE MONITOR");
  
  display.setTextSize(2);
  display.setCursor(0,20);
  display.print("Tmp: ");
  display.print(t);
  display.println(" C");
  
  display.setCursor(0,45);
  display.print("Hum: ");
  display.print(h);
  display.println(" %");

  display.display();

  // Alert Thresholds: Buzzer goes off if Temp > 28C or Humidity < 40%
  if (t > 28.0 || h < 40.0) {
    digitalWrite(buzzerPin, HIGH);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}

