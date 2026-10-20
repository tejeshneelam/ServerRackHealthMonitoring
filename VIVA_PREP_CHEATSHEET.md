# 🎓 AI-Powered Data Center Rack Health Monitor (E-CSGI / PI-SSET)
## 7th Semester IoT / CPS Capstone Viva & Review Preparation Guide (50 Marks)

---

### 📌 Quick Project Elevator Pitch (Say this in the first 30 seconds):
> *"Our project is an **AI-Powered Data Center Rack Health Monitor** designed with an **Autonomous Dual-Tier Architecture**. 
> At the **Edge tier**, an ESP32 microcontroller runs our **E-CSGI (Edge Causal Structure Graph Inference)** engine directly on-chip, providing sub-10ms autonomous emergency fan actuation and fault diagnosis with zero internet dependency. 
> At the **Gateway tier**, our **PI-SSET (Physics-Informed Sparse Spatial Edge-Digital Twin)** engine uses Radial Basis Function (RBF) interpolation to reconstruct a continuous 3D volumetric thermal heatmap in real-time on an interactive WebGL dashboard."*

---

### 🏆 Rubric-by-Rubric Pinpoint Defense (How to get full marks)

#### 1. Custom Library Implementation [5 Marks]
* **Professor's Question:** *"Did you just download third-party libraries from GitHub, or did you write custom sensor drivers?"*
* **Your Answer:**
  > *"We developed custom low-level drivers without third-party libraries:*
  > *1. **CustomDHT (`CustomDHT.h`):** Written directly using low-level microsecond pulse-timing (`delayMicroseconds()`) and 40-bit packet decoding (16-bit humidity, 16-bit temperature, 8-bit checksum) with timeout guards.*
  > *2. **SW-420 Dual-Edge ISR Driver:** Implemented an on-chip Hardware Interrupt Service Routine (`vibeISR()`) on GPIO 19 with a custom 25ms delta-edge window filter to reject LM393 comparator oscillation."*
* **Code to show:** `firmware/04_test_all_sensors/CustomDHT.h` and lines 38–53 of `04_test_all_sensors.ino`.

---

#### 2. Local Database Working (Flash / EEPROM Persistence) [5 Marks]
* **Professor's Question:** *"Where is your database? What happens if power is cut or the device reboots?"*
* **Your Answer:**
  > *"For true edge resilience, our local database runs on the ESP32's on-chip **Non-Volatile Flash Storage (NVS)** using `<Preferences.h>`. It logs:*
  > *1. `boot_count`: Number of times the rack system has booted/restarted.*
  > *2. `peak_temp`: The all-time highest temperature recorded across all 4 tiers.*
  > *3. `total_shocks`: Cumulative physical tampering/vibration shock events.*
  > *When power is interrupted, the data is preserved in SPI Flash. On reboot, the OLED and Serial print the persistent stats immediately."*
* **How to demonstrate:** Press the `EN` (Reset) button on the ESP32. Point to the OLED:
  `NVS Boot:#XX  Pk:YY.Y°C`.

---

#### 3. Edge Analytics / ML Pre-Processing [5 Marks]
* **Professor's Question:** *"What ML or analytics model are you using? Why not just use an If-Else rule or train a CNN?"*
* **Your Answer:**
  > *"We use **E-CSGI (Edge Causal Structure Graph Inference)**. Standard ML models only find correlation, which can create false alarms (e.g. assuming high temperature always means fire).*
  > *E-CSGI is a **Causal Directed Acyclic Graph (DAG)** that isolates root causes by cross-correlating 3 independent physical domains in real time:*
  > *- **Thermal:** $\Delta T = \max(T_1..T_4) - T_{\text{ambient}}$*
  > *- **Mechanical:** SW-420 impulse frequency*
  > *- **Chemical:** MQ-135 analog voltage*
  > *This allows us to distinguish between:*
  > *1. **Compute Overload Hotspot:** High Temp + Normal Vibe + Clean Gas (CPU workload spike).*
  > *2. **Cooling Fan Mechanical Failure:** High Temp + High Vibe + Clean Gas (Bearing friction).*
  > *3. **Smoldering Smoke Hazard:** Gas > 1000 ADC (Electrical fire).*
  > *4. **Chassis Tampering:** Normal Temp + High Vibe (Physical impact)."*

