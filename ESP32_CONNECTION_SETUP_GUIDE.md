# ESP32 Smart Pen Connection Setup Guide

## 🎯 Overview

This guide helps you set up your ESP32 Smart Pen with three different connection modes:
- **USB Mode**: Direct serial connection (most reliable)
- **WiFi Mode**: Wireless HTTP connection via local network
- **Bluetooth Mode**: BLE connection for mobile/web applications

## 🔧 Prerequisites

### Hardware Requirements
- ESP32 Development Board
- MPU6050 6-axis sensor
- USB cable (for programming and USB mode)
- LED on pin 2 (optional, for status indication)
- Button on pin 0 (optional, for mode switching)

### Software Requirements
- Arduino IDE with ESP32 board support
- Required libraries:
  - WiFi (built-in)
  - HTTPClient (built-in)
  - ArduinoJson
  - Wire (built-in)
  - MPU6050
  - BLEDevice (built-in)
  - BLEServer (built-in)
  - BLE2902 (built-in)

### Network Requirements
- Laptop/computer on local WiFi network
- Backend server running on port 3005
- Firewall configured to allow port 3005

## 📊 Current Network Configuration

**Your Setup:**
- Laptop IP: `192.168.2.13`
- WiFi Network: `TRUNG KIEN`
- Server Port: `3005`
- Backend URL: `http://192.168.2.13:3005`

## 🚀 Quick Start

### Step 1: Start the Backend Server
```bash
cd /Users/phuongngo/Code/Smart_pen_v2.0
./start-smart-pen-enhanced.sh
```

### Step 2: Choose Your Connection Mode

## 📱 Mode 1: USB Connection (Recommended for Testing)

### Features
- **Most Reliable**: Direct serial connection
- **Highest Data Rate**: 20Hz (50ms intervals)
- **No Network Required**: Works offline
- **Plug & Play**: No configuration needed

### Setup Instructions

1. **Upload Firmware**:
   ```
   File: esp32_usb_mode.ino
   ```

2. **Hardware Connection**:
   - Connect ESP32 to laptop via USB cable
   - Device should appear as `/dev/tty.usbserial-0001` (macOS)

3. **Backend Connection**:
   ```bash
   curl -X POST "http://localhost:3005/api/devices/connect-usb" \
     -H "Content-Type: application/json" \
     -d '{"devicePath": "/dev/tty.usbserial-0001", "baudRate": "115200"}'
   ```

4. **Verify Connection**:
   - Open Serial Monitor (115200 baud)
   - Should see: "🦡 Smart Pen ESP32 - USB Mode"
   - Press button to toggle streaming

### LED Indicators
- **Very Fast Blink**: Streaming active (20Hz)
- **Slow Blink**: Streaming paused
- **3 Blinks at startup**: USB mode selected

### Data Format
```
CSV: gyro_x,gyro_y,gyro_z,accel_x,accel_y,accel_z
Example: 1.2345,-0.9876,0.1234,8.7654,-1.5432,3.2109
```

## 📶 Mode 2: WiFi Connection

### Features
- **Wireless**: No physical connection needed
- **Network Dependent**: Requires stable WiFi
- **Good Data Rate**: 10Hz (100ms intervals)
- **HTTP Protocol**: JSON data transmission

### Setup Instructions

1. **Update WiFi Credentials**:
   ```cpp
   // In esp32_wifi_mode.ino
   const char* WIFI_SSID = "TRUNG KIEN";
   const char* WIFI_PASSWORD = "YOUR_ACTUAL_PASSWORD";  // ⚠️ UPDATE THIS!
   ```

2. **Upload Firmware**:
   ```
   File: esp32_wifi_mode.ino
   ```

3. **Network Setup**:
   - Ensure ESP32 and laptop on same WiFi network
   - Verify laptop IP: `192.168.2.13`
   - Check firewall allows port 3005

4. **Connection Process**:
   - ESP32 auto-connects to WiFi
   - Tests server connection on startup
   - If WiFi fails, creates AP: "ESP32_SmartPen_Setup"

### WiFi Setup Mode (if connection fails)
1. **Connect to ESP32 AP**:
   - SSID: `ESP32_SmartPen_Setup`
   - Password: `smartpen2024`

2. **Configure WiFi**:
   - Open browser: `http://192.168.4.1`
   - Enter your WiFi credentials
   - ESP32 will restart and connect

### LED Indicators
- **Very Fast Blink**: Disconnected from WiFi
- **Fast Blink**: Connected + Streaming
- **Slow Blink**: Connected + Paused
- **3 Blinks at startup**: WiFi mode selected

### Data Format
```json
{
  "device_id": "ESP32_WiFi_SmartPen",
  "timestamp": 1234567890,
  "gyro_x": 1.23,
  "gyro_y": -0.98,
  "gyro_z": 0.12,
  "accel_x": 8.76,
  "accel_y": -1.54,
  "accel_z": 3.21
}
```

### Troubleshooting WiFi Mode

**Connection Failed:**
```bash
# Check laptop IP
ifconfig | grep "inet " | grep -v 127.0.0.1

# Test server
curl http://192.168.2.13:3005/api/devices

# Check firewall
sudo ufw status  # Linux
# or check macOS System Preferences > Security & Privacy > Firewall
```

**ESP32 Not Connecting:**
1. Verify WiFi credentials
2. Check network compatibility (2.4GHz required)
3. Try ESP32 setup mode
4. Check serial monitor for error messages

## 📘 Mode 3: Bluetooth BLE Connection

### Features
- **Mobile Friendly**: Works with phones/tablets
- **Universal**: No network configuration
- **Moderate Data Rate**: 8Hz (125ms intervals)
- **Low Power**: Optimized for battery usage

### Setup Instructions

1. **Upload Firmware**:
   ```
   File: esp32_bluetooth_mode.ino
   ```

2. **BLE Configuration**:
   - Device Name: `BadgerPen_SmartPen`
   - Service UUID: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
   - Compatible with Nordic UART Service

3. **Connection Process**:
   - ESP32 starts advertising automatically
   - Scan for "BadgerPen_SmartPen" in BLE apps
   - Connect and start streaming

### BLE Client Applications

**Web Browser (Chrome/Edge)**:
```javascript
// Web Bluetooth API
navigator.bluetooth.requestDevice({
  filters: [{ name: 'BadgerPen_SmartPen' }]
})
```

**Mobile Apps**:
- iOS: "LightBlue Explorer"
- Android: "nRF Connect"
- Cross-platform: Web Bluetooth in browser

### LED Indicators
- **Double Blink**: Advertising/Discoverable
- **Fast Blink**: Connected + Streaming
- **Slow Blink**: Connected + Paused
- **Very Slow**: Disconnected
- **4 Blinks at startup**: Bluetooth mode selected

### BLE Commands
```json
// Start streaming
{"command": "start"}

// Stop streaming
{"command": "stop"}

// Get status
{"command": "status"}
```

### Data Format
```json
{
  "type": "sensor_data",
  "device_id": "ESP32_BLE_SmartPen",
  "timestamp": 1234567890,
  "gyro_x": 1.23,
  "gyro_y": -0.98,
  "gyro_z": 0.12,
  "accel_x": 8.76,
  "accel_y": -1.54,
  "accel_z": 3.21
}
```

## 🔄 Triple Mode Firmware (All Modes Combined)

For maximum flexibility, use the triple mode firmware:

```
File: esp32_triple_mode_firmware.ino
```

### Mode Switching
- **Short Button Press**: Toggle streaming ON/OFF
- **Long Button Press (3s)**: Switch between modes
- **LED Pattern**: Number of blinks = mode number

### Mode Indicators
1. **1 Blink**: USB Mode
2. **2 Blinks**: WiFi Mode  
3. **3 Blinks**: Bluetooth Mode

## 🔧 Backend Integration

### API Endpoints

**USB Connection**:
```bash
POST /api/devices/connect-usb
{
  "devicePath": "/dev/tty.usbserial-0001",
  "baudRate": "115200"
}
```

**WiFi Connection**:
```bash
POST /api/mpu6050/readings
{
  "device_id": "ESP32_WiFi_SmartPen",
  "timestamp": 1234567890,
  "gyro_x": 1.23, "gyro_y": -0.98, "gyro_z": 0.12,
  "accel_x": 8.76, "accel_y": -1.54, "accel_z": 3.21
}
```

**Device Status**:
```bash
GET /api/devices
```

### Real-time Data Streams
```bash
GET /api/stream/mpu6050/{deviceId}  # Server-Sent Events
```

## 🐛 Troubleshooting

### Common Issues

**ESP32 Not Detected**:
1. Install CP2102 USB drivers
2. Check USB cable (data + power)
3. Verify ESP32 board in Arduino IDE
4. Try different USB port

**No Sensor Data**:
1. Check MPU6050 wiring (SDA=21, SCL=22)
2. Verify I2C address (0x68)
3. Check power supply (3.3V)
4. Test with I2C scanner

**Network Connection Failed**:
1. Verify WiFi credentials
2. Check 2.4GHz network (ESP32 doesn't support 5GHz)
3. Test laptop connectivity
4. Check firewall settings

**Backend Not Receiving Data**:
1. Verify server is running: `lsof -i :3005`
2. Check IP address configuration
3. Test API endpoint: `curl http://192.168.2.13:3005/api/devices`
4. Monitor server logs

### Debug Commands

**Check Network**:
```bash
# Laptop IP
ifconfig | grep "inet " | grep -v 127.0.0.1

# WiFi network
networksetup -getairportnetwork en0

# Server status
lsof -i :3005
curl http://192.168.2.13:3005/api/devices
```

**Check USB Devices**:
```bash
# macOS
ls /dev/tty.usb*

# Linux
ls /dev/ttyUSB*

# Test connection
screen /dev/tty.usbserial-0001 115200
```

## 📋 Performance Comparison

| Mode | Data Rate | Latency | Reliability | Range | Power Usage |
|------|-----------|---------|-------------|--------|-------------|
| USB | 20Hz | ~5ms | Excellent | Cable length | Low |
| WiFi | 10Hz | ~10-50ms | Good | WiFi range | Medium |
| Bluetooth | 8Hz | ~20-100ms | Good | ~10 meters | Low |

## 🔐 Security Considerations

### WiFi Mode
- Uses unencrypted HTTP (local network)
- Consider VPN for remote access
- Firewall protection recommended

### Bluetooth Mode
- BLE pairing provides basic security
- Data transmitted in clear JSON
- Limited to close proximity

### USB Mode
- Most secure (physical connection)
- No network exposure
- Recommended for sensitive data

## 📞 Support

For additional help:
1. Check serial monitor output
2. Verify all connections and configurations
3. Test with simpler examples first
4. Ensure all dependencies are installed

## 🚀 Next Steps

1. **Choose your preferred mode** based on requirements
2. **Update WiFi credentials** if using WiFi mode
3. **Test connection** with backend server
4. **Verify data flow** in web application
5. **Monitor performance** and adjust settings as needed

---

**✅ Success Indicators:**
- ESP32 connects without errors
- Backend receives sensor data
- Web interface shows real-time data
- LED patterns indicate correct mode
- No error messages in logs 