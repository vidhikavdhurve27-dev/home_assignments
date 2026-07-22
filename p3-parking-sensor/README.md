# Project 3: Ultrasonic Parking Assistant
Replicate a real car parking sensor — distance-based LED + buzzer alert with OLED readout.
### Components Required
Arduino Uno R3
HC-SR04 Ultrasonic Sensor
Active Buzzer
LEDs: Red, Yellow, Green + 220Ω resistors
OLED Display 0.96" I2C (optional — for display)
Breadboard + Jumper Wires
### The physics formula used for distance calculation
long duration = pulseIn(ECHO, HIGH);
float distance = duration * 0.034 / 2;
### Link from Wokwi: https://wokwi.com/projects/468714987091362817