---

#### 4. Communication Pipeline [5 Marks]
* **Professor's Question:** *"How does data travel from sensors to the dashboard? Why not use Wi-Fi?"*
* **Your Answer:**
  > *"We implement an **Industrial IoT Edge Gateway Pipeline**:*
  > *1. The ESP32 streams structured JSON over **Hardware UART Serial at 115200 baud**.*
  > *2. Our Python gateway (`data_streamer.py`) continuously ingests this stream and maintains an atomic buffer (`latest_data.json`).*
  > *3. A lightweight HTTP server (`web_server.py` on port 8000) exposes a `/data` REST API to the WebGL dashboard.*
  > *In data centers, physical server racks act as metal Faraday cages where consumer Wi-Fi suffers heavy interference and packet loss. Direct serial-to-gateway communication provides 100% reliability with zero packet drops."*

---

#### 5. GUI / Dashboard Prototype [5 Marks]
* **Professor's Question:** *"What makes your dashboard unique compared to a standard graph?"*
* **Your Answer:**
  > *"Our dashboard features a **Physics-Informed 3D Digital Twin (PI-SSET)** built with WebGL and Plotly.js:*
  > *- Instead of just showing 4 numbers, it runs a **Radial Basis Function (RBF) spatial interpolation** that solves the continuous heat diffusion field ($\nabla^2 T = 0$) inside the 3D server rack.*
  > *- We implemented client-side `Plotly.react` with `uirevision` camera locking, which eliminates web page flickering and allows full 3D rotation while live telemetry updates at 2 Hz.*
  > *- Dynamic status cards visually reflect active causal nodes (🔴 HOTSPOT, 🔴 SHOCK, 🔴 HAZARD)."*

---

#### 6. Engineering & Hardware Debugging Challenges (Guaranteed 25/25 in Q&A!)
Professors love when students explain real engineering problems they encountered and solved:

1. **The LM393 Comparator Oscillation Problem (Vibration Sensor):**
   * *Problem:* The SW-420 module was rapidly incrementing counts and the DO-LED was glowing halfway.
   * *Physics:* The LM393 comparator has no built-in hysteresis. When the sensor is right on the threshold, analog thermal noise causes the output to oscillate at 20–50 kHz in the linear region.
   * *Our Solution:* 
     1. Adjusted the potentiometer 1/4 turn counter-clockwise away from the knife-edge threshold.
     2. Implemented an on-chip software filter rejecting any edge arriving $< 25\,\text{ms}$ after the previous edge (`deltaEdge < 25`).

2. **Breadboard Contact Resistance & Current Starvation:**
   * *Problem:* When plugging the 5V Relay and MQ-135 gas sensor into the breadboard, the relay LED flickered and MQ-135 gave 0V.
   * *Physics:* The MQ-135 internal heating element draws ~160mA and the relay coil draws ~80mA. Breadboard spring clips have contact resistance that causes a voltage drop ($\Delta V = I \cdot R$) under load, starving the sensors.
   * *Our Solution:* Ran dedicated female-to-male jumper wires directly from the ESP32 `VIN` (5V) and `GND` pins to the high-current modules.

3. **Relay Terminal Chinese Markings:**
   * `常开` (Cháng kāi) = **NO (Normally Open)** $\rightarrow$ Connected to Fan (+) wire.
   * `公共端` (Gōnggòng duān) = **COM (Common)** $\rightarrow$ Connected to 5V power feed.
   * `常闭` (Cháng bì) = **NC (Normally Closed)** $\rightarrow$ Left empty.

---
*Created for 7th Semester IoT Review. Keep this guide open on your phone or laptop during preparation!*
