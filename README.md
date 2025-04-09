# 🚨 IoT-Based Air Quality Monitoring System using ESP32

This project is a smart air quality monitoring system built using an ESP32 microcontroller. It utilizes gas sensors (MQ2 for smoke/LPG and MQ136 for H₂S), a DHT22 temperature and humidity sensor, and a 16x2 LCD to monitor and display real-time environmental data. The system uploads sensor values to ThingSpeak and sends email alerts when dangerous gas levels are detected.

---

## 🔧 Features

- **Real-Time Monitoring:** Detects harmful gases (smoke, H₂S), temperature, and humidity.
- **LCD Display:** Displays sensor values and alerts directly on the screen.
- **Wi-Fi Connectivity:** Sends data to ThingSpeak cloud for remote monitoring.
- **Email Alerts:** Sends instant email notifications during dangerous gas exposure.
- **Visual & Audio Alerts:** Activates buzzer and LED for immediate on-site warnings.
- **Safe Offline Mode:** Continues local alerts and display even without internet access.

---

## 📦 Hardware Requirements

- ESP32 Dev Module  
- MQ2 Gas Sensor  
- MQ136 Gas Sensor  
- DHT22 Temperature & Humidity Sensor  
- 16x2 LCD (Parallel, not I2C)  
- Buzzer  
- LED  
- Resistors, Breadboard, Jumper Wires  

---

## 🌐 Wi-Fi & Cloud Integration

- **Wi-Fi:** Connects to local network using credentials.
- **ThingSpeak:** Uploads real-time data (gas, temp, humidity) to public/private dashboard.
- **SMTP Email:** Sends formatted HTML email alerts when threshold is crossed.

---

## 📤 Sensor Thresholds

- Default danger threshold: `2200` (for both MQ2 and MQ136 sensors)
- Can be adjusted in the code using:  
  ```cpp
  int sensorThres = 2200;
