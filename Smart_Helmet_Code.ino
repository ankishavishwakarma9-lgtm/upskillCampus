/*
 * ============================================
 * SMART HELMET AND MONITORING FOR MINERS
 * WITH ENHANCED PROTECTION
 * ============================================
 * 
 * Department of Electrical and Electronics Engineering
 * Vidya Jyothi Institute of Technology (Autonomous)
 * 
 * Project Guide: Mrs. Haritha
 * 
 * This code implements an IoT-based smart helmet for mining workers
 * using ESP32 microcontroller with multiple sensors for real-time
 * safety monitoring.
 * 
 * Sensors:
 * - MQ135: Gas detection (toxic gases like CO2, ammonia, benzene)
 * - DHT11: Temperature monitoring
 * - IR Sensor: Helmet fall/removal detection
 * - Fire Sensor: Flame detection
 * 
 * Output:
 * - OLED Display (I2C): Local real-time data visualization
 * - Buzzer: Audible alerts for hazardous conditions
 * - Blynk Cloud: Remote monitoring and push notifications
 * ============================================
 */

// ============================================
// BLYNK CONFIGURATION
// ============================================
#define BLYNK_TEMPLATE_ID "TMPL3Nz4daUSb"
#define BLYNK_TEMPLATE_NAME "Smart helmet"
#define BLYNK_AUTH_TOKEN "Xo1gUJfelup-FvU8zbsiEj0jFeRcJNia"

#define BLYNK_PRINT Serial

// ============================================
// INCLUDED LIBRARIES
// ============================================
#include <WiFi.h>                    // WiFi connectivity
#include <BlynkSimpleEsp32.h>        // Blynk IoT platform integration
#include <Wire.h>                    // I2C communication
#include <U8g2lib.h>                 // OLED display library
#include <DHT.h>                     // DHT11 sensor library

// ============================================
// WIFI CREDENTIALS
// ============================================
char ssid[] = "Loraproject";
char pass[] = "Project@123";

// ============================================
// OLED DISPLAY (SH1106) CONFIGURATION
// ============================================
// Using SH1106 128x64 OLED display with I2C interface
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ============================================
// SENSOR PIN DEFINITIONS
// ============================================
// DHT11 Temperature Sensor
#define DHTPIN 4                    // Data pin connected to GPIO4
#define DHTTYPE DHT11               // Sensor type
DHT dht(DHTPIN, DHTTYPE);

// MQ135 Gas Sensor
#define MQ135_PIN 34                // Analog output on GPIO34

// IR Sensor (Helmet Fall Detection)
#define IR_PIN 27                   // Digital output on GPIO27

// Fire Sensor (Flame Detection)
#define FIRE_PIN 26                 // Digital output on GPIO26

// Buzzer
#define BUZZER 25                   // Buzzer control on GPIO25

// ============================================
// THRESHOLD VALUES
// ============================================
int gasThreshold = 1500;            // Hazardous gas threshold
float tempThreshold = 36.0;         // High temperature threshold (Celsius)

// ============================================
// TIMING AND ALERT VARIABLES
// ============================================
unsigned long lastGasAlert = 0;
unsigned long lastFireAlert = 0;
unsigned long lastHelmetAlert = 0;
unsigned long lastTempAlert = 0;
const long alertInterval = 10000;   // 10 seconds between alerts

BlynkTimer timer;                   // Timer for periodic sensor reading

// ============================================
// SETUP FUNCTION
// ============================================
void setup() {
    // Initialize Serial Monitor for debugging
    Serial.begin(115200);
    Serial.println("\n\n=== SMART HELMET SYSTEM STARTING ===");
    
    // Initialize OLED Display
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 20, "Smart Helmet");
    u8g2.drawStr(15, 40, "Initializing...");
    u8g2.sendBuffer();
    delay(1000);
    
    // Initialize DHT11 Sensor
    dht.begin();
    Serial.println("[OK] DHT11 initialized");
    
    // Configure Pin Modes
    pinMode(MQ135_PIN, INPUT);
    pinMode(IR_PIN, INPUT_PULLUP);
    pinMode(FIRE_PIN, INPUT_PULLUP);
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    Serial.println("[OK] Pins configured");
    
    // Connect to Blynk Cloud
    Serial.println("[INFO] Connecting to WiFi...");
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    
    // Set up timer to read sensors every 2 seconds
    timer.setInterval(2000L, sendSensorData);
    
    // Display ready message on OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 20, "Smart Helmet");
    u8g2.drawStr(5, 40, "Ready...");
    u8g2.sendBuffer();
    
    Serial.println("[OK] System ready!");
}

// ============================================
// MAIN LOOP FUNCTION
// ============================================
void loop() {
    // Run Blynk tasks
    Blynk.run();
    
    // Run timer tasks (sensor reading every 2 seconds)
    timer.run();
}

