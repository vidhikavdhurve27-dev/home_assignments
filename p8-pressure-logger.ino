#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h> // Switched to BMP180/BMP085 official driver
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BMP085 bmp; // Create BMP180 object instance

const int POT_PIN = 34;

// Historical log configuration (24 indices)
const int MAX_READINGS = 24;
float pressureHistory[MAX_READINGS];
int readingCount = 0;

// Timing mechanisms (1 minute for demo instead of 1 hour)
const unsigned long LOG_INTERVAL = 60000UL; 
unsigned long lastLogTime = 0;

// Custom Render Pipeline Helper for Screen UI Vectors
void drawTrendArrow(int x, int y, int direction) {
  if (direction == 1) { // Upward Arrow Vector
    display.drawLine(x, y + 10, x, y, SSD1306_WHITE);
    display.drawLine(x, y, x - 4, y + 4, SSD1306_WHITE);
    display.drawLine(x, y, x + 4, y + 4, SSD1306_WHITE);
  } else if (direction == -1) { // Downward Arrow Vector
    display.drawLine(x, y, x, y + 10, SSD1306_WHITE);
    display.drawLine(x, y + 10, x - 4, y + 6, SSD1306_WHITE);
    display.drawLine(x, y + 10, x + 4, y + 6, SSD1306_WHITE);
  } else { // Flat Lateral Arrow Vector (Steady)
    display.drawLine(x - 5, y + 5, x + 5, y + 5, SSD1306_WHITE);
    display.drawLine(x + 5, y + 5, x + 1, y + 1, SSD1306_WHITE);
    display.drawLine(x + 5, y + 5, x + 1, y + 9, SSD1306_WHITE);
  }
}

void setup() {
  Serial.begin(115200);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED failure"));
    for(;;);
  }
  
  // Initialize BMP180 (Defaults automatically to standard I2C address 0x77)
  if (!bmp.begin()) { 
    Serial.println(F("BMP180 not found! Check wiring."));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Prime array with an initial reading to prevent empty lookups
  float initialStationP = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  float initialTemp = bmp.readTemperature();
  int potVal = analogRead(POT_PIN);
  float initialAlt = map(potVal, 0, 4095, 0, 2000);
  float initialSeaLevelP = initialStationP * pow(1.0 - (0.0065 * initialAlt) / (initialTemp + (0.0065 * initialAlt) + 273.15), -5.257);
  
  for(int i = 0; i < MAX_READINGS; i++) {
    pressureHistory[i] = initialSeaLevelP;
  }
  readingCount = 1;
  lastLogTime = millis();
}

void loop() {
  // 1. Read Potentiometer to scale Elevation Compensation Input (0 - 2000m)
  int potValue = analogRead(POT_PIN);
  float localAltitude = map(potValue, 0, 4095, 0, 2000);

  // 2. Read environmental station raw telemetry from BMP180
  float temp = bmp.readTemperature();
  float stationPressure = bmp.readPressure() / 100.0F; // Pa to hPa

  // 3. Compute sea level pressure equation mapping local altitude variables
  float seaLevelPressure = stationPressure * pow(1.0 - (0.0065 * localAltitude) / (temp + (0.0065 * localAltitude) + 273.15), -5.257);

  // 4. Log intervals to FIFO structural buffer
  if (millis() - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = millis();
    
    // Shift elements leftward (FIFO Queue eviction)
    for (int i = 0; i < MAX_READINGS - 1; i++) {
      pressureHistory[i] = pressureHistory[i + 1];
    }
    pressureHistory[MAX_READINGS - 1] = seaLevelPressure;
    if (readingCount < MAX_READINGS) readingCount++;

    Serial.print("Logged BMP180 Sea-Level Pressure: "); Serial.print(seaLevelPressure, 2); Serial.println(" hPa");
  }

  // 5. Evaluate weather trend over last 3 updates (3 minutes in demo mode)
  float oldPressure = pressureHistory[MAX_READINGS - 3];
  float deltaP = seaLevelPressure - oldPressure;

  String trendStr = "Steady";
  String forecastStr = "Unchanged";
  int arrowDirection = 0; // 0: Steady, 1: Rising, -1: Falling

  if (deltaP >= 1.0) {
    trendStr = "Rising";
    forecastStr = "Good Weather";
    arrowDirection = 1;
  } else if (deltaP <= -1.0) {
    trendStr = "Falling";
    forecastStr = "Rain Likely";
    arrowDirection = -1;
  }

  // 6. Graphics Pipeline Output
  display.clearDisplay();
  
  // Render Header Bar
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("METEO BMP180");
  display.setCursor(85, 0);
  display.print("Alt:"); display.print((int)localAltitude); display.print("m");
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  // Show Calculated Sea-Level Pressure
  display.setCursor(0, 16);
  display.print("SLP: ");
  display.setTextSize(2);
  display.print(seaLevelPressure, 1); 
  display.setTextSize(1);
  display.print(" hPa");

  // Forecast Text Output
  display.setCursor(0, 36);
  display.print("Trend: "); display.print(trendStr);
  display.setCursor(0, 48);
  display.print("Forecast: ");
  display.setCursor(0, 56);
  display.print(forecastStr);

  // Draw Context-Aware Micro-Trend Vector Arrow Icon
  drawTrendArrow(110, 24, arrowDirection);

  display.display();
  delay(250); 
}

