# Project 2: Gas & Fire Safety Alert System
A dual-sensor safety device that detects gas leaks and fire simultaneously — practical for any home.
### Components Required
Arduino Uno R3
MQ-2 Gas Sensor (LPG / Smoke)
Flame Sensor
Active Buzzer (×2)
LEDs: Red, Yellow, Green + resistors
Breadboard + Jumper Wires
### Technical Notes
MQ-2 Sensor Warm-Up Requirement
Stabilization: Requires 1–2 minutes of pre-heating upon boot to reach operating temperature.
Accuracy: Readings fluctuate wildly during this window; initial values should be ignored to prevent false alarms.
Burn-in: New sensors require a 24–48 hour continuous power-on period to clear manufacturing residues.
### Why the Flame Sensor is Active-Low
Circuit Design: The onboard LM393 comparator pulls the digital output down to GND (LOW / 0V) when IR light from a flame hits the photodiode.
Default State: It stays HIGH (5V) via pull-up resistors when the environment is safe. 
Fail-Safe: Ensures reliable triggering and prevents accidental environmental noise from being mistaken for a fire signature.
Link from Wokwi: https://wokwi.com/projects/469871255283761153