// ============================================
// SEND SENSOR DATA FUNCTION
// Called every 2 seconds by BlynkTimer
// ============================================
void sendSensorData() {
    // ----- READ SENSORS -----
    
    // Read Gas Sensor (Analog value 0-4095)
    int gasValue = analogRead(MQ135_PIN);
    
    // Read Temperature from DHT11
    float temperature = dht.readTemperature();
    
    // Read IR Sensor (LOW = Fall detected, HIGH = Normal)
    int helmetFall = digitalRead(IR_PIN);
    
    // Read Fire Sensor (LOW = Fire detected, HIGH = Normal)
    int fireStatus = digitalRead(FIRE_PIN);
    
    // Check if temperature reading is valid
    if (isnan(temperature)) {
        Serial.println("[ERROR] DHT11 read failed!");
        temperature = 0.0;
    }
    
    // ----- PROCESS ALERTS -----
    bool alert = false;
    unsigned long currentMillis = millis();
    
    // 1. Gas Alert
    if (gasValue > gasThreshold && (currentMillis - lastGasAlert > alertInterval)) {
        Blynk.logEvent("gas_alert", "Gas Level High!");
        lastGasAlert = currentMillis;
        alert = true;
        Serial.println("[ALERT] High Gas Level Detected!");
    }
    
    // 2. Fire Alert
    if (fireStatus == 0 && (currentMillis - lastFireAlert > alertInterval)) {
        Blynk.logEvent("fire_alert", "Fire Detected!");
        lastFireAlert = currentMillis;
        alert = true;
        Serial.println("[ALERT] Fire Detected!");
    }
    
    // 3. Helmet Fall Alert
    if (helmetFall == 0 && (currentMillis - lastHelmetAlert > alertInterval)) {
        Blynk.logEvent("helmet_fall", "Helmet Fallen!");
        lastHelmetAlert = currentMillis;
        alert = true;
        Serial.println("[ALERT] Helmet Fall Detected!");
    }
    
    // 4. High Temperature Alert
    if (temperature > tempThreshold && (currentMillis - lastTempAlert > alertInterval)) {
        Blynk.logEvent("temp_rise", "High Temperature!");
        lastTempAlert = currentMillis;
        alert = true;
        Serial.println("[ALERT] High Temperature Detected!");
    }
    
    // ----- CONTROL BUZZER -----
    digitalWrite(BUZZER, alert ? HIGH : LOW);
    
    // ----- UPDATE BLYNK VIRTUAL PINS -----
    Blynk.virtualWrite(V0, gasValue);      // Gas value to gauge
    Blynk.virtualWrite(V1, temperature);   // Temperature to gauge
    Blynk.virtualWrite(V2, fireStatus == 0 ? 1 : 0);  // Fire status to LED
    Blynk.virtualWrite(V3, helmetFall == 0 ? 1 : 0);  // Helmet status to LED
    
    // ----- UPDATE OLED DISPLAY -----
    updateOLED(gasValue, temperature, helmetFall, fireStatus, alert);
    
    // ----- DEBUG OUTPUT -----
    Serial.print("Gas: "); Serial.print(gasValue);
    Serial.print(" | Temp: "); Serial.print(temperature);
    Serial.print(" | Fire: "); Serial.print(fireStatus);
    Serial.print(" | Helmet: "); Serial.println(helmetFall);
}

// ============================================
// OLED DISPLAY UPDATE FUNCTION
// ============================================
void updateOLED(int gas, float temp, int helmet, int fire, bool alert) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
    // Line 1: GAS
    u8g2.setCursor(0, 12);
    u8g2.print("Gas:");
    u8g2.setCursor(50, 12);
    if (alert && gas > gasThreshold) {
        u8g2.print(" ALERT!");
    } else {
        u8g2.print(gas);
    }
    
    // Line 2: TEMPERATURE
    u8g2.setCursor(0, 28);
    u8g2.print("Temp:");
    u8g2.setCursor(50, 28);
    if (alert && temp > tempThreshold) {
        u8g2.print(" ALERT!");
    } else {
        u8g2.print(temp, 1);
        u8g2.print(" C");
    }
    
    // Line 3: FIRE STATUS
    u8g2.setCursor(0, 44);
    u8g2.print("Fire:");
    u8g2.setCursor(50, 44);
    if (fire == 0) {
        u8g2.print(" FIRE!");
    } else {
        u8g2.print("Safe");
    }
    
    // Line 4: HELMET STATUS
    u8g2.setCursor(0, 60);
    u8g2.print("Helmet:");
    u8g2.setCursor(70, 60);
    if (helmet == 0) {
        u8g2.print("FALL!");
    } else {
        u8g2.print("OK");
    }
    
    u8g2.sendBuffer();
}

// ============================================
// BLYNK CONNECTION EVENTS
// ============================================
BLYNK_CONNECTED() {
    Serial.println("[INFO] Connected to Blynk Cloud!");
}

BLYNK_DISCONNECTED() {
    Serial.println("[WARN] Disconnected from Blynk Cloud!");
}

// ============================================
// END OF CODE
// ============================================
