# 🚁 FlightLab 3D
### Real-Time Drone Visualization Using ESP32 & MPU6050

FlightLab 3D is an ESP32-based drone visualization project that uses the MPU6050 sensor to capture real-time orientation data and display drone movements through an interactive 3D web dashboard.

## ✨ Features
- Real-time Roll and Pitch monitoring.
- MPU6050 sensor integration.
- ESP32 Wi-Fi Access Point connectivity.
- Interactive 3D drone model.
- Live web-based visualization.
- Wireless monitoring through a browser.

## 🛠️ Hardware Components
- ESP32 DevKit V1
- MPU6050 Accelerometer & Gyroscope
- Jumper Wires
- USB Cable

## 🔌 Circuit Connections

| MPU6050 | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## 💻 Technologies Used
- ESP32
- Arduino IDE
- Embedded C++
- HTML
- CSS
- JavaScript
- Web Server
- 3D Visualization

## 🚀 How to Run
1. Connect the MPU6050 sensor to ESP32.
2. Open the `.ino` file in Arduino IDE.
3. Select ESP32 DevKit V1.
4. Upload the code to ESP32.
5. Connect your computer or phone to the `Drone-ESP32` Wi-Fi network.
6. Open your browser and visit:

   `http://192.168.4.1`

7. View the live 3D drone visualization.

## 📊 Working Principle

The MPU6050 sensor measures acceleration and angular velocity. ESP32 processes the sensor readings and sends orientation data to a web dashboard over Wi-Fi. The 3D drone model updates its Roll and Pitch movements in real time.

## 📸 Project Screenshots

### Hardware Setup with Live Dashboard
ESP32 and MPU6050 on a breadboard, with the dashboard running on the laptop and responding to the sensor's tilt.

![Hardware setup with live dashboard](<img width="772" height="433" alt="hardware-setup" src="https://github.com/user-attachments/assets/78f013fd-8d0b-4eb3-8856-a2a94fb30241" />)


### FlightLab Web Dashboard
Live 3D airframe view with Roll and Pitch telemetry and system status.

![FlightLab web dashboard](<img width="967" height="487" alt="dashboard" src="https://github.com/user-attachments/assets/932f5df0-2d47-4499-9ca7-0ee7871447c3" />
)

## 🎥 Project Demo

Add your demonstration video link here.

## 👨‍💻 Developer

**Nishant Tiwari**

Electronics and Telecommunication Engineering  
Anantrao Pawar College of Engineering (APCOER), Pune

## 📌 Project Status
Completed – Real-time 3D visualization and wireless sensor monitoring demonstrated.

---

⭐ If you find this project interesting, consider giving this repository a star!
