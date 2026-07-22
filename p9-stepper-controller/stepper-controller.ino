#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>
#include <math.h>

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Stepper Configuration 
// 28BYJ-48 geared motor has 2048 full steps per revolution.
const int STEPS_PER_REV = 2048; 
// Note the pin order: 8, 10, 9, 11 is required by Stepper.h for ULN2003 sequencing
Stepper myStepper(STEPS_PER_REV, 8, 10, 9, 11);

// Pin Assignments
const int POT_PIN = A0;
const int BTN_CW = 2;
const int BTN_CCW = 3;
const int BTN_HOME = 4;
const int LED_PIN = 7;

// Global Tracking Variables
long currentSteps = 0;
long targetSteps = 0;
float currentAngle = 0.0;
float targetAngle = 0.0;
String motorDir = "STOP";

// Button Debounce States
bool lastCwState = HIGH;
bool lastCcwState = HIGH;
bool lastHomeState = HIGH;

// Timing Variables to prevent screen lag
unsigned long lastOledUpdateTime = 0;
const unsigned long OLED_INTERVAL = 200; // Update screen every 200ms (5Hz)

// Setup Routine
void setup() {
  Serial.begin(115200);
  
  // Configure Inputs with Internal Pullups (Pressed = LOW)
  pinMode(BTN_CW, INPUT_PULLUP);
  pinMode(BTN_CCW, INPUT_PULLUP);
  pinMode(BTN_HOME, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 24);
  display.println("System Booting...");
  display.display();
  delay(1000);
}

// Loop Routine
void loop() {
  // 1. Read Buttons First (Edge Detection)
  bool cwState = digitalRead(BTN_CW);
  bool ccwState = digitalRead(BTN_CCW);
  bool homeState = digitalRead(BTN_HOME);

  bool buttonActive = false;

  // CW Button Press (Transitions from HIGH to LOW)
  if (cwState == LOW) {
    buttonActive = true;
    motorDir = "CW";
    digitalWrite(LED_PIN, HIGH);
    myStepper.setSpeed(14); // Set running speed
    myStepper.step(10);     // Step manually while held
    currentSteps += 10;
  }
  
  // CCW Button Press
  else if (ccwState == LOW) {
    buttonActive = true;
    motorDir = "CCW";
    digitalWrite(LED_PIN, HIGH);
    myStepper.setSpeed(14);
    myStepper.step(-10);
    currentSteps -= 10;
  }

  // Home Button Press
  if (homeState == LOW && lastHomeState == HIGH) {
    currentSteps = 0;
    Serial.println(F("[EVENT] Home Position Recalibrated to 0"));
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
  }
  lastHomeState = homeState;

  // 2. Automated Tracking (Only runs if manual jog buttons aren't being held)
  if (!buttonActive) {
    int potValue = analogRead(POT_PIN);
    
    // Add a small deadzone tolerance (e.g., noise filtering)
    static int lastPotValue = 0;
    if (abs(potValue - lastPotValue) > 3) { 
      targetAngle = map(potValue, 0, 1023, 0, 360);
      targetSteps = round((targetAngle / 360.0) * STEPS_PER_REV);
      lastPotValue = potValue;
    }

    long stepDifference = targetSteps - currentSteps;
    
    if (abs(stepDifference) > 2) { // 2-step deadband prevents motor oscillation
      int stepDirection = (stepDifference > 0) ? 1 : -1;
      motorDir = (stepDirection > 0) ? "CW" : "CCW";
      
      digitalWrite(LED_PIN, HIGH);
      myStepper.setSpeed(12); // Safe speed for regular tracking
      myStepper.step(stepDirection);
      currentSteps += stepDirection;
    } else {
      motorDir = "STILL";
      digitalWrite(LED_PIN, LOW);
    }
  }

  // 3. Calculate Absolute Angles safely
  currentAngle = fmod((currentSteps * 360.0) / STEPS_PER_REV, 360.0);
  if (currentAngle < 0) currentAngle += 360.0; 

  // 4. Update Telemetry Systems asynchronously (NON-BLOCKING)
  if (millis() - lastOledUpdateTime >= OLED_INTERVAL) {
    lastOledUpdateTime = millis();
    updateOLED();
  }
  
  logSerial();
}

// Render Data to OLED Screen
void updateOLED() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("--- STEPPER CONTR ---");
  
  display.setCursor(0, 16);
  display.print("Current: "); display.print(currentAngle, 1); display.write(247); 
  
  display.setCursor(0, 28);
  display.print("Target : "); display.print(targetAngle, 1); display.write(247);
  
  display.setCursor(0, 40);
  display.print("Steps  : "); display.print(currentSteps);
  
  display.setCursor(0, 52);
  display.print("Dir    : "); display.print(motorDir);
  
  display.display();
}

// Log Data over Hardware Serial Pipeline
void logSerial() {
  static unsigned long lastLogTime = 0;
  if (millis() - lastLogTime > 300) { 
    Serial.print(F("STP:")); Serial.print(currentSteps);
    Serial.print(F(" | ANG:")); Serial.print(currentAngle, 1);
    Serial.print(F(" | TRG:")); Serial.print(targetAngle, 1);
    Serial.print(F(" | DIR:")); Serial.println(motorDir);
    lastLogTime = millis();
  }
}
