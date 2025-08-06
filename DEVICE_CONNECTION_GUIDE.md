# 🦡 BadgerPen ESP32 Device Connection Guide

This guide shows you how to connect your ESP32 Smart Pen device to the BadgerPen web application. You can follow either the **Hardware Setup** (for real ESP32 devices) or use the **Software Simulation** to test the system.

## 🎯 Quick Demo

Want to see it working immediately? Run this demo:

```bash
python3 device-connection-demo.py
```

This simulates an ESP32 device sending real sensor data to your BadgerPen application!

## 📋 Prerequisites

### System Requirements
- BadgerPen application running (Docker containers up)
- Python 3.7+ for testing/simulation
- For hardware: ESP32 dev board + MPU6050 sensor

### Verify System is Running
```bash
# Check containers are running
docker-compose ps

# Test backend connectivity
curl http://localhost:3005/api/health

# Open web application
open http://localhost:3004
```

## 🔌 Method 1: Software Simulation (Recommended for Testing)

This method simulates ESP32 device data without physical hardware.

### Step 1: Run the Device Simulator
```bash
# Navigate to project directory
cd Smart_pen_v2.0

# Run the device connection demo
python3 device-connection-demo.py
```

### Step 2: Follow the Interactive Demo
The script will:
1. ✅ Test backend connectivity
2. 📊 Send sample sensor data
3. 🔄 Offer continuous monitoring simulation
4. 📈 Show real-time data transmission

### Step 3: View Data in Web App
1. Open http://localhost:3004
2. Login with demo credentials:
   - Email: `admin@badgerpen.com`
   - Password: `badger123`
3. Navigate to **Dashboard** to see real-time data
4. Check **Analytics** for tremor analysis

## 🔧 Method 2: Hardware Setup (ESP32 + MPU6050)

For real hardware implementation with ESP32 and MPU6050 sensor.

### Hardware Connections
```
ESP32 Pin    →    MPU6050 Pin    →    Function
─────────────────────────────────────────────────
3.3V         →    VCC            →    Power
GND          →    GND            →    Ground
GPIO21 (SDA) →    SDA            →    I2C Data
GPIO22 (SCL) →    SCL            →    I2C Clock
GPIO2        →    LED + 220Ω     →    Status indicator (optional)
GPIO0        →    Button to GND  →    Mode switch (optional)
```

### Firmware Setup

#### Option A: Quick Setup (Arduino IDE)
1. **Install ESP32 board support:**
   - Add `https://dl.espressif.com/dl/package_esp32_index.json` to Board Manager URLs
   - Install "ESP32 by Espressif Systems"

2. **Install required libraries:**
   - ArduinoJson (v6.21.3+)
   - MPU6050 by Electronic Cats

3. **Configure WiFi and server:**
   ```cpp
   // In your ESP32 code:
   const char* ssid = "YourWiFiNetwork";
   const char* password = "YourWiFiPassword";
   const char* serverURL = "http://192.168.1.XXX:3005";  // Your computer's IP
   ```

4. **Use this data transmission format:**
   ```cpp
   // Send data to: http://your-ip:3005/api/mpu6050/readings
   {
     "deviceId": "ESP32_001",
     "gyro_x": gyroX,
     "gyro_y": gyroY, 
     "gyro_z": gyroZ,
     "accel_x": accelX,
     "accel_y": accelY,
     "accel_z": accelZ,
     "movement_type": "WRITING"
   }
   ```

#### Option B: PlatformIO Setup
```bash
# Install PlatformIO
pip install -U platformio

# Create and configure project
cd esp32/
pio init --board esp32dev
pio lib install "ArduinoJson" "MPU6050lib"

# Build and upload
pio run --target upload
pio device monitor
```

### Find Your Computer's IP Address
```bash
# macOS/Linux
ifconfig | grep "inet " | grep -v 127.0.0.1

# Windows
ipconfig | findstr "IPv4"

# Use this IP in your ESP32 server URL
```

## 📊 Testing Your Connection

### Real-time Data Verification

1. **Backend API Test:**
   ```bash
   # Manual data test
   curl -X POST http://localhost:3005/api/mpu6050/readings \
     -H "Content-Type: application/json" \
     -d '{
       "deviceId": "YOUR_DEVICE_ID",
       "gyro_x": 1.2, "gyro_y": 0.8, "gyro_z": 0.3,
       "accel_x": 0.1, "accel_y": 0.05, "accel_z": 1.0
     }'
   ```

2. **Web Application Check:**
   - Dashboard shows device status: **Online**
   - Real-time sensor graphs update
   - Device appears in device list
   - Analytics page shows tremor analysis

3. **Data Stream Verification:**
   ```bash
   # Check stored readings
   curl "http://localhost:3005/api/mpu6050/readings?deviceId=YOUR_DEVICE_ID&limit=5"
   ```

## 🌐 Web Application Integration

### Dashboard Features
- **Real-time sensor visualization**
- **3D movement tracking**
- **Battery and connection status**
- **Device health monitoring**

### Analytics Features
- **Tremor frequency analysis**
- **Movement pattern recognition**
- **Parkinson's disease indicators**
- **Historical trend analysis**

### Alerts & Notifications
- **Tremor detection alerts**
- **Device connectivity status**
- **Battery level warnings**
- **Calibration reminders**

## 🔧 Troubleshooting

### Common Issues

| Problem | Symptoms | Solution |
|---------|----------|----------|
| **Backend Not Accessible** | Connection refused | `docker-compose up -d` |
| **Wrong IP Address** | ESP32 can't connect | Check computer IP with `ifconfig` |
| **WiFi Connection Failed** | ESP32 WiFi error | Use 2.4GHz network, check credentials |
| **No Data in Web App** | Empty dashboard | Verify device ID matches |
| **Sensor Not Working** | No MPU6050 data | Check I2C wiring (SDA/SCL) |

### Debug Commands
```bash
# Check all services
docker-compose ps

# View backend logs
docker-compose logs backend -f

# Test network connectivity
ping 192.168.1.XXX  # Your computer's IP

# Verify data storage
curl "http://localhost:3005/api/mpu6050/readings?limit=1"
```

### ESP32 Serial Monitor Debug
Look for these messages in Arduino Serial Monitor:
```
✅ WiFi connected!
📡 IP address: 192.168.1.200
📤 Data sent successfully
💓 Heartbeat sent
```

## 🎯 Expected Results

When everything is working correctly:

1. **ESP32 Serial Output:**
   ```
   🦡 BadgerPen Smart Pen v2.1.3
   📶 WiFi connected: YourNetwork
   📡 IP: 192.168.1.200
   📊 Sending sensor data...
   📤 Data sent successfully
   ```

2. **Web Application Dashboard:**
   - Device status: **🟢 Online**
   - Real-time graphs showing movement
   - Latest sensor readings updating
   - 3D visualization of pen movement

3. **Backend Logs:**
   ```
   📊 Sensor data from ESP32_001: accel(0.12, 0.05, 1.00)
   📈 Processing tremor analysis...
   ✅ Data stored successfully
   ```

## 🚀 Next Steps

Once your device is connected:

1. **Calibrate the device** for accurate readings
2. **Start a monitoring session** for data collection
3. **View analytics** for tremor patterns
4. **Set up alerts** for abnormal activity
5. **Export data** for clinical analysis

## 📞 Support

If you encounter issues:

1. **Check the logs:** `docker-compose logs -f`
2. **Verify network:** Can ESP32 ping your computer?
3. **Test manually:** Use the Python demo script
4. **Check firewall:** Allow port 3005
5. **Review wiring:** Ensure proper ESP32-MPU6050 connections

---

## 🎉 Success!

Your ESP32 Smart Pen device is now successfully connected to the BadgerPen web application! You can monitor real-time sensor data, analyze movement patterns, and detect tremor activity through the comprehensive web interface.

**Next:** Explore the Analytics dashboard to see AI-powered Parkinson's disease detection in action! 🧠✨ 