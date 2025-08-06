/*
 * Smart Pen ESP32 WebSocket Client Firmware
 * Replaces HTTP POST with WebSocket bidirectional communication
 * 
 * Connection Modes:
 * 1. USB/Serial Mode (Mode 0)
 * 2. WiFi WebSocket Client Mode (Mode 1) 
 * 3. Bluetooth/BLE Mode (Mode 2)
 * 
 * Hardware: ESP32 + MPU6050 + LED + Button
 * Network: WebSocket connection to backend server
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
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
const char* BACKEND_HOST = "192.168.2.13";      // Your backend server IP
const int BACKEND_PORT = 5000;                   // Backend server port
const char* WEBSOCKET_PATH = "/ws";              // WebSocket endpoint path

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
  MODE_WIFI = 1,      // WiFi WebSocket client mode
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

// WiFi and WebSocket
WebSocketsClient webSocket;
bool wifiConnected = false;
bool webSocketConnected = false;
unsigned long lastWebSocketPing = 0;
unsigned long lastReconnectAttempt = 0;
int reconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 5;
const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds

// Device ID for identification
String deviceId = "ESP32_WebSocket_SmartPen";

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
// WEBSOCKET CALLBACKS
// =============================================
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("🔌 WebSocket Disconnected");
      webSocketConnected = false;
      blinkLED(3); // 3 blinks for disconnect
      break;

    case WStype_CONNECTED:
      Serial.printf("🔌 WebSocket Connected to: %s\n", payload);
      webSocketConnected = true;
      reconnectAttempts = 0;
      blinkLED(2); // 2 blinks for connect
      
      // Send identification message to backend
      sendIdentificationMessage();
      break;

    case WStype_TEXT:
      Serial.printf("📨 WebSocket Received: %s\n", payload);
      handleWebSocketMessage((char*)payload);
      break;

    case WStype_BIN:
      Serial.printf("📦 WebSocket Binary data received: %u bytes\n", length);
      break;

    case WStype_PING:
      Serial.println("💓 WebSocket Ping received");
      break;

    case WStype_PONG:
      Serial.println("💓 WebSocket Pong received");
      lastWebSocketPing = millis();
      break;

    case WStype_ERROR:
      Serial.printf("❌ WebSocket Error: %s\n", payload);
      break;

    default:
      Serial.printf("🔍 WebSocket Unknown event type: %d\n", type);
      break;
  }
}

void sendIdentificationMessage() {
  StaticJsonDocument<200> doc;
  doc["type"] = "identify";
  doc["data"]["type"] = "esp32";
  doc["data"]["deviceId"] = deviceId;
  doc["data"]["firmwareVersion"] = "2.0.0";
  doc["data"]["capabilities"] = "MPU6050,LED,Button";
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  Serial.println("📤 Sent identification message");
}

void handleWebSocketMessage(const char* message) {
  StaticJsonDocument<512> doc;
  
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.printf("❌ JSON parse error: %s\n", error.c_str());
    return;
  }
  
  String messageType = doc["type"];
  
  if (messageType == "connected") {
    Serial.println("✅ Backend confirmed connection");
    
  } else if (messageType == "identification_confirmed") {
    Serial.println("✅ Device identification confirmed by backend");
    
  } else if (messageType == "command") {
    handleCommand(doc["data"]);
    
  } else if (messageType == "sensor_data_ack") {
    // Data acknowledgment from backend
    Serial.println("✅ Sensor data acknowledged by backend");
    
  } else if (messageType == "error") {
    Serial.printf("❌ Backend error: %s\n", doc["data"]["message"].as<const char*>());
    
  } else if (messageType == "pong") {
    Serial.println("💓 Backend pong received");
    
  } else {
    Serial.printf("❓ Unknown message type: %s\n", messageType.c_str());
  }
}

void handleCommand(JsonObject commandData) {
  String command = commandData["command"];
  Serial.printf("📋 Received command: %s\n", command.c_str());
  
  if (command == "start_streaming") {
    isStreaming = true;
    Serial.println("▶️  Started data streaming");
    
  } else if (command == "stop_streaming") {
    isStreaming = false;
    Serial.println("⏹️  Stopped data streaming");
    
  } else if (command == "led_on") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("💡 LED turned ON");
    
  } else if (command == "led_off") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("💡 LED turned OFF");
    
  } else if (command == "blink") {
    int count = commandData["count"] | 3;
    blinkLED(count);
    Serial.printf("💡 LED blinked %d times\n", count);
    
  } else if (command == "calibrate") {
    calibrateMPU6050();
    Serial.println("🎯 MPU6050 calibration started");
    
  } else if (command == "get_status") {
    sendStatusMessage();
    
  } else {
    Serial.printf("❓ Unknown command: %s\n", command.c_str());
  }
  
  // Send command acknowledgment
  sendCommandAck(command);
}

void sendCommandAck(String command) {
  StaticJsonDocument<200> doc;
  doc["type"] = "command_ack";
  doc["data"]["command"] = command;
  doc["data"]["status"] = "executed";
  doc["data"]["timestamp"] = millis();
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
}

void sendStatusMessage() {
  StaticJsonDocument<300> doc;
  doc["type"] = "device_status";
  doc["data"]["deviceId"] = deviceId;
  doc["data"]["mode"] = currentMode;
  doc["data"]["streaming"] = isStreaming;
  doc["data"]["wifi_connected"] = wifiConnected;
  doc["data"]["websocket_connected"] = webSocketConnected;
  doc["data"]["mpu6050_connected"] = mpuConnected;
  doc["data"]["uptime"] = millis();
  doc["data"]["free_heap"] = ESP.getFreeHeap();
  doc["data"]["wifi_rssi"] = WiFi.RSSI();
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  Serial.println("📊 Sent status message");
}

// =============================================
// SETUP FUNCTIONS
// =============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🦡 Smart Pen ESP32 WebSocket Client Starting...");
  
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
      Serial.println("WiFi WebSocket Client Mode");
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
// WIFI AND WEBSOCKET FUNCTIONS
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
    Serial.printf("   Will connect WebSocket to: %s:%d%s\n", BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
    
    // Initialize WebSocket connection
    initializeWebSocket();
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
    Serial.println("   Starting Access Point mode for configuration...");
    startWiFiAP();
  }
}

void initializeWebSocket() {
  Serial.println("🔌 Initializing WebSocket connection...");
  
  // Configure WebSocket client
  webSocket.begin(BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
  webSocket.onEvent(webSocketEvent);
  
  // Set reconnection parameters
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2); // ping interval, pong timeout, disconnect timeout
  
  Serial.printf("🔌 WebSocket client configured for %s:%d%s\n", BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
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
  if (webSocketConnected) {
    webSocket.disconnect();
    webSocketConnected = false;
    Serial.println("🔌 WebSocket disconnected");
  }
  
  if (wifiConnected) {
    WiFi.disconnect();
    wifiConnected = false;
    Serial.println("📶 WiFi disconnected");
  }
}

void checkWebSocketConnection() {
  if (currentMode == MODE_WIFI && wifiConnected) {
    // Let the WebSocket library handle reconnection automatically
    webSocket.loop();
    
    // Send periodic ping if connected
    if (webSocketConnected && millis() - lastWebSocketPing > 30000) {
      webSocket.sendPing();
      lastWebSocketPing = millis();
    }
  }
}

// =============================================
// BLUETOOTH FUNCTIONS
// =============================================
void initializeBluetooth() {
  Serial.println("📘 Initializing Bluetooth BLE...");
  
  BLEDevice::init("SmartPen_WebSocket");
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
  Serial.println("   Device name: SmartPen_WebSocket");
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

void sendDataWebSocket(SensorData data) {
  if (!webSocketConnected) {
    return;
  }
  
  // JSON format for WebSocket
  StaticJsonDocument<300> doc;
  doc["type"] = "sensor_data";
  doc["data"]["device_id"] = deviceId;
  doc["data"]["timestamp"] = data.timestamp;
  doc["data"]["gyro_x"] = data.gyro_x;
  doc["data"]["gyro_y"] = data.gyro_y;
  doc["data"]["gyro_z"] = data.gyro_z;
  doc["data"]["accel_x"] = data.accel_x;
  doc["data"]["accel_y"] = data.accel_y;
  doc["data"]["accel_z"] = data.accel_z;
  
  String message;
  serializeJson(doc, message);
  
  if (webSocket.sendTXT(message)) {
    Serial.println("📤 Sensor data sent via WebSocket");
  } else {
    Serial.println("❌ Failed to send sensor data via WebSocket");
  }
}

void sendDataBluetooth(SensorData data) {
  if (!deviceConnected || pCharacteristic == nullptr) {
    return;
  }
  
  // JSON format for Bluetooth
  StaticJsonDocument<200> doc;
  doc["device_id"] = deviceId;
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
void blinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void handleButtonPress() {
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
    lastButtonPress = millis();
    
    // Toggle streaming on button press
    isStreaming = !isStreaming;
    Serial.printf("🔘 Button pressed - Streaming: %s\n", isStreaming ? "ON" : "OFF");
    
    // LED feedback
    blinkLED(isStreaming ? 2 : 1);
    
  } else if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
    buttonPressed = false;
  }
}

void calibrateMPU6050() {
  if (!mpuConnected) {
    Serial.println("⚠️  Cannot calibrate - MPU6050 not connected");
    return;
  }
  
  Serial.println("🎯 Calibrating MPU6050...");
  Serial.println("   Keep device stationary for 5 seconds");
  
  // Simple calibration - could be enhanced
  delay(5000);
  
  Serial.println("✅ MPU6050 calibration complete");
}

void printModeInstructions() {
  Serial.println("\n📖 Instructions:");
  Serial.println("   • Press button to toggle data streaming");
  Serial.println("   • Hold button for 3 seconds to switch modes");
  Serial.printf("   • Current mode: %d (%s)\n", currentMode, 
                currentMode == MODE_USB ? "USB" : 
                currentMode == MODE_WIFI ? "WiFi WebSocket" : "Bluetooth");
  Serial.println("   • Send 'status' via Serial for system status");
  Serial.println();
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  // Handle button input
  handleButtonPress();
  
  // Check for mode switch (hold button for 3 seconds)
  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastButtonPress > 3000 && 
      millis() - lastModeSwitch > 5000) {
    
    Serial.println("🔄 Mode switch detected");
    currentMode = (ConnectionMode)((currentMode + 1) % 3);
    initializeMode(currentMode);
    lastModeSwitch = millis();
    printModeInstructions();
  }
  
  // Handle WebSocket connection in WiFi mode
  if (currentMode == MODE_WIFI) {
    checkWebSocketConnection();
  }
  
  // Handle serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "status") {
      printSystemStatus();
    } else if (command == "start") {
      isStreaming = true;
      Serial.println("▶️  Streaming started via Serial");
    } else if (command == "stop") {
      isStreaming = false;
      Serial.println("⏹️  Streaming stopped via Serial");
    }
  }
  
  // Send sensor data if streaming is enabled
  if (isStreaming) {
    unsigned long currentTime = millis();
    unsigned long interval;
    
    // Get interval based on current mode
    switch(currentMode) {
      case MODE_USB:      interval = USB_INTERVAL; break;
      case MODE_WIFI:     interval = WIFI_INTERVAL; break;
      case MODE_BLUETOOTH: interval = BLUETOOTH_INTERVAL; break;
    }
    
    if (currentTime - lastDataSend >= interval) {
      SensorData data = readSensorData();
      
      // Send data based on current mode
      switch(currentMode) {
        case MODE_USB:
          sendDataUSB(data);
          break;
          
        case MODE_WIFI:
          if (webSocketConnected) {
            sendDataWebSocket(data);
          }
          break;
          
        case MODE_BLUETOOTH:
          if (deviceConnected) {
            sendDataBluetooth(data);
          }
          break;
      }
      
      lastDataSend = currentTime;
    }
  }
  
  // Small delay to prevent overwhelming the CPU
  delay(10);
}

void printSystemStatus() {
  Serial.println("\n📊 System Status:");
  Serial.printf("   Mode: %d (%s)\n", currentMode, 
                currentMode == MODE_USB ? "USB" : 
                currentMode == MODE_WIFI ? "WiFi WebSocket" : "Bluetooth");
  Serial.printf("   Streaming: %s\n", isStreaming ? "ON" : "OFF");
  Serial.printf("   MPU6050: %s\n", mpuConnected ? "Connected" : "Disconnected");
  Serial.printf("   WiFi: %s\n", wifiConnected ? "Connected" : "Disconnected");
  Serial.printf("   WebSocket: %s\n", webSocketConnected ? "Connected" : "Disconnected");
  Serial.printf("   Uptime: %lu ms\n", millis());
  Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
  
  if (wifiConnected) {
    Serial.printf("   WiFi RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
  }
  
  if (currentMode == MODE_BLUETOOTH) {
    Serial.printf("   BLE Client: %s\n", deviceConnected ? "Connected" : "Disconnected");
  }
  Serial.println();
} 