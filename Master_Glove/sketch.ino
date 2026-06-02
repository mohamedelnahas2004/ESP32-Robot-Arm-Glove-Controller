/**
 * @file Master_Transmitter_Lightweight.ino
 * @brief Optimized 6-DOF Smart Glove Controller (Transmitter)
 * @note Optimized for fast cloud compiling on Wokwi via Adafruit MPU6050
 * @author Mohamed Abdel-Aal Mohamed
 * @version 2.2
 * @date 2026-06-02
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Target Slave MAC Address
uint8_t slaveMacAddress[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

// Hardware Pin Configuration for 5-Finger Glove
constexpr int POT_THUMB  = 34; 
constexpr int POT_INDEX  = 35; 
constexpr int POT_MIDDLE = 32; 
constexpr int POT_RING   = 33; 
constexpr int POT_PINKY  = 39; 

// Structured Telemetry Packet
typedef struct struct_message {
    int16_t servoAngles[6]; 
} struct_message;

struct_message controlPacket;
esp_now_peer_info_t peerInfo;
Adafruit_MPU6050 mpu;

// Timing Constraints (50Hz refresh rate)
unsigned long lastTxTimestamp = 0;
constexpr unsigned long TX_INTERVAL = 20; 

void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
    // Diagnostic log thread
}

void setup() {
    Serial.begin(115200);
    
    // Initialize I2C Bus on standard ESP32 pins (SDA=21, SCL=22)
    Wire.begin(21, 22); 
    
    // Initialize Adafruit MPU6050 Hardware
    if (!mpu.begin()) {
        Serial.println("[CRITICAL] MPU6050 missing or connection failed!");
        while (1); 
    }

    // Configure Wi-Fi Interface
    WiFi.mode(WIFI_STA);

    // Initialize ESP-NOW Protocol Stack
    if (esp_now_init() != ESP_OK) {
        Serial.println("[CRITICAL] Failed to initialize ESP-NOW");
        return;
    }

    // Register Tx Status Handler
    esp_now_register_send_cb((esp_now_send_cb_t)onDataSent);
    
    // Configure Peer Registration
    memcpy(peerInfo.peer_addr, slaveMacAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("[CRITICAL] Peer registration failed");
        return;
    }
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastTxTimestamp >= TX_INTERVAL) {
        lastTxTimestamp = currentMillis;

        // 1. Fetch data using Adafruit Sensor API
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        
        // Map Accelerometer X-axis directly to Base Rotation Angle
        // a.acceleration.x returns values roughly between -10 and 10 m/s^2
        float x_accel = constrain(a.acceleration.x, -10.0, 10.0);
        controlPacket.servoAngles[0] = map(x_accel * 100, -1000, 1000, 0, 180); 

        // 2. Sample Analog Inputs from the 5 Glove Potentiometers
        controlPacket.servoAngles[1] = map(analogRead(POT_THUMB),  0, 4095, 0, 180); 
        controlPacket.servoAngles[2] = map(analogRead(POT_INDEX),  0, 4095, 0, 180); 
        controlPacket.servoAngles[3] = map(analogRead(POT_MIDDLE), 0, 4095, 0, 180); 
        controlPacket.servoAngles[4] = map(analogRead(POT_RING),   0, 4095, 0, 180); 
        controlPacket.servoAngles[5] = map(analogRead(POT_PINKY),  0, 4095, 0, 180); 

        // 3. Dispatch Packet
        esp_now_send(slaveMacAddress, (uint8_t *) &controlPacket, sizeof(controlPacket));
    }
}