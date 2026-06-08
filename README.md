# 6-DOF Robot Arm Wireless Control System via Smart Glove
### Production-Grade Dual-ESP32 Real-Time Control Simulation (ESP-NOW Protocol)
---
## 1. System Architecture & Design Philosophy
This repository contains a decoupled, dual-core embedded control system designed to map human hand kinematics to a 6-Degree of Freedom (6-DOF) robotic arm. The architecture implements a **Master-Transmitter (Smart Glove)** and a **Slave-Receiver (Actuator Driver)** leveraging the connectionless **ESP-NOW** wireless protocol to achieve near-zero propagation delay (< 5ms).
### Core Constraints & Low-Level Design:
* **Protocol Efficiency:** Bypasses the heavy TCP/IP stack overhead by transmitting structured binary payloads directly over the 2.4GHz Link Layer (802.11 Vendor-Specific Action Frames).
* **Deterministic Timing:** Transmitter task cycle is strictly managed via non-blocking hardware-timer emulation (`millis()`) set at a deterministic interval of 20ms (50Hz update rate), optimal for human-machine interfacing without bus saturation.
* **Jitter Reduction:** Direct register-to-PWM mapping prevents servo twitching caused by software interrupt latency overheads.
---
## 2. Hardware Specifications & Peripherals Mapping
The simulation models an industrial-grade deployment using two **ESP32-WROOM-32** modules executing asynchronous routines.
```
[ MASTER GLOVE ]                                  [ SLAVE ROBOTIC ARM ]
+----------------------------+                     +-------------------------------+
|  MPU6050 (I2C: GPIO 21/22) |                     | Servo 0 (Base)   -> GPIO 13   |
|  Thumb Pot  -> GPIO 34     |   >>> ESP-NOW >>>   | Servo 1 (Shoulder)-> GPIO 12  |
|  Index Pot  -> GPIO 35     |    (2.4GHz Link)    | Servo 2 (Elbow)   -> GPIO 14  |
|  Middle Pot -> GPIO 32     |                     | Servo 3 (Pitch)   -> GPIO 27  |
|  Ring Pot   -> GPIO 33     |                     | Servo 4 (Roll)    -> GPIO 26  |
|  Pinky Pot  -> GPIO 39     |                     | Servo 5 (Gripper) -> GPIO 25  |
+----------------------------+                     +-------------------------------+
```
### I/O Peripheral Configuration Matrix
| Subsystem | Peripheral | Bus / Signal Type | Hardware Pin / Channel | Operational Range |
| :--- | :--- | :--- | :--- | :--- |
| **Master** | MPU6050 IMU | I2C (Standard 400kHz) | SDA: GPIO 21 \| SCL: GPIO 22 | ±250°/s \| ±2g |
| **Master** | 5x Analog Inputs | ADC1 (12-bit Resolution) | GPIO 34, 35, 32, 33, 39 | 0V - 3.3V (0 - 4095 raw) |
| **Slave** | 6x Actuators | PWM (LEDC Peripheral) | GPIO 13, 12, 14, 27, 26, 25 | 50Hz Frequency (500μs - 2500μs) |
---
## 3. Communication Protocol & Payload Data Structure
To ensure deterministic deserialization and absolute minimal payload size (12 bytes total), data is packed into a packed C-struct with no padding bytes.
```c
typedef struct __attribute__((packed)) {
    int16_t servoAngles[6]; // 6 channels * 2 bytes = 12 bytes payload
} control_packet_t;
```
---
## 4. Repository Structure
```
.
├── README.md                      # System Engineering Documentation
├── Master_Glove/                  # Transmitter Firmware Workspace
│   ├── sketch.ino                 # Core Inversion-of-Control Loop (Master Code)
│   ├── diagram.json               # Hardware Schematic Layout Profile
│   ├── libraries.txt              # Environment Managed Dependencies
│   ├── wokwi-project.txt          # Simulator Target Metadata
│   └── circuit_diagram.png        # Master Glove Wiring Diagram
└── Slave_Arm/                     # Receiver Firmware Workspace
    ├── sketch.ino                 # PWM Generation Engine (Slave Code)
    ├── diagram.json               # Actuator Network Schematic Layout Profile
    ├── libraries.txt              # Environment Managed Dependencies
    ├── wokwi-project.txt          # Simulator Target Metadata
    └── circuit_diagram.png        # Slave Arm Wiring Diagram
```

---

