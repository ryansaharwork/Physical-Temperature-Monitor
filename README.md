# Temperature Monitor Project 

A real-time **temperature monitoring system** built with Arduino. It displays current temperature, adjusts settings, and shows status using LEDs, a servo, and an LCD screen.

![Temperature Monitor Demo](spedupdemo.gif)

## ⚙️ Features
- **Live temperature readings** using DS18B20 sensor 📈  
- **LCD display** shows temperature + thresholds 🖥️  
- **Rotary encoder** to adjust min & max temperature limits 🔁  
- **RGB LED indicators** (Red = Hot 🔴, Blue = Cold 🔵, Green = Normal 🟢)  
- **Servo motor dial** shows temp status with movement ➰  
- **EEPROM** saves settings after power-off 💾  
- **Serial communication** for sending/receiving data over UART 🔌  

## 🔁 Demo Mode
For demonstration, **data is self-looped** — the device sends and receives its own data to show communication in action.

## 🧪 How It Works
- **Turn the encoder** to set temperature thresholds.  
- **Watch the display**, LED color, and servo respond in real-time.  
- **Press the SEND button** to transmit temperature data.  

## 🔧 Hardware Used
- Arduino Uno R3  
- DS18B20 Temp Sensor  
- LCD Keypad Shield  
- RGB LED (common anode)  
- Rotary Encoder (with push-button)  
- Servo Motor  
- Push-button switch  
- Tri-state buffer + resistors/wires/breadboard  

## 🚀 Setup
1. **Wire the components** (see demo GIF for reference).  
2. **Upload the code** to your Arduino Uno.  
3. **Open Serial Monitor** to observe communication.  
