/*
 * Smart Pen ESP32 WiFi WebSocket Client
 * 
 * This firmware connects your ESP32 to WiFi and sends sensor data
 * to the Smart Pen WebSocket backend server running on your computer.
 * 
 * Hardware: ESP32 + MPU6050 + LED + Button
 * Communication: WiFi WebSocket Client
 * 
 * Backend Server: http://192.168.61.204:3005
 * WebSocket Endpoint: ws://192.168.61.204:3005/ws
 * 
 * Required Libraries (Install via Arduino Library Manager):
 * - WebSocketsClient by Markus Sattler
 * - ArduinoJson by Benoit Blanchon  
 * - MPU6050 library (if using real sensor)
 * 
 * Setup Instructions:
 * 1. Update WiFi credentials below (WIFI_SSID and WIFI_PASSWORD)
 * 2. Verify backend IP address (BACKEND_HOST)
 * 3. Upload firmware to ESP32
 * 4. Open Serial Monitor (115200 baud) to see connection status
 * 5. Press button to toggle data streaming
 */

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

// =============================================
// NETWORK CONFIGURATION - UPDATE THESE VALUES
// =============================================
const char* WIFI_SSID = "Your_WiFi_Network";        // Replace with your WiFi name
const char* WIFI_PASSWORD = "Your_WiFi_Password";    // Replace with your WiFi password
const char* BACKEND_HOST = "192.168.61.204";        // Your computer's IP address
const int BACKEND_PORT = 3005;                      // Backend server port  
const char* WEBSOCKET_PATH = "/ws";                 // WebSocket endpoint path

// =============================================
// HARDWARE CONFIGURATION
// =============================================
#define LED_PIN 2           // Built-in LED pin
#define BUTTON_PIN 0        // Boot button pin
#define SDA_PIN 21          // I2C data pin
#define SCL_PIN 22          // I2C clock pin

// =============================================
// DEVICE CONFIGURATION
// =============================================
#define DEVICE_NAME "SmartPen_ESP32_WiFi"
#define FIRMWARE_VERSION "2.0.1"
#define DATA_SEND_INTERVAL 100        // Send data every 100ms (10Hz)
#define HEARTBEAT_INTERVAL 30000      // Send heartbeat every 30s
#define BUTTON_DEBOUNCE 500           // Button debounce time (ms)

// =============================================
// GLOBAL VARIABLES
// =============================================
WebSocketsClient webSocket;
String deviceId = String("ESP32_") + String((uint32_t)ESP.getEfuseMac(), HEX);

// Connection status
bool wifiConnected = false;
bool webSocketConnected = false;
bool mpuConnected = false;

// Data streaming control
bool isStreaming = false;
unsigned long lastDataSend = 0;
unsigned long lastHeartbeat = 0;

// Button handling
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;

// Sensor data (simulated if no real MPU6050)
struct SensorData {
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float temperature;
  unsigned long timestamp;
};

// Statistics
unsigned long totalMessagesSent = 0;
unsigned long totalMessagesReceived = 0;
unsigned long connectionStartTime = 0;

// =============================================
// SETUP FUNCTION
// =============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Print startup banner
  Serial.println();
  Serial.println("╔═══════════════════════════════════════════════════════╗");
  Serial.println("║          Smart Pen ESP32 WiFi WebSocket Client       ║");
  Serial.println("║                    Version 2.0.1                     ║");
  Serial.println("╚═══════════════════════════════════════════════════════╝");
  Serial.println();
  
  Serial.printf("Device ID: %s\n", deviceId.c_str());
  Serial.printf("Target Backend: %s:%d%s\n", BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
  Serial.println();
  
  // Initialize hardware
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize I2C for MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Check for MPU6050 sensor
  checkMPU6050();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize WebSocket if WiFi connected
  if (wifiConnected) {
    initializeWebSocket();
  }
  
  Serial.println("✅ Setup complete!");
  Serial.println("📝 Press the BOOT button to toggle data streaming");
  Serial.println("📋 Available serial commands: 'status', 'start', 'stop', 'restart'");
  Serial.println();
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  // Handle WebSocket events
  if (wifiConnected) {
    webSocket.loop();
  }
  
  // Check WiFi connection
  checkWiFiConnection();
  
  // Handle button press
  handleButton();
  
  // Handle serial commands
  handleSerialCommands();
  
  // Send sensor data if streaming is enabled
  if (isStreaming && webSocketConnected && (millis() - lastDataSend > DATA_SEND_INTERVAL)) {
    sendSensorData();
    lastDataSend = millis();
  }
  
  // Send periodic heartbeat
  if (webSocketConnected && (millis() - lastHeartbeat > HEARTBEAT_INTERVAL)) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
  
  // Update LED status
  updateStatusLED();
  
  delay(10); // Small delay to prevent watchdog issues
}

void connectToWiFi() {
  Serial.println("📶 Connecting to WiFi...");
  Serial.printf("   SSID: %s\n", WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (millis() - startTime > 20000) { // 20 second timeout
      break;
    }
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("✅ WiFi connected successfully!");
    Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   MAC Address: %s\n", WiFi.macAddress().c_str());
    Serial.printf("   Signal Strength: %d dBm\n", WiFi.RSSI());
    Serial.printf("   Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    blinkLED(2, 200); // 2 quick blinks for WiFi success
  } else {
    wifiConnected = false;
    Serial.println("❌ WiFi connection failed!");
    Serial.println("   Please check your WiFi credentials and network");
    Serial.println("   Continuing in offline mode...");
    blinkLED(5, 100); // 5 quick blinks for WiFi failure
  }
  Serial.println();
}

void checkWiFiConnection() {
  static unsigned long lastWiFiCheck = 0;
  
  if (millis() - lastWiFiCheck > 5000) { // Check every 5 seconds
    if (wifiConnected && WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️  WiFi connection lost! Attempting to reconnect...");
      wifiConnected = false;
      webSocketConnected = false;
      connectToWiFi();
      
      if (wifiConnected) {
        initializeWebSocket();
      }
    }
    lastWiFiCheck = millis();
  }
}

void initializeWebSocket() {
  Serial.printf("🔌 Initializing WebSocket connection...\n");
  Serial.printf("   Target: ws://%s:%d%s\n", BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
  
  // Configure WebSocket client
  webSocket.begin(BACKEND_HOST, BACKEND_PORT, WEBSOCKET_PATH);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2); // ping interval, pong timeout, disconnect timeout
  
  Serial.println("🔌 WebSocket client configured, attempting connection...");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("🔌 WebSocket Disconnected");
      webSocketConnected = false;
      break;

    case WStype_CONNECTED:
      Serial.printf("🔌 WebSocket Connected to: %s\n", payload);
      webSocketConnected = true;
      connectionStartTime = millis();
      blinkLED(3, 300); // 3 blinks for WebSocket connection
      
      // Send device identification
      sendDeviceIdentification();
      break;

    case WStype_TEXT:
      Serial.printf("📨 Received: %s\n", payload);
      totalMessagesReceived++;
      handleWebSocketMessage((char*)payload);
      break;

    case WStype_BIN:
      Serial.printf("📦 Binary data received: %u bytes\n", length);
      break;

    case WStype_PING:
      Serial.println("💓 Ping received");
      break;

    case WStype_PONG:
      Serial.println("💓 Pong received");
      break;

    case WStype_ERROR:
      Serial.printf("❌ WebSocket Error: %s\n", payload);
      break;

    default:
      Serial.printf("🔍 Unknown WebSocket event: %d\n", type);
      break;
  }
}

void sendDeviceIdentification() {
  StaticJsonDocument<300> doc;
  doc["type"] = "client_identify";
  doc["client_type"] = "esp32";
  doc["deviceId"] = deviceId;
  doc["name"] = DEVICE_NAME;
  doc["firmwareVersion"] = FIRMWARE_VERSION;
  doc["capabilities"] = "MPU6050,LED,Button,WiFi";
  doc["macAddress"] = WiFi.macAddress();
  doc["ipAddress"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  totalMessagesSent++;
  Serial.println("📤 Sent device identification");
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
    Serial.println("✅ Backend confirmed WebSocket connection");
    
  } else if (messageType == "identification_confirmed") {
    Serial.println("✅ Device identification confirmed by backend");
    blinkLED(1, 500); // 1 long blink for identification success
    
  } else if (messageType == "command") {
    handleCommand(doc);
    
  } else if (messageType == "sensor_data_ack") {
    // Optional: acknowledge sensor data receipt
    
  } else if (messageType == "error") {
    String errorMsg = doc["message"] | "Unknown error";
    Serial.printf("❌ Backend error: %s\n", errorMsg.c_str());
    
  } else if (messageType == "pong") {
    Serial.println("💓 Backend pong received");
    
  } else {
    Serial.printf("❓ Unknown message type: %s\n", messageType.c_str());
  }
}

