// Note: Classic Bluetooth (BluetoothSerial.h) cannot be actively discovered 
// over internet browsers. We use standard Serial command simulation for testing inside Wokwi.
const int relayPins[] = {23, 22, 21}; // 3 Relays now handling all 3 LEDs
const int buttonPins[] = {25, 26, 271};

bool relayStates[] = {HIGH, HIGH, HIGH}; // HIGH = OFF for Active-Low simulation modules
bool lastButtonStates[] = {HIGH, HIGH, HIGH};

void setup() {
  Serial.begin(115200);
  Serial.println("Wokwi 3-Relay Simulation Booted!");
  Serial.println("Use Buttons or type commands: 1/a, 2/b, 3/c");

  for(int i = 0; i < 3; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], relayStates[i]); 
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  // 1. Monitor Virtual Buttons
  for(int i = 0; i < 3; i++) {
    bool currentBtnState = digitalRead(buttonPins[i]);
    if(currentBtnState == LOW && lastButtonStates[i] == HIGH) {
      delay(30); // Quick debounce
      if(digitalRead(buttonPins[i]) == LOW) {
        relayStates[i] = !relayStates[i];
        digitalWrite(relayPins[i], relayStates[i]);
        printFeedback();
      }
    }
    lastButtonStates[i] = currentBtnState;
  }
  // 2. Monitor Serial Inputs (Simulating Phone App Packets)
  if (Serial.available()) {
    char incomingChar = Serial.read();
    if (incomingChar == '\n' || incomingChar == '\r') return;

    switch(incomingChar) {
      case '1': relayStates[0] = LOW;  break; // Relay 1 ON
      case 'a': relayStates[0] = HIGH; break; // Relay 1 OFF
      case '2': relayStates[1] = LOW;  break; // Relay 2 ON
      case 'b': relayStates[1] = HIGH; break; // Relay 2 OFF
      case '3': relayStates[2] = LOW;  break; // Relay 3 ON
      case 'c': relayStates[2] = HIGH; break; // Relay 3 OFF
    }
    digitalWrite(relayPins[0], relayStates[0]);
    digitalWrite(relayPins[1], relayStates[1]);
    digitalWrite(relayPins[2], relayStates[2]);
    printFeedback();
  }
}

void printFeedback() {
  String statusMsg = "ST:";
  statusMsg += (relayStates[0] == LOW) ? "1" : "0";
  statusMsg += ",";
  statusMsg += (relayStates[1] == LOW) ? "1" : "0";
  statusMsg += ",";
  statusMsg += (relayStates[2] == LOW) ? "1" : "0";
  Serial.println(statusMsg);
}
