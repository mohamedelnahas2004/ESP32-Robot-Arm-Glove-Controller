/**
 * @file Slave_Receiver_Direct.ino
 * @brief 6-DOF Robot Arm Direct Actuator Driver (Receiver)
 * @note Architecture: Direct Multi-Channel PWM Generation via ESP32 LEDC
 * @author Mohamed Abdel-Aal Mohamed
 * @version 3.0
 * @date 2026-06-02
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h> // Professional library for direct ESP32 servo control

// Create 6 Servo Objects for the 6-DOF Robot Arm
Servo servo0; // Base Rotation
Servo servo1; // Joint 2
Servo servo2; // Joint 3
Servo servo3; // Joint 4
Servo servo4; // Joint 5
Servo servo5; // Gripper

// Hardware Pin Configuration (Directly on ESP32)
constexpr int SERVO_PIN_0 = 13;
constexpr int SERVO_PIN_1 = 12;
constexpr int SERVO_PIN_2 = 14;
constexpr int SERVO_PIN_3 = 27;
constexpr int SERVO_PIN_4 = 26;
constexpr int SERVO_PIN_5 = 25;

// Packet Blueprint Structure (Must match Transmitter exactly)
typedef struct struct_message {
    int16_t servoAngles[6];
} struct_message;

struct_message receivedPacket;

/**
 * @brief Interrupt Service Routine (ISR) Callback triggered whenever a packet arrives
 */
void onDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Memory copy raw buffer bytes directly into structural framework
    memcpy(&receivedPacket, incomingData, sizeof(receivedPacket));
    
    // Industrial Actuation Sequence: Update all 6 joint positions directly
    servo0.write(constrain(receivedPacket.servoAngles[0], 0, 180));
    servo1.write(constrain(receivedPacket.servoAngles[1], 0, 180));
    servo2.write(constrain(receivedPacket.servoAngles[2], 0, 180));
    servo3.write(constrain(receivedPacket.servoAngles[3], 0, 180));
    servo4.write(constrain(receivedPacket.servoAngles[4], 0, 180));
    servo5.write(constrain(receivedPacket.servoAngles[5], 0, 180));
}

void setup() {
    Serial.begin(115200);
    
    // Allocate Hardware PWM timers and attach pins to Servo objects
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    servo0.attach(SERVO_PIN_0, 500, 2400);
    servo1.attach(SERVO_PIN_1, 500, 2400);
    servo2.attach(SERVO_PIN_2, 500, 2400);
    servo3.attach(SERVO_PIN_3, 500, 2400);
    servo4.attach(SERVO_PIN_4, 500, 2400);
    servo5.attach(SERVO_PIN_5, 500, 2400);

    // Put Wireless chip into Station Mode
    WiFi.mode(WIFI_STA);

    // Initialize ESP-NOW Protocol Layer
    if (esp_now_init() != ESP_OK) {
        Serial.println("[CRITICAL] ESP-NOW Core initialization failed!");
        return;
    }
    
    // Bind Packet Receive Event Handler
    esp_now_register_recv_cb(esp_now_recv_cb_t(onDataReceived));
}

void loop() {
    // Empty Loop: Asynchronous architecture utilizes hardware-level interrupts.
}