Temperature Monitor Project (EE 109 – Spring 2025)
Overview
This project is a temperature monitoring system built using an Arduino Uno and a digital temperature sensor. It displays live temperature readings on an LCD screen, indicates status using LEDs and a servo dial, and supports sending and receiving data through serial communication.

Features
Real-Time Temperature Display: Uses a DS18B20 digital sensor to read the ambient temperature.

LCD Interface: Shows current temperature and user-adjustable settings.

LED Indicators:

Red: Temperature is above the high threshold.

Blue: Temperature is below the low threshold.

Green: Temperature is within the acceptable range (brightness increases with temperature).

Rotary Encoder Control: Adjusts low and high temperature thresholds.

EEPROM Storage: Saves threshold settings even after power-off.

Servo Dial: Moves based on temperature to visually represent status.

Serial Communication: Transmits and receives temperature and setting data.

Hardware Used
Arduino Uno R3

DS18B20 Temperature Sensor

LCD Shield

Common anode RGB LED

Rotary Encoder (with push-button)

Servo Motor

Push-button switch (for sending data)

Tri-state buffer (for UART control)

Resistors, wires, and breadboard

Operation
Adjust Settings: Use the encoder to set desired low and high temperature thresholds.

Monitor Temperature: Watch the LCD, LED, and servo dial update in real-time.

Send/Receive Data: Press the send button to transmit current data via UART.

Notes
The project is set up to self-loop serial communication for demonstration purposes. This allows users to see both sending and receiving functions in action on a single device.

Demo
A short demonstration video is included below showing the system in action.
(Add your video link or embed it here once it's ready.)
