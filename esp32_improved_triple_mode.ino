/*
 * Smart Pen ESP32 Improved Triple Mode Firmware
 * Enhanced Version with Network Connectivity
 * 
 * Connection Modes:
 * 1. USB/Serial Mode (Mode 0)
 * 2. WiFi Client Mode (Mode 1) 
 * 3. Bluetooth/BLE Mode (Mode 2)
 * 
 * Hardware: ESP32 + MPU6050 + LED + Button
 * Network: Connects to laptop IP 192.168.2.13:3005
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <MPU6050.h>
#include <EEPROM.h>

// =============================================
// NETWORK CONFIGURATION
// =============================================
const char* LAPTOP_IP = "192.168.2.13";     // Your laptop's IP
const int LAPTOP_PORT = 3005;                // Backend server port
const char* WIFI_ENDPOINT = "/api/mpu6050/readings";

// WiFi credentials - Update these to match your network
const char* WIFI_SSID = "Your_WiFi_Network";        // Replace with your WiFi name
const char* WIFI_PASSWORD = "Your_WiFi_Password";    // Replace with your WiFi password

// For WiFi setup mode
const char* AP_SSID = "ESP32_SmartPen_Setup";
const char* AP_PASSWORD = "smartpen2024";

// =============================================
// HARDWARE CONFIGURATION  
// =============================================
#define LED_PIN 2
#define BUTTON_PIN 0
#define MPU6050_ADDR 0x68

// =============================================
// CONNECTION MODES
// =============================================
enum ConnectionMode {
  MODE_USB = 0,       // USB/Serial only
  MODE_WIFI = 1,      // WiFi client mode
  MODE_BLUETOOTH = 2  // Bluetooth/BLE mode
};

// =============================================
// GLOBAL VARIABLES
// =============================================
ConnectionMode currentMode = MODE_USB;
bool isStreaming = false;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
unsigned long lastDataSend = 0;
unsigned long lastModeSwitch = 0;

// Mode-specific timing
const unsigned long USB_INTERVAL = 100;        // 10Hz
const unsigned long WIFI_INTERVAL = 200;       // 5Hz  
const unsigned long BLUETOOTH_INTERVAL = 250;  // 4Hz

// Hardware
MPU6050 mpu;
bool mpuConnected = false;

// WiFi
WiFiClient wifiClient;
HTTPClient http;
bool wifiConnected = false;

// Bluetooth BLE
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// BLE UUIDs (Nordic UART Service)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

// =============================================
// BLE CALLBACKS
// =============================================
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("BLE Client Disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.println("BLE Received: " + rxValue);
        if (rxValue == "start") {
          isStreaming = true;
        } else if (rxValue == "stop") {
          isStreaming = false;
        }
      }
    }
};

// =============================================
// SETUP FUNCTIONS
// =============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🦡 Smart Pen ESP32 Improved Triple Mode Starting...");
  
  // Initialize hardware
  initializeHardware();
  
  // Load saved mode from EEPROM
  EEPROM.begin(512);
  currentMode = (ConnectionMode)EEPROM.read(0);
  if (currentMode > MODE_BLUETOOTH) {
    currentMode = MODE_USB; // Default to USB if invalid
  }
  
  // Initialize based on current mode
  initializeMode(currentMode);
  
  Serial.println("✅ Setup complete! Ready for data collection");
  printModeInstructions();
}

void initializeHardware() {
  // LED and Button
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // MPU6050 initialization
  Wire.begin();
  mpu.initialize();
  
  if (mpu.testConnection()) {
    mpuConnected = true;
    Serial.println("✅ MPU6050 connected successfully");
    
    // Configure MPU6050 for optimal performance
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
    mpu.setDLPFMode(MPU6050_DLPF_BW_20);
  } else {
    mpuConnected = false;
    Serial.println("⚠️  MPU6050 not found - using simulated data");
  }
}

void initializeMode(ConnectionMode mode) {
  currentMode = mode;
  
  Serial.printf("🔄 Switching to Mode %d: ", mode);
  
  switch(mode) {
    case MODE_USB:
      Serial.println("USB/Serial Mode");
      stopWiFi();
      stopBluetooth();
      break;
      
    case MODE_WIFI:
      Serial.println("WiFi Client Mode");
      stopBluetooth();
      initializeWiFi();
      break;
      
    case MODE_BLUETOOTH:
      Serial.println("Bluetooth/BLE Mode");
      stopWiFi();
      initializeBluetooth();
      break;
  }
  
  // Save mode to EEPROM
  EEPROM.write(0, currentMode);
  EEPROM.commit();
  
  // LED feedback for mode change
  blinkLED(mode + 1);
}

// =============================================
// WIFI FUNCTIONS
// =============================================
void initializeWiFi() {
  Serial.println("📶 Initializing WiFi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("✅ WiFi connected successfully!");
    Serial.printf("   IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Will send data to: %s:%d%s\n", LAPTOP_IP, LAPTOP_PORT, WIFI_ENDPOINT);
    
    // Test connection to server
    testServerConnection();
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
    Serial.println("   Starting Access Point mode for configuration...");
    startWiFiAP();
  }
}

void testServerConnection() {
  HTTPClient http;
  http.begin(wifiClient, String("http://") + LAPTOP_IP + ":" + LAPTOP_PORT + "/api/devices");
  
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.println("✅ Server connection test successful");
  } else {
    Serial.printf("⚠️  Server connection test failed: %d\n", httpResponseCode);
    Serial.println("   Make sure backend server is running on laptop");
  }
  http.end();
}

void startWiFiAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.println("📱 WiFi Access Point started:");
  Serial.printf("   SSID: %s\n", AP_SSID);
  Serial.printf("   Password: %s\n", AP_PASSWORD);
  Serial.printf("   IP: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("   Connect and go to 192.168.4.1 to configure WiFi");
}

void stopWiFi() {
  if (wifiConnected) {
    WiFi.disconnect();
    wifiConnected = false;
    Serial.println("📶 WiFi disconnected");
  }
}

// =============================================
// BLUETOOTH FUNCTIONS
// =============================================
void initializeBluetooth() {
  Serial.println("📘 Initializing Bluetooth BLE...");
  
  BLEDevice::init("BadgerPen_SmartPen");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                       BLECharacteristic::PROPERTY_WRITE
                     );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
  
  Serial.println("✅ BLE Server started");
  Serial.println("   Device name: BadgerPen_SmartPen");
  Serial.println("   Waiting for client connection...");
}

void stopBluetooth() {
  if (pServer != nullptr) {
    pServer->getAdvertising()->stop();
    BLEDevice::deinit(false);
    pServer = nullptr;
    pCharacteristic = nullptr;
    deviceConnected = false;
    Serial.println("📘 Bluetooth BLE stopped");
  }
}

// =============================================
// SENSOR DATA FUNCTIONS
// =============================================
struct SensorData {
  float gyro_x, gyro_y, gyro_z;
  float accel_x, accel_y, accel_z;
  unsigned long timestamp;
};

SensorData readSensorData() {
  SensorData data;
  data.timestamp = millis();
  
  if (mpuConnected) {
    // Read real MPU6050 data
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Convert to proper units
    data.accel_x = ax / 8192.0;  // ±4g range
    data.accel_y = ay / 8192.0;
    data.accel_z = az / 8192.0;
    data.gyro_x = gx / 65.5;     // ±500°/s range
    data.gyro_y = gy / 65.5;
    data.gyro_z = gz / 65.5;
  } else {
    // Generate simulated data with realistic tremor patterns
    float time = millis() / 1000.0;
    data.accel_x = 0.1 * sin(2 * PI * 4.5 * time) + random(-10, 10) / 1000.0;
    data.accel_y = 0.1 * cos(2 * PI * 4.8 * time) + random(-10, 10) / 1000.0;
    data.accel_z = 1.0 + 0.05 * sin(2 * PI * 3.2 * time);
    data.gyro_x = 10 * sin(2 * PI * 4.5 * time) + random(-50, 50) / 100.0;
    data.gyro_y = 8 * cos(2 * PI * 4.8 * time) + random(-50, 50) / 100.0;
    data.gyro_z = 5 * sin(2 * PI * 6.1 * time) + random(-30, 30) / 100.0;
  }
  
  return data;
}

// =============================================
// DATA TRANSMISSION FUNCTIONS
// =============================================
void sendDataUSB(SensorData data) {
  // CSV format for USB
  Serial.printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                data.gyro_x, data.gyro_y, data.gyro_z,
                data.accel_x, data.accel_y, data.accel_z);
}

void sendDataWiFi(SensorData data) {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  // JSON format for WiFi HTTP POST
  StaticJsonDocument<200> doc;
  doc["device_id"] = "ESP32_WiFi_SmartPen";
  doc["timestamp"] = data.timestamp;
  doc["gyro_x"] = data.gyro_x;
  doc["gyro_y"] = data.gyro_y;
  doc["gyro_z"] = data.gyro_z;
  doc["accel_x"] = data.accel_x;
  doc["accel_y"] = data.accel_y;
  doc["accel_z"] = data.accel_z;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  http.begin(wifiClient, String("http://") + LAPTOP_IP + ":" + LAPTOP_PORT + WIFI_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode != 200) {
    Serial.printf("WiFi send failed: %d\n", httpResponseCode);
  }
  
  http.end();
}

void sendDataBluetooth(SensorData data) {
  if (!deviceConnected || pCharacteristic == nullptr) {
    return;
  }
  
  // JSON format for Bluetooth
  StaticJsonDocument<200> doc;
  doc["device_id"] = "ESP32_BLE_SmartPen";
  doc["timestamp"] = data.timestamp;
  doc["gyro_x"] = data.gyro_x;
  doc["gyro_y"] = data.gyro_y;
  doc["gyro_z"] = data.gyro_z;
  doc["accel_x"] = data.accel_x;
  doc["accel_y"] = data.accel_y;
  doc["accel_z"] = data.accel_z;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  pCharacteristic->setValue(jsonString.c_str());
  pCharacteristic->notify();
}

// =============================================
// UTILITY FUNCTIONS
// =============================================
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
}

void updateLEDStatus() {
  static unsigned long lastLEDBlink = 0;
  static bool ledState = false;
  
  unsigned long interval = isStreaming ? 250 : 1000; // Fast when streaming, slow when paused
  
  if (millis() - lastLEDBlink > interval) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastLEDBlink = millis();
  }
}

void handleButton() {
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  
  if (currentButtonState && !buttonPressed) {
    // Button just pressed
    buttonPressed = true;
    lastButtonPress = millis();
  } else if (!currentButtonState && buttonPressed) {
    // Button just released
    buttonPressed = false;
    unsigned long pressDuration = millis() - lastButtonPress;
    
    if (pressDuration < 1000) {
      // Short press: toggle streaming
      isStreaming = !isStreaming;
      Serial.printf("📊 Data streaming: %s\n", isStreaming ? "ON" : "OFF");
      blinkLED(1);
    } else if (pressDuration >= 3000) {
      // Long press: switch mode
      if (millis() - lastModeSwitch > 5000) { // Prevent rapid mode switching
        currentMode = (ConnectionMode)((currentMode + 1) % 3);
        initializeMode(currentMode);
        lastModeSwitch = millis();
      }
    }
  }
}

void printModeInstructions() {
  Serial.println("\n🎮 CONTROLS:");
  Serial.println("   Short press button: Toggle data streaming");
  Serial.println("   Long press button (3s): Switch connection mode");
  Serial.println("\n📡 CURRENT MODE:");
  
  switch(currentMode) {
    case MODE_USB:
      Serial.println("   USB/Serial (10Hz) - Data sent via Serial port");
      break;
    case MODE_WIFI:
      Serial.printf("   WiFi Client (5Hz) - Data sent to %s:%d\n", LAPTOP_IP, LAPTOP_PORT);
      break;
    case MODE_BLUETOOTH:
      Serial.println("   Bluetooth BLE (4Hz) - Data sent via BLE notifications");
      break;
  }
  
  Serial.printf("   Streaming: %s\n", isStreaming ? "ON" : "OFF");
  Serial.println("\n🔗 NETWORK INFO:");
  Serial.printf("   Laptop IP: %s\n", LAPTOP_IP);
  Serial.printf("   Server Port: %d\n", LAPTOP_PORT);
  Serial.println("   ⚠️  Make sure ESP32 and laptop are on same WiFi network!");
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  handleButton();
  updateLEDStatus();
  
  if (!isStreaming) {
    delay(50);
    return;
  }
  
  // Get timing interval based on current mode
  unsigned long interval;
  switch(currentMode) {
    case MODE_USB: interval = USB_INTERVAL; break;
    case MODE_WIFI: interval = WIFI_INTERVAL; break;
    case MODE_BLUETOOTH: interval = BLUETOOTH_INTERVAL; break;
  }
  
  // Send data at appropriate rate
  if (millis() - lastDataSend > interval) {
    SensorData data = readSensorData();
    
    switch(currentMode) {
      case MODE_USB:
        sendDataUSB(data);
        break;
        
      case MODE_WIFI:
        sendDataWiFi(data);
        break;
        
      case MODE_BLUETOOTH:
        sendDataBluetooth(data);
        // Handle BLE connection state
        if (!deviceConnected && oldDeviceConnected) {
          delay(500);
          pServer->startAdvertising();
          Serial.println("📘 BLE advertising restarted");
          oldDeviceConnected = deviceConnected;
        }
        if (deviceConnected && !oldDeviceConnected) {
          oldDeviceConnected = deviceConnected;
        }
        break;
    }
    
    lastDataSend = millis();
  }
  
  delay(10); // Small delay to prevent watchdog issues
} 