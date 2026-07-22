# Project 5: Bluetooth Home Light Controller
Control your room lights from your phone via Bluetooth — no Wi-Fi needed.
### Components Required 
ESP32 Dev Board (built-in BLE + Classic BT)
2-Channel Relay Module
LEDs (simulate light bulbs) + resistors
Push Button (manual override)
Active Buzzer (confirmation beep)
Breadboard + Jumper Wires
### Supported Bluetooth Data Commands
The system firmware filters out data string padding like carriage return (\r), newline (\n), or whitespace character markers. Commands are strictly case-sensitive.
### Output Actuator Controls
| Character Command | Action Vector Target | Hardware Interface Model |Output Result |
| --- | --- | --- | --- |
| 1 | Channel 1 Output | Relay 1 Trigger Pin (GPIO 23) | Turns ON Simulation LED 1 |
| a | Channel 1 Output | Relay 1 Trigger Pin (GPIO 23) | Turns OFF Simulation LED 1 |
| 2 | Channel 2 Output | Relay 2 Trigger Pin (GPIO 22) | Turns ON Simulation LED 2 |
| b | Channel 2 Output | Relay 2 Trigger Pin (GPIO 22) | Turns OFF Simulation LED 2 |
| 3 | Channel 3 Output | ESP32 Direct Digital Pin (GPIO 21) | Turns ON Direct Simulation LED 3 |
| c | Channel 3 Output | ESP32 Direct Digital Pin (GPIO 21) | Turns OFF Direct Simulation LED 3 |

Link from Wokwi: https://wokwi.com/projects/469980453421451265