void handleCommand(JsonObject doc) {
  String command = doc["command"] | "";
  Serial.printf("📋 Received command: %s\n", command.c_str());
  
  if (command == "start_streaming") {
    isStreaming = true;
    Serial.println("▶️  Started data streaming via command");
    sendCommandAck(command, "started");
    
  } else if (command == "stop_streaming") {
    isStreaming = false;
    Serial.println("⏹️  Stopped data streaming via command");
    sendCommandAck(command, "stopped");
    
  } else if (command == "led_on") {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("💡 LED turned ON");
    sendCommandAck(command, "led_on");
    
  } else if (command == "led_off") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("💡 LED turned OFF");
    sendCommandAck(command, "led_off");
    
  } else if (command == "blink") {
    int count = doc["count"] | 3;
    blinkLED(count, 200);
    Serial.printf("💡 LED blinked %d times\n", count);
    sendCommandAck(command, "blinked");
    
  } else if (command == "get_status") {
    sendDeviceStatus();
    sendCommandAck(command, "status_sent");
    
  } else if (command == "restart") {
    Serial.println("🔄 Restarting ESP32 via command...");
    sendCommandAck(command, "restarting");
    delay(1000);
    ESP.restart();
    
  } else {
    Serial.printf("❓ Unknown command: %s\n", command.c_str());
    sendCommandAck(command, "unknown");
  }
}

void sendCommandAck(String command, String status) {
  StaticJsonDocument<200> doc;
  doc["type"] = "command_ack";
  doc["command"] = command;
  doc["status"] = status;
  doc["deviceId"] = deviceId;
  doc["timestamp"] = millis();
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  totalMessagesSent++;
}

void checkMPU6050() {
  Wire.beginTransmission(0x68); // MPU6050 default address
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    mpuConnected = true;
    Serial.println("✅ MPU6050 sensor detected");
    
    // Initialize MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // Wake up the MPU6050
    Wire.endTransmission(true);
  } else {
    mpuConnected = false;
    Serial.println("⚠️  MPU6050 not found - using simulated data");
  }
}

SensorData readSensorData() {
  SensorData data;
  data.timestamp = millis();
  
  if (mpuConnected) {
    // Read real MPU6050 data
    Wire.beginTransmission(0x68);
    Wire.write(0x3B); // Starting register for accelerometer
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);
    
    // Read accelerometer data
    int16_t accelX_raw = Wire.read() << 8 | Wire.read();
    int16_t accelY_raw = Wire.read() << 8 | Wire.read();
    int16_t accelZ_raw = Wire.read() << 8 | Wire.read();
    
    // Skip temperature for now
    Wire.read(); Wire.read();
    
    // Read gyroscope data
    int16_t gyroX_raw = Wire.read() << 8 | Wire.read();
    int16_t gyroY_raw = Wire.read() << 8 | Wire.read();
    int16_t gyroZ_raw = Wire.read() << 8 | Wire.read();
    
    // Convert to real units
    data.accelX = accelX_raw / 16384.0; // ±2g range
    data.accelY = accelY_raw / 16384.0;
    data.accelZ = accelZ_raw / 16384.0;
    data.gyroX = gyroX_raw / 131.0;     // ±250°/s range
    data.gyroY = gyroY_raw / 131.0;
    data.gyroZ = gyroZ_raw / 131.0;
    data.temperature = 25.0; // Placeholder
    
  } else {
    // Generate simulated sensor data
    float time = millis() / 1000.0;
    data.accelX = sin(time * 2) * 0.1 + (random(-100, 100) / 1000.0);
    data.accelY = cos(time * 1.5) * 0.1 + (random(-100, 100) / 1000.0);
    data.accelZ = 1.0 + sin(time * 0.5) * 0.1; // Gravity + movement
    data.gyroX = sin(time * 3) * 50 + (random(-500, 500) / 100.0);
    data.gyroY = cos(time * 2.5) * 30 + (random(-300, 300) / 100.0);
    data.gyroZ = sin(time * 1.8) * 40 + (random(-400, 400) / 100.0);
    data.temperature = 24.5 + sin(time * 0.1) * 2; // Simulated temperature variation
  }
  
  return data;
}

