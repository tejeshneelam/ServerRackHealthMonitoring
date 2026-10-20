# ⚡ AI-Powered Cyber-Physical Data Center Server Rack Health & Predictive Management System

[![Edge ML](https://img.shields.io/badge/Edge%20ML-Isolation%20Forest%20%7C%20Kalman%20%7C%20Taylor-blueviolet?style=for-the-badge&logo=python)](https://github.com/tejeshneelam/ServerRackHealthMonitoring)
[![ESP32](https://img.shields.io/badge/Microcontroller-ESP32%20Dual--Core%20240MHz-red?style=for-the-badge&logo=espressif)](https://github.com/tejeshneelam/ServerRackHealthMonitoring)
[![Dashboard](https://img.shields.io/badge/Real--Time-3D%20Digital%20Twin%20Web-success?style=for-the-badge&logo=html5)](https://github.com/tejeshneelam/ServerRackHealthMonitoring)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](LICENSE)

An enterprise-grade **Cyber-Physical Edge AI & Reliability Physics Platform** designed to monitor, forecast, and protect high-density server rack environments. 

Rather than relying on naive, reactive thresholding (`if temp > 31.5°C`), this system employs an **8-Pillar Machine Learning & Physics-of-Failure architecture** that combines **continuous kinematic state estimation**, **multi-step predictive horizon forecasting**, **high-dimensional unsupervised anomaly detection**, and **semiconductor physics degradation modeling**.

---

## 🎯 The Real-World Problem Solved

In high-performance data centers, basic IoT monitoring approaches fail catastrophically for three reasons:
1. **Thermal Inertia & Severe Overshoot:** Heavy copper heat pipes and aluminum heatsinks store massive amounts of heat. Turning on a fan only when hitting a trip limit causes the temperature to overshoot by $3^\circ\text{C}-5^\circ\text{C}$ before cooling takes effect, accelerating silicon wear.
2. **Sensor ADC Quantization Noise:** Digital probes jitter by $\pm 0.2^\circ\text{C}$. Numerically subtracting raw readings produces false spikes in rate-of-rise calculations, causing cooling relays to rapidly chatter.
3. **Multivariate Blindness:** Real component failures are coupled. A dying fan bearing emits microscopic vibrations long before it generates heat; a smoldering cable produces toxic gases before visible flames appear.

---

## 🏛️ System Architecture

```
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      ESP32 EMBEDDED EDGE SENSING                       │
 │  • 4x DS18B20 Probes (GPIO 4)      • SW-420 Shock/Vibration (GPIO 19)   │
 │  • MQ-2 Combustible Gas (GPIO 34)  • DHT11 Ambient Reference (GPIO 15)  │
 │  • SSD1306 OLED Display (I2C)      • 5V Relay Actuator (GPIO 18)        │
 │  • Red/Green Status LEDs (G25/26)  • Active Warning Buzzer (GPIO 27)    │
 │  • NVS Flash Black-Box Storage     • 2-Second Sliding Energy Window     │
 └───────────────────────────────────┬────────────────────────────────────┘
                                     │ 1 Hz JSON Telemetry (115200 Baud)
                                     ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      PYTHON CYBER-PHYSICAL ML ENGINE                    │
 │                                                                        │
 │  1. 3-State Kinematic Kalman Filter (T, dT/dt, d²T/dt²)               │
 │  2. 2nd-Order Taylor Thermal Horizon Forecasting (t+15s to t+60s)       │
 │  3. Remaining Useful Time to Trip (RUTT) Real-Time Countdown           │
 │  4. Multivariate Anomaly Isolation (Forest N=80 + Mahalanobis χ²_7)    │
 │  5. Real-Time Explainable AI (SHAP-Style Feature Attribution)           │
 │  6. Arrhenius Semiconductor Silicon Aging & Component RUL (Days)       │
 │  7. Airflow Enthalpy Heat Removal & Green PUE Ratio Accounting        │
 └───────────────────────────────────┬────────────────────────────────────┘
                                     │ WebSocket / REST API
                                     ▼
 ┌────────────────────────────────────────────────────────────────────────┐
 │                      INTERACTIVE WEB DASHBOARD                         │
 │  • Real-Time Cyber-Physical Edge AI Intelligence Center (6 Top Cards)  │
 │  • PI-SSET 3D Thermal Digital Twin (Stratification Heatmap Wireframe)  │
 │  • Real-Time RUTT Countdown & Thermal Horizon Uncertainty Cones        │
 │  • E-CSGI Root-Cause Fault Attribution (SHAP %)                        │
 └────────────────────────────────────────────────────────────────────────┘
```

---

## 🧠 The 8 Cyber-Physical Pillars

### 1. 3-State Kinematic Kalman Filter
* **State Vector:** $x = [T, \frac{dT}{dt}, \frac{d^2T}{dt^2}]^T$ (True Temperature, Thermal Velocity, and Thermal Acceleration).
* **Benefit:** Extracts zero-delay thermal velocity without noise spikes ($R=0.04$, matching sensor $\pm 0.2^\circ\text{C}$ noise variance), preventing false relay trigger and chatter.

### 2. 2nd-Order Taylor Horizon Forecaster
* Projects thermal state into the future:
  $$T(t + \Delta t) = T_0 + v\cdot \Delta t + \frac{1}{2} a \cdot \Delta t^2 \cdot \eta$$
  *(where $\eta = 0.88$ represents convective atmospheric cooling damping).*
* Generates 95% confidence cones over $+15\text{s}$, $+30\text{s}$, $+45\text{s}$, and $+60\text{s}$ horizons.

### 3. Remaining Useful Time to Trip (RUTT)
* An active emergency countdown clock:
  $$\text{RUTT} = \frac{T_{\text{trip}} - T_{\text{current}}}{v_{\text{kalman}}}$$
* Gives operators seconds-level warning before safety threshold breach occurs.

### 4. Multivariate Anomaly Detection
* Combines **Isolation Forest ($N=80$ trees)** with **Mahalanobis Distance** across a 7-dimensional physical feature space (vertical gradient, thermal velocity, thermal acceleration, core hotspot, ambient delta, gas rise velocity, and vibration shock energy).

### 5. Real-Time Explainable AI (SHAP Attribution)
* Decomposes detected anomalies into exact percentage root causes (e.g. `Gas Velocity: 82% | Mechanical Vibration: 14% | Thermal: 4%`), eliminating operator guesswork.

### 6. Arrhenius Semiconductor Silicon Aging & RUL
* Models electromigration failure physics in microchips:
  $$\text{Acceleration Factor (AF)} = \exp\left[ \frac{E_a}{k_B} \left( \frac{1}{T_{\text{nominal}}} - \frac{1}{T_{\text{core}}} \right) \right]$$
* Projects Component Remaining Useful Life (RUL) in days from a 5-year MTTF baseline.

### 7. Thermodynamics & Green PUE Accounting
* Measures airflow enthalpy heat removal ($Q = \dot{m} C_p \Delta T$) and live Power Usage Effectiveness ($\text{PUE} = \frac{P_{\text{IT}} + P_{\text{Cooling}}}{P_{\text{IT}}}$).

### 8. Edge Filtering & Sliding Energy Windows
* ESP32 firmware executes a 2-second sliding energy window on mechanical vibration to differentiate accidental physical bumps from destructive motor/bearing resonance ($\ge 4$ pulses).

---

## 🔌 Hardware Pin Mapping

| Peripheral / Sensor | ESP32 Pin | Interface | Operational Function |
| :--- | :--- | :--- | :--- |
| **4x DS18B20 Probes** | GPIO 4 | 1-Wire (4.7kΩ Pullup) | Vertical rack stratification ($T_1$ bottom to $T_4$ top) |
| **MQ-2 Gas / Smoke** | GPIO 34 | Analog ADC1 | Toxic smoke & combustible hydrocarbon detection |
| **SW-420 Piezo Shock** | GPIO 19 | Digital Interrupt | Impeller vibration & motor bearing fatigue |
| **DHT11 Environment** | GPIO 15 | Digital Single-Bus | Ambient intake temperature & humidity baseline |
| **5V Relay Module** | GPIO 18 | Optocoupled Digital | Actuation line for brushless 5V DC cooling fan |
| **SSD1306 OLED (128x64)** | GPIO 21 (SDA), 22 (SCL) | I2C | Edge status, IP display, and telemetry rotation |
| **Warning Buzzer** | GPIO 27 | PWM / Digital Active | High-priority edge auditory alert |
| **Status LEDs** | GPIO 26 (Grn), 25 (Red) | Digital Output | Green (Nominal) / Red (Critical Trip Alarm) |
| **NVS Flash Storage** | Internal Flash | Non-Volatile Memory | Power-loss proof boot count, lifetime shocks & peak temp |

---

## 🚀 Quick Start Guide

### Prerequisites
* Python 3.9+
* Arduino IDE (with ESP32 board definitions installed)
* Connected ESP32 via USB Serial (`/dev/cu.usbserial-0001` or `COMx`)

### 1. Flash the ESP32 Firmware
1. Open [`firmware/04_test_all_sensors/04_test_all_sensors.ino`](firmware/04_test_all_sensors/04_test_all_sensors.ino) in Arduino IDE.
2. Ensure required libraries are installed:
   * `OneWire` & `DallasTemperature`
   * `Adafruit SSD1306` & `Adafruit GFX`
   * `Preferences` (built-in ESP32 NVS)
3. Select board **ESP32 Dev Module** and upload.

### 2. Run the Python Edge ML Backend
```bash
# Navigate to the Python backend directory
cd python_backend

# Install required scientific packages
pip install numpy scikit-learn pyserial

# Start the web server and ML processing engine
python3 web_server.py
```

### 3. Open the Dashboard
Navigate to `http://localhost:8000` in any web browser to view the live Cyber-Physical AI Monitoring Center and 3D Digital Twin.

---

## 👥 Project Team & Work Division

| Team Member | Core Specialization & Defense Domain |
| :--- | :--- |
| **Tejesh Neelam** | Embedded Hardware, ADC Noise Rejection & 3-State Kinematic Kalman Filter |
| **Vasishta** | Predictive Horizon Control, 2nd-Order Taylor Forecaster & RUTT Countdown |
| **Girish** | Multivariate Machine Learning, Isolation Forest ($N=80$) & SHAP Attribution |
| **Nandakishore** | Reliability Physics, Arrhenius Silicon Aging RUL, PUE & Green Computing |

---

## 📜 License
This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
