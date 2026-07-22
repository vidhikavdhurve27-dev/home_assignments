# Project 8: IoT Pressure & Altitude Logger
Track atmospheric pressure changes over time — predict local weather using the BMP280 sensor.
### Components Required
ESP32 Dev Board
BMP280 Pressure + Altitude + Temperature Sensor
OLED Display 0.96" I2C (SSD1306)
Potentiometer 10kΩ (to set altitude baseline)
LEDs (Red + Green) + resistors
Breadboard + Jumper Wires
###  Altitude Effects & Sea-Level Formula
#### How Altitude Affects Readings: 
Atmospheric pressure drops as altitude increases because there is less air mass pushing down from above. To identify actual weather fronts, meteorologists must isolate this elevation factor. They do this by converting raw Station Pressure into normalized Sea-Level Pressure.
Without this adjustment, a station on a hill would always report a false low-pressure storm warning.