void sendSensorData() {
  if (!webSocketConnected) return;
  
  SensorData data = readSensorData();
  
  StaticJsonDocument<400> doc;
  doc["type"] = "sensor_data";
  doc["device_id"] = deviceId;
  doc["timestamp"] = data.timestamp;
  
  JsonObject gyro = doc.createNestedObject("gyro");
  gyro["x"] = round(data.gyroX * 100) / 100.0;  // Round to 2 decimal places
  gyro["y"] = round(data.gyroY * 100) / 100.0;
  gyro["z"] = round(data.gyroZ * 100) / 100.0;
  
  JsonObject accel = doc.createNestedObject("accel");
  accel["x"] = round(data.accelX * 1000) / 1000.0; // Round to 3 decimal places
  accel["y"] = round(data.accelY * 1000) / 1000.0;
  accel["z"] = round(data.accelZ * 1000) / 1000.0;
  
  doc["temperature"] = round(data.temperature * 10) / 10.0;
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  totalMessagesSent++;
  
  // Print sensor data periodically (every 2 seconds)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    Serial.printf("📊 Sensor Data - Accel(%.3f,%.3f,%.3f) Gyro(%.1f,%.1f,%.1f) Temp:%.1f°C\n",
                  data.accelX, data.accelY, data.accelZ,
                  data.gyroX, data.gyroY, data.gyroZ,
                  data.temperature);
    lastPrint = millis();
  }
}

void sendHeartbeat() {
  if (!webSocketConnected) return;
  
  StaticJsonDocument<200> doc;
  doc["type"] = "heartbeat";
  doc["deviceId"] = deviceId;
  doc["timestamp"] = millis();
  doc["uptime"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["streaming"] = isStreaming;
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  totalMessagesSent++;
  
  Serial.printf("💓 Heartbeat sent (Uptime: %lu seconds, Free Heap: %d bytes)\n", 
                millis()/1000, ESP.getFreeHeap());
}

void sendDeviceStatus() {
  StaticJsonDocument<400> doc;
  doc["type"] = "device_status";
  doc["deviceId"] = deviceId;
  doc["name"] = DEVICE_NAME;
  doc["firmwareVersion"] = FIRMWARE_VERSION;
  doc["streaming"] = isStreaming;
  doc["wifi_connected"] = wifiConnected;
  doc["websocket_connected"] = webSocketConnected;
  doc["mpu6050_connected"] = mpuConnected;
  doc["uptime"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["totalMessagesSent"] = totalMessagesSent;
  doc["totalMessagesReceived"] = totalMessagesReceived;
  
  if (wifiConnected) {
    doc["wifi_ssid"] = WiFi.SSID();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["ip_address"] = WiFi.localIP().toString();
    doc["mac_address"] = WiFi.macAddress();
  }
  
  String message;
  serializeJson(doc, message);
  
  webSocket.sendTXT(message);
  totalMessagesSent++;
  Serial.println("📊 Device status sent to backend");
}

void handleButton() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  // Check for button press (falling edge with debounce)
  if (currentButtonState == LOW && lastButtonState == HIGH && 
      (millis() - lastButtonPress > BUTTON_DEBOUNCE)) {
    
    isStreaming = !isStreaming;
    lastButtonPress = millis();
    
    Serial.printf("🔘 Button pressed - Streaming %s\n", isStreaming ? "STARTED" : "STOPPED");
    
    if (isStreaming) {
      blinkLED(2, 150); // Quick double blink for start
    } else {
      blinkLED(1, 500); // Single long blink for stop
    }
    
    // Send notification to backend
    if (webSocketConnected) {
      StaticJsonDocument<150> doc;
      doc["type"] = "streaming_toggle";
      doc["deviceId"] = deviceId;
      doc["streaming"] = isStreaming;
      doc["timestamp"] = millis();
      
      String message;
      serializeJson(doc, message);
      webSocket.sendTXT(message);
      totalMessagesSent++;
    }
  }
  
  lastButtonState = currentButtonState;
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "status") {
      printSystemStatus();
    } else if (command == "start") {
      isStreaming = true;
      Serial.println("▶️  Data streaming started via serial command");
    } else if (command == "stop") {
      isStreaming = false;
      Serial.println("⏹️  Data streaming stopped via serial command");
    } else if (command == "restart") {
      Serial.println("🔄 Restarting ESP32...");
      delay(1000);
      ESP.restart();
    } else if (command == "wifi") {
      printWiFiInfo();
    } else if (command == "help") {
      printHelpCommands();
    } else if (command.length() > 0) {
      Serial.printf("❓ Unknown command: '%s'. Type 'help' for available commands.\n", command.c_str());
    }
  }
}

void updateStatusLED() {
  static unsigned long lastLEDUpdate = 0;
  static bool ledState = false;
  
  if (millis() - lastLEDUpdate > 1000) { // Update every second
    if (webSocketConnected) {
      if (isStreaming) {
        // Fast blink when streaming
        digitalWrite(LED_PIN, ledState);
        ledState = !ledState;
      } else {
        // Solid on when connected but not streaming
        digitalWrite(LED_PIN, HIGH);
      }
    } else if (wifiConnected) {
      // Slow blink when WiFi connected but no WebSocket
      digitalWrite(LED_PIN, ledState);
      ledState = !ledState;
    } else {
      // Off when not connected
      digitalWrite(LED_PIN, LOW);
    }
    lastLEDUpdate = millis();
  }
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    if (i < times - 1) delay(delayMs);
  }
}

void printSystemStatus() {
  Serial.println();
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║                 SYSTEM STATUS                  ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.printf("Device ID: %s\n", deviceId.c_str());
  Serial.printf("Firmware: %s\n", FIRMWARE_VERSION);
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println();
  
  Serial.println("📶 WiFi Status:");
  Serial.printf("  Connected: %s\n", wifiConnected ? "YES" : "NO");
  if (wifiConnected) {
    Serial.printf("  SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Signal: %d dBm\n", WiFi.RSSI());
  }
  Serial.println();
  
  Serial.println("🔌 WebSocket Status:");
  Serial.printf("  Connected: %s\n", webSocketConnected ? "YES" : "NO");
  if (webSocketConnected) {
    Serial.printf("  Connected for: %lu seconds\n", (millis() - connectionStartTime) / 1000);
  }
  Serial.printf("  Messages sent: %lu\n", totalMessagesSent);
  Serial.printf("  Messages received: %lu\n", totalMessagesReceived);
  Serial.println();
  
  Serial.println("📊 Sensor Status:");
  Serial.printf("  MPU6050: %s\n", mpuConnected ? "CONNECTED" : "SIMULATED");
  Serial.printf("  Streaming: %s\n", isStreaming ? "ACTIVE" : "STOPPED");
  Serial.println();
}

void printWiFiInfo() {
  Serial.println();
  Serial.println("📶 WiFi Information:");
  Serial.printf("  Status: %s\n", wifiConnected ? "Connected" : "Disconnected");
  if (wifiConnected) {
    Serial.printf("  SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("  IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("  Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("  Subnet: %s\n", WiFi.subnetMask().toString().c_str());
    Serial.printf("  DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("  MAC: %s\n", WiFi.macAddress().c_str());
    Serial.printf("  Signal: %d dBm\n", WiFi.RSSI());
  }
  Serial.println();
}

void printHelpCommands() {
  Serial.println();
  Serial.println("📋 Available Serial Commands:");
  Serial.println("  status   - Show system status");
  Serial.println("  start    - Start data streaming");
  Serial.println("  stop     - Stop data streaming");
  Serial.println("  wifi     - Show WiFi information");
  Serial.println("  restart  - Restart ESP32");
  Serial.println("  help     - Show this help message");
  Serial.println();
} 