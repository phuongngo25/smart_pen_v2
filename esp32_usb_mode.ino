/**
 * Smart Pen ESP32 - USB Mode Only
 * Optimized for maximum USB/Serial performance
 * 
 * Version: 1.0.0
 * Connection: USB Serial to laptop
 * Data Rate: 20Hz (optimized for USB bandwidth)
 */

#include <Wire.h>
#include <MPU6050.h>

// =============================================
// HARDWARE CONFIGURATION  
// =============================================
#define LED_PIN 2
#define BUTTON_PIN 0
#define MPU6050_ADDR 0x68
#define USB_BAUD_RATE 115200

// =============================================
// TIMING CONFIGURATION
// =============================================
const unsigned long USB_INTERVAL = 50;  // 20Hz for maximum USB performance
const unsigned long BUTTON_DEBOUNCE = 50;

// =============================================
// GLOBAL VARIABLES
// =============================================
bool isStreaming = true;  // Start streaming immediately
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
unsigned long lastDataSend = 0;
unsigned long lastLEDBlink = 0;
bool ledState = false;

// Hardware
MPU6050 mpu;
bool mpuConnected = false;

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
  Serial.begin(USB_BAUD_RATE);
  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }
  
  Serial.println("🦡 Smart Pen ESP32 - USB Mode");
  Serial.println("📊 Optimized for maximum USB performance (20Hz)");
  
  // Initialize hardware
  initializeHardware();
  
  Serial.println("✅ USB Mode ready! Data streaming will start automatically");
  Serial.println("🎮 Press button to toggle streaming ON/OFF");
  Serial.println("📈 Format: gyro_x,gyro_y,gyro_z,accel_x,accel_y,accel_z");
}

void initializeHardware() {
  // LED and Button
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initial LED pattern
  blinkLED(3);
  
  // MPU6050 initialization
  Wire.begin();
  mpu.initialize();
  
  if (mpu.testConnection()) {
    mpuConnected = true;
    Serial.println("✅ MPU6050 connected successfully");
    
    // Optimize MPU6050 for high-frequency sampling
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);     // ±500°/s
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);     // ±4g
    mpu.setDLPFMode(MPU6050_DLPF_BW_42);                // 42Hz low-pass filter
    mpu.setRate(0);                                      // 1kHz sample rate
    
    Serial.println("⚡ MPU6050 optimized for high-speed USB streaming");
  } else {
    mpuConnected = false;
    Serial.println("⚠️  MPU6050 not found - using simulated data");
  }
}

// =============================================
// SENSOR FUNCTIONS
// =============================================
SensorData readSensorData() {
  SensorData data;
  data.timestamp = millis();
  
  if (mpuConnected) {
    // Read real MPU6050 data with optimized conversion
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Convert to proper units (optimized for precision)
    data.accel_x = ax / 8192.0;  // ±4g range
    data.accel_y = ay / 8192.0;
    data.accel_z = az / 8192.0;
    data.gyro_x = gx / 65.5;     // ±500°/s range
    data.gyro_y = gy / 65.5;
    data.gyro_z = gz / 65.5;
  } else {
    // High-quality simulated data for testing
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

void sendDataUSB(SensorData data) {
  // Optimized CSV format for USB with high precision
  Serial.printf("%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                data.gyro_x, data.gyro_y, data.gyro_z,
                data.accel_x, data.accel_y, data.accel_z);
}

// =============================================
// UTILITY FUNCTIONS
// =============================================
void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void updateLEDStatus() {
  unsigned long interval = isStreaming ? 100 : 1000; // Very fast when streaming
  
  if (millis() - lastLEDBlink > interval) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastLEDBlink = millis();
  }
}

void handleButton() {
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  
  if (currentButtonState && !buttonPressed && (millis() - lastButtonPress > BUTTON_DEBOUNCE)) {
    buttonPressed = true;
    lastButtonPress = millis();
  } else if (!currentButtonState && buttonPressed) {
    buttonPressed = false;
    
    // Toggle streaming
    isStreaming = !isStreaming;
    Serial.printf("📊 USB Data streaming: %s\n", isStreaming ? "ON" : "OFF");
    blinkLED(isStreaming ? 2 : 1);
  }
}

// =============================================
// MAIN LOOP
// =============================================
void loop() {
  handleButton();
  updateLEDStatus();
  
  if (!isStreaming) {
    delay(20);
    return;
  }
  
  // High-frequency data transmission for USB
  if (millis() - lastDataSend >= USB_INTERVAL) {
    SensorData data = readSensorData();
    sendDataUSB(data);
    lastDataSend = millis();
  }
  
  // Minimal delay for maximum performance
  delay(1);
} 