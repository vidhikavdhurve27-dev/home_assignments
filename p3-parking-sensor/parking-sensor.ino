// importing Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions (Perfect for Arduino Uno R3)
const int TRIG_PIN = 5;      // HC-SR04 Trig Pin
const int ECHO_PIN = 4;      // HC-SR04 Echo Pin
const int LED_YELLOW = 10;   // Warning zone indicator
const int LED_RED = 12;      // Critical zone indicator
const int BUZZER_PIN = 8;    // Buzzer alarm pin

// OLED configuration
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// System Tracking Variables (Non-blocking Timers)
unsigned long lastSensorRead = 0;
unsigned long lastBuzzerToggle = 0;
unsigned long lastFlashToggle = 0;

const unsigned long SENSOR_INTERVAL = 100; // Sample distance every 100ms

// Dynamic State Variables
int buzzerState = LOW;
int flashState = LOW;
float currentDistance = 999.0;

void setup() {
  Serial.begin(115200); // Remember to set Serial Monitor to 115200 baud
  
  // Ultrasonic Sensor Pin Modes
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Indicator Outputs
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize OLED display (I2C address 0x3C is standard for 0.96" screens)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.print("Parking Assistant");
  display.setCursor(25, 40);
  display.print("Initializing...");
  display.display();
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();
  bool newSensorDataAvailable = false;

  // --- 1. Non-Blocking Ultrasonic Sensor Read (Every 100ms) ---
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentTime;
    newSensorDataAvailable = true; // Signal that we have a fresh reading to display
    
    // Generate clean 10-microsecond trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Read acoustic return wave bounce time (microseconds)
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout limit
    
    if (duration > 0) {
      currentDistance = (duration * 0.034) / 2.0;
    } else {
      currentDistance = 999.0; // Clear read fault fallback
    }
  }

  // --- 2. Update Display ONLY when new data arrives (Prevents Flickering/Blank Screens) ---
  if (newSensorDataAvailable) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("PARKING ASSISTANT");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    if (currentDistance > 50.0) {
      display.setCursor(0,15);
      display.print("ZONE: SAFE ");
      display.setCursor(0,40);
      display.print("Car is ");
      display.print(currentDistance, 1);
      display.print("cm away");
    } 
    else if (currentDistance >= 20.0 && currentDistance <= 50.0) {
      display.setCursor(0,18);
      display.print("ZONE: WARNING! ");
      display.setCursor(0,40);
      display.print("Car is ");
      display.print(currentDistance, 1);
      display.print("cm away");
    } 
    else if (currentDistance >= 10.0 && currentDistance < 20.0) {
      display.setCursor(0,18);
      display.print("ZONE: CRITICAL! ");
      display.setCursor(0,40);
      display.print("Car is only ");
      display.print(currentDistance, 1);
      display.print("cm away");
    } 
    else if (currentDistance < 10.0) {
      display.setCursor(0,18);
      display.print("ZONE: DANGER!! ");
      display.setCursor(0,35);
      display.print("STOP THE CAR!!");
      display.setCursor(0,45);
      display.print("Car is too close!! ");
    }
    
    display.display(); // Push frame buffer data to physical display
  }

  // --- 3. Alarm Hardware Control Engine (Must run continuously for smooth buzzing/flashing) ---
  if (currentDistance > 50.0) {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
  } 
  else if (currentDistance >= 20.0 && currentDistance <= 50.0) {
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
    
    if (currentTime - lastBuzzerToggle >= 500) {
      lastBuzzerToggle = currentTime;
      buzzerState = !buzzerState;
      if (buzzerState) tone(BUZZER_PIN, 1000); else noTone(BUZZER_PIN);
    }
  } 
  else if (currentDistance >= 10.0 && currentDistance < 20.0) {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
    
    if (currentTime - lastBuzzerToggle >= 200) {
      lastBuzzerToggle = currentTime;
      buzzerState = !buzzerState;
      if (buzzerState) tone(BUZZER_PIN, 1500); else noTone(BUZZER_PIN);
    }
  } 
  else if (currentDistance < 10.0) {
    tone(BUZZER_PIN, 2000); // Continuous piercing lock frequency
    
    if (currentTime - lastFlashToggle >= 80) { 
      lastFlashToggle = currentTime;
      flashState = !flashState;
      digitalWrite(LED_YELLOW, flashState);
      digitalWrite(LED_RED, flashState);
    }
  }
}
