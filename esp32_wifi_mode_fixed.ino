/**
 * Smart Pen ESP32 - WiFi Mode (Fixed for 192.168.2.x Network)
 * Optimized for reliable WiFi connection to laptop server
 * 
 * Version: 2.1.0 - Network Configuration Fixed
 * Connection: WiFi HTTP POST to backend server
 * Data Rate: 10Hz (optimized for WiFi stability)
 * Features: Dynamic IP discovery, AP setup mode, robust connection
 * 
 * NETWORK CONFIGURATION:
 * - Laptop IP: 192.168.2.13 (detected from ifconfig)
 * - Subnet: 192.168.2.0/24
 * - Server Port: 3005
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>
#include <WebServer.h>
#include <EEPROM.h>

// =============================================
// NETWORK CONFIGURATION (UPDATED FOR YOUR NETWORK)
// =============================================
String LAPTOP_IP = "192.168.2.13";          // ✅ Your actual laptop IP
const int LAPTOP_PORT = 3005;                // Backend server port
const char* WIFI_ENDPOINT = "/api/mpu6050/readings";

// WiFi credentials - UPDATE THESE with your actual WiFi details
String WIFI_SSID = "YourWiFiNetwork";        // UPDATE THIS with your WiFi name
String WIFI_PASSWORD = "";                   // UPDATE THIS with your WiFi password

// For WiFi setup mode (when main WiFi fails or first setup)
const char* AP_SSID = "ESP32_SmartPen_Setup";
const char* AP_PASSWORD = "smartpen2024";

// =============================================
// EEPROM CONFIGURATION
// =============================================
#define EEPROM_SIZE 512
#define EEPROM_SSID_ADDR 0
#define EEPROM_PASS_ADDR 64
#define EEPROM_IP_ADDR 128
#define EEPROM_CONFIGURED_ADDR 192

// =============================================
// HARDWARE CONFIGURATION  
// =============================================
#define LED_PIN 2
#define BUTTON_PIN 0
#define MPU6050_ADDR 0x68

// =============================================
// TIMING CONFIGURATION
// =============================================
const unsigned long WIFI_INTERVAL = 100;    // 10Hz for stable WiFi transmission
const unsigned long RECONNECT_INTERVAL = 5000;
const unsigned long BUTTON_DEBOUNCE = 50;
const unsigned long IP_DISCOVERY_INTERVAL = 10000; // Try new IPs every 10 seconds

// =============================================
// GLOBAL VARIABLES
// =============================================
bool isStreaming = false; // Start with streaming off for WiFi setup
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
unsigned long lastDataSend = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long lastLEDBlink = 0;
unsigned long lastIPDiscovery = 0;
bool ledState = false;
bool configurationMode = false;

// Network
WiFiClient wifiClient;
HTTPClient http;
bool wifiConnected = false;
WebServer server(80);

// Hardware
MPU6050 mpu;
bool mpuConnected = false;

// IP Discovery - Updated for your network (192.168.2.x)
String commonIPs[] = {
  "192.168.2.13",   // ✅ Your detected laptop IP (primary)
  "192.168.2.1",    // Router/gateway
  "192.168.2.100", "192.168.2.101", "192.168.2.102", "192.168.2.103",
  "192.168.2.110", "192.168.2.111", "192.168.2.112", "192.168.2.113",
  // Fallback to other common subnets
  "192.168.1.100", "192.168.1.101", "192.168.1.102",
  "192.168.0.100", "192.168.0.101", "192.168.0.102",
  "10.0.0.100", "10.0.0.101", "172.20.10.2", "172.20.10.3"
};
int currentIPIndex = 0;

// =============================================
// SENSOR DATA STRUCTURE
// =============================================
struct SensorData {
  float gyro_x, gyro_y, gyro_z;
  float accel_x, accel_y, accel_z;
  unsigned long timestamp;
};

// =============================================
// SETUP FUNCTION
// =============================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("🦡 Smart Pen ESP32 - WiFi Mode v2.1 (Network Fixed)");
  Serial.println("📶 Configured for 192.168.2.x network");
  Serial.printf("📡 Target server: %s:%d\n", LAPTOP_IP.c_str(), LAPTOP_PORT);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize hardware first
  initializeHardware();
  
  // Load saved configuration
  loadConfiguration();
  
  // Initialize WiFi
  initializeWiFi();
  
  Serial.println("✅ WiFi Mode ready!");
  Serial.println("🎮 Press button to toggle streaming ON/OFF");
  printNetworkInfo();
}

void initializeHardware() {
  // LED and Button
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initial LED pattern (3 blinks for WiFi mode)
  blinkLED(3);
  
  // MPU6050 initialization
  Wire.begin();
  mpu.initialize();
  
  if (mpu.testConnection()) {
    mpuConnected = true;
    Serial.println("✅ MPU6050 connected successfully");
    
    // Configure MPU6050 for WiFi mode (balanced performance)
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
    mpu.setDLPFMode(MPU6050_DLPF_BW_20);  // 20Hz filter for stable readings
  } else {
    mpuConnected = false;
    Serial.println("⚠️  MPU6050 not found - using simulated data");
  }
}

// =============================================
// CONFIGURATION FUNCTIONS
// =============================================
void loadConfiguration() {
  // Check if device has been configured
  if (EEPROM.read(EEPROM_CONFIGURED_ADDR) == 0xFF) {
    Serial.println("📋 No saved configuration found - will start AP setup mode");
    return;
  }
  
  // Load WiFi SSID
  char ssid[64] = {0};
  for (int i = 0; i < 63; i++) {
    char c = EEPROM.read(EEPROM_SSID_ADDR + i);
    if (c == 0) break;
    ssid[i] = c;
  }
  WIFI_SSID = String(ssid);
  
  // Load WiFi Password
  char pass[64] = {0};
  for (int i = 0; i < 63; i++) {
    char c = EEPROM.read(EEPROM_PASS_ADDR + i);
    if (c == 0) break;
    pass[i] = c;
  }
  WIFI_PASSWORD = String(pass);
  
  // Load Laptop IP
  char ip[16] = {0};
  for (int i = 0; i < 15; i++) {
    char c = EEPROM.read(EEPROM_IP_ADDR + i);
    if (c == 0) break;
    ip[i] = c;
  }
  if (strlen(ip) > 0) {
    LAPTOP_IP = String(ip);
  }
  
  Serial.println("📋 Loaded saved configuration:");
  Serial.printf("   SSID: %s\n", WIFI_SSID.c_str());
  Serial.printf("   Laptop IP: %s\n", LAPTOP_IP.c_str());
}

void saveConfiguration() {
  // Clear configuration area
  for (int i = 0; i < 256; i++) {
    EEPROM.write(i, 0);
  }
  
  // Save WiFi SSID
  for (int i = 0; i < WIFI_SSID.length() && i < 63; i++) {
    EEPROM.write(EEPROM_SSID_ADDR + i, WIFI_SSID[i]);
  }
  
  // Save WiFi Password
  for (int i = 0; i < WIFI_PASSWORD.length() && i < 63; i++) {
    EEPROM.write(EEPROM_PASS_ADDR + i, WIFI_PASSWORD[i]);
  }
  
  // Save Laptop IP
  for (int i = 0; i < LAPTOP_IP.length() && i < 15; i++) {
    EEPROM.write(EEPROM_IP_ADDR + i, LAPTOP_IP[i]);
  }
  
  // Mark as configured
  EEPROM.write(EEPROM_CONFIGURED_ADDR, 0x01);
  EEPROM.commit();
  
  Serial.println("💾 Configuration saved to EEPROM");
}

// =============================================
// WIFI FUNCTIONS
// =============================================
void initializeWiFi() {
  if (WIFI_SSID == "YourWiFiNetwork" || WIFI_SSID.length() == 0) {
    Serial.println("📶 No WiFi configuration - starting AP setup mode");
    startWiFiAP();
    return;
  }
  
  Serial.println("📶 Connecting to WiFi...");
  Serial.printf("   Network: %s\n", WIFI_SSID.c_str());
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Blink LED during connection attempt
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("✅ WiFi connected successfully!");
    Serial.printf("   ESP32 IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Signal strength: %d dBm\n", WiFi.RSSI());
    
    // Test server connection with current IP
    if (!testServerConnection()) {
      Serial.println("🔍 Server not found at saved IP - starting discovery");
      discoverServerIP();
    }
    
    // Solid LED to indicate successful connection
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
    Serial.println("   Starting Access Point for configuration...");
    startWiFiAP();
  }
}

bool testServerConnection() {
  Serial.printf("🔍 Testing connection to server %s:%d\n", LAPTOP_IP.c_str(), LAPTOP_PORT);
  
  http.begin(wifiClient, String("http://") + LAPTOP_IP + ":" + LAPTOP_PORT + "/api/health");
  http.setTimeout(3000);
  
  int httpResponseCode = http.GET();
  http.end();
  
  if (httpResponseCode == 200) {
    Serial.println("✅ Server connection test successful - Ready to send data!");
    blinkLED(2); // Success indication
    return true;
  } else {
    Serial.printf("⚠️  Server connection test failed: HTTP %d\n", httpResponseCode);
    return false;
  }
}

void discoverServerIP() {
  Serial.println("🔍 Starting IP discovery process (prioritizing 192.168.2.x)...");
  
  for (int i = 0; i < sizeof(commonIPs)/sizeof(commonIPs[0]); i++) {
    Serial.printf("   Trying IP: %s\n", commonIPs[i].c_str());
    
    http.begin(wifiClient, String("http://") + commonIPs[i] + ":" + LAPTOP_PORT + "/api/health");
    http.setTimeout(2000);
    
    int httpResponseCode = http.GET();
    http.end();
    
    if (httpResponseCode == 200) {
      LAPTOP_IP = commonIPs[i];
      Serial.printf("✅ Found server at: %s\n", LAPTOP_IP.c_str());
      saveConfiguration(); // Save the discovered IP
      blinkLED(3); // Discovery success
      return;
    }
    
    delay(100);
  }
  
  Serial.println("❌ Server IP discovery failed - using saved IP");
  blinkLED(5); // Discovery failed
}

void startWiFiAP() {
  configurationMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  Serial.println("📱 WiFi Access Point started for configuration:");
  Serial.printf("   SSID: %s\n", AP_SSID);
  Serial.printf("   Password: %s\n", AP_PASSWORD);
  Serial.printf("   Setup URL: http://%s\n", WiFi.softAPIP().toString().c_str());
  Serial.println("   Connect to this network and visit the URL to configure WiFi");
  
  setupConfigServer();
}

void setupConfigServer() {
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 WiFi Setup</title>";
    html += "<style>body{font-family:Arial,sans-serif;margin:40px;background:#f5f5f5;}";
    html += ".container{max-width:500px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
    html += "h1{color:#333;text-align:center;}input,select{width:100%;padding:10px;margin:10px 0;border:1px solid #ddd;border-radius:5px;}";
    html += "button{background:#007bff;color:white;padding:12px 30px;border:none;border-radius:5px;cursor:pointer;width:100%;}";
    html += "button:hover{background:#0056b3;}.notice{background:#e7f3ff;padding:15px;border-radius:5px;margin:10px 0;}</style></head><body>";
    html += "<div class='container'><h1>🦡 Smart Pen WiFi Setup</h1>";
    html += "<div class='notice'><strong>📡 Detected Network:</strong><br>";
    html += "Target laptop IP: <code>192.168.2.13</code><br>";
    html += "Make sure your laptop and ESP32 will be on the same WiFi network!</div>";
    html += "<form method='POST' action='/configure'>";
    html += "<label>WiFi Network:</label><input type='text' name='ssid' placeholder='Your WiFi Network Name' required><br>";
    html += "<label>WiFi Password:</label><input type='password' name='password' placeholder='WiFi Password'><br>";
    html += "<label>Laptop IP Address:</label><input type='text' name='laptop_ip' value='" + LAPTOP_IP + "' placeholder='192.168.2.13' required><br>";
    html += "<label>Auto-discover IP:</label><select name='auto_discover'><option value='yes'>Yes (Recommended)</option><option value='no'>No</option></select><br>";
    html += "<button type='submit'>💾 Save & Connect</button>";
    html += "</form><br><hr><br>";
    html += "<p><strong>Instructions:</strong></p>";
    html += "<ol><li>Enter your WiFi network credentials</li>";
    html += "<li>Confirm laptop IP is 192.168.2.13 (or update if different)</li>";
    html += "<li>Make sure Smart Pen backend is running on port 3005</li>";
    html += "<li>Both devices must be on the same WiFi network</li></ol>";
    html += "<p><strong>Troubleshooting:</strong></p>";
    html += "<ul><li>Run <code>ifconfig</code> on Mac to confirm IP</li>";
    html += "<li>Start server: <code>node backend/enhanced-simple-server.js</code></li>";
    html += "<li>Test: <code>curl http://192.168.2.13:3005/api/health</code></li></ul>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  });
  
  server.on("/configure", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String laptop_ip = server.arg("laptop_ip");
    String auto_discover = server.arg("auto_discover");
    
    Serial.printf("Received WiFi config: %s\n", ssid.c_str());
    
    // Update global variables
    WIFI_SSID = ssid;
    WIFI_PASSWORD = password;
    if (auto_discover != "yes") {
      LAPTOP_IP = laptop_ip;
    }
    
    server.send(200, "text/html", 
      "<html><head><title>Configuration Saved</title></head><body style='font-family:Arial;text-align:center;margin-top:100px;'>"
      "<h1>✅ Configuration Saved!</h1>"
      "<p>ESP32 is connecting to WiFi...</p>"
      "<p>Target server: <strong>" + LAPTOP_IP + ":3005</strong></p>"
      "<p>The device will restart and connect automatically.</p>"
      "<p><strong>Next:</strong> Check your Smart Pen dashboard for the connected device.</p>"
      "</body></html>");
    
    // Save configuration
    saveConfiguration();
    
    delay(2000);
    ESP.restart();
  });
  
  server.begin();
  Serial.println("🌐 Configuration web server started");
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    wifiConnected = false;
    Serial.println("📶 WiFi connection lost - attempting reconnection...");
  }
  
  if (!wifiConnected && !configurationMode && millis() - lastReconnectAttempt > RECONNECT_INTERVAL) {
    Serial.print("📶 Reconnecting to WiFi...");
    WiFi.reconnect();
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.println(" ✅ Reconnected!");
      
      // Test server connection after reconnect
      if (!testServerConnection() && millis() - lastIPDiscovery > IP_DISCOVERY_INTERVAL) {
        discoverServerIP();
        lastIPDiscovery = millis();
      }
    } else {
      Serial.println(" ❌ Failed");
    }
    
    lastReconnectAttempt = millis();
  }
}

// =============================================
// SENSOR FUNCTIONS
// =============================================
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
    // Generate simulated data
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

void sendDataWiFi(SensorData data) {
  if (!wifiConnected || WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["device_id"] = "ESP32_WiFi_SmartPen_Fixed";
  doc["timestamp"] = data.timestamp;
  doc["gyro_x"] = data.gyro_x;
  doc["gyro_y"] = data.gyro_y;
  doc["gyro_z"] = data.gyro_z;
  doc["accel_x"] = data.accel_x;
  doc["accel_y"] = data.accel_y;
  doc["accel_z"] = data.accel_z;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send HTTP POST
  http.begin(wifiClient, String("http://") + LAPTOP_IP + ":" + LAPTOP_PORT + WIFI_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(2000);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode == 200) {
    // Success - only log occasionally to avoid spam
    static unsigned long lastSuccessLog = 0;
    if (millis() - lastSuccessLog > 5000) { // Log every 5 seconds
      Serial.printf("✅ Data sent successfully to %s:%d\n", LAPTOP_IP.c_str(), LAPTOP_PORT);
      lastSuccessLog = millis();
    }
  } else {
    Serial.printf("❌ WiFi send failed: HTTP %d\n", httpResponseCode);
    if (httpResponseCode == -1) {
      Serial.println("   Connection timeout - check server status");
    }
  }
  
  http.end();
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

void handleButton() {
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  
  if (currentButtonState && !buttonPressed && (millis() - lastButtonPress > BUTTON_DEBOUNCE)) {
    buttonPressed = true;
    lastButtonPress = millis();
  } else if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    
    if (configurationMode) {
      // Exit configuration mode and try to connect
      Serial.println("🔄 Exiting configuration mode...");
      configurationMode = false;
      ESP.restart();
    } else if (wifiConnected) {
      // Toggle streaming only if WiFi is connected
      isStreaming = !isStreaming;
      Serial.printf("📊 WiFi streaming: %s\n", isStreaming ? "ON" : "OFF");
      blinkLED(isStreaming ? 2 : 1);
    } else {
      Serial.println("⚠️  Cannot start streaming - WiFi not connected");
      blinkLED(3);
    }
  }
}

void updateLEDStatus() {
  if (configurationMode) {
    // Fast blink when in configuration mode
    if (millis() - lastLEDBlink > 100) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastLEDBlink = millis();
    }
  } else if (!wifiConnected) {
    // Fast blink when disconnected
    if (millis() - lastLEDBlink > 200) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastLEDBlink = millis();
    }
  } else {
    // Slow blink when connected, fast when streaming
    unsigned long interval = isStreaming ? 250 : 1000;
    if (millis() - lastLEDBlink > interval) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastLEDBlink = millis();
    }
  }
}

void printNetworkInfo() {
  Serial.println("\n🌐 NETWORK CONFIGURATION:");
  Serial.printf("   WiFi SSID: %s\n", WIFI_SSID.c_str());
  Serial.printf("   Laptop IP: %s\n", LAPTOP_IP.c_str());
  Serial.printf("   Server Port: %d\n", LAPTOP_PORT);
  Serial.printf("   Endpoint: %s\n", WIFI_ENDPOINT);
  Serial.printf("   Data Rate: 10Hz\n");
  Serial.printf("   WiFi Status: %s\n", wifiConnected ? "CONNECTED" : "DISCONNECTED");
  
  if (wifiConnected) {
    Serial.printf("   ESP32 IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Signal: %d dBm\n", WiFi.RSSI());
  }
  
  Serial.println("\n🎮 CONTROLS:");
  Serial.println("   Press button: Toggle streaming ON/OFF");
  Serial.println("   LED: Fast blink = streaming, Slow = connected, Very fast = disconnected");
  
  Serial.println("\n📋 SETUP STATUS:");
  Serial.printf("   Configuration Mode: %s\n", configurationMode ? "YES" : "NO");
  Serial.printf("   MPU6050 Connected: %s\n", mpuConnected ? "YES" : "NO (using simulation)");
  Serial.printf("   Target Server: http://%s:%d%s\n", LAPTOP_IP.c_str(), LAPTOP_PORT, WIFI_ENDPOINT);
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  if (configurationMode) {
    // Handle configuration server
    server.handleClient();
    handleButton();
    updateLEDStatus();
    delay(10);
    return;
  }
  
  checkWiFiConnection();
  handleButton();
  updateLEDStatus();
  
  if (!isStreaming || !wifiConnected) {
    delay(50);
    return;
  }
  
  // Send data at WiFi-optimized rate
  if (millis() - lastDataSend >= WIFI_INTERVAL) {
    SensorData data = readSensorData();
    sendDataWiFi(data);
    lastDataSend = millis();
  }
  
  delay(10);
} 