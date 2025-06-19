# 🦡 BadgerPen Smart Pen - Triple Mode Connection Setup Guide

## 📡 **Triple Communication System Overview**

The BadgerPen Smart Pen v3.0 now supports **three connection modes** for maximum flexibility and reliability:

1. **🔌 USB/Wired Connection** - Direct serial communication (most reliable)
2. **📶 WiFi Connection** - Wireless HTTP communication via local network  
3. **🔵 Bluetooth Connection** - Low Energy (BLE) wireless communication

## 🚀 **Quick Start - Getting Started**

### 1. Start the System
```bash
# Terminal 1: Start backend server
cd backend
node enhanced-simple-server.js

# Terminal 2: Start frontend
cd frontend  
npm start
```

### 2. Access Device Setup
- Open browser: `http://localhost:3000`
- Navigate to: **Patient → Device Setup**
- Choose your preferred connection mode

## 🔌 **USB/Wired Connection Setup**

### Prerequisites
- ESP32 device with USB cable
- CP2102 or CH340 USB drivers installed

### Connection Steps
1. **Connect Hardware**
   - Connect ESP32 to computer via USB cable
   - Ensure high-quality USB cable for stable connection

2. **Scan for Devices**
   - Click "Scan USB Ports" in the Device Setup page
   - System will detect all compatible USB devices
   - Look for devices like `/dev/tty.usbserial-*` (macOS) or `COM*` (Windows)

3. **Configure Connection**
   - Select the correct USB port from dropdown
   - Set baud rate to **115200** (recommended)
   - Click "Connect USB Device"

4. **Verify Connection**
   - Device should appear in "Connected Devices" section
   - Real-time sensor data should start flowing
   - LED on ESP32 should blink to indicate active streaming

### Troubleshooting USB Connection
- **Device not detected**: Install CP2102/CH340 drivers
- **Connection failed**: Try different USB port or cable
- **No data**: Check ESP32 firmware and baud rate
- **Intermittent connection**: Use high-quality USB cable

## 📶 **WiFi Connection Setup**

### Prerequisites
- ESP32 with WiFi capability
- Computer and ESP32 on same WiFi network
- Backend server running and accessible

### Connection Steps

#### Option A: Direct WiFi Configuration
1. **Flash WiFi Firmware**
   - Upload `esp32_triple_mode_firmware.ino` to ESP32
   - ESP32 will start in Access Point mode

2. **Configure ESP32 WiFi**
   - Connect to ESP32 AP: `ESP32_SmartPen_Setup` (password: `smartpen2024`)
   - Open browser: `http://192.168.4.1`
   - Enter your WiFi credentials and server details
   - ESP32 will restart and connect to your network

3. **Scan and Connect via Web Interface**
   - In Device Setup page, click "Scan WiFi Networks"
   - Select your network from the list
   - Enter WiFi password
   - Click "Connect to WiFi"

#### Option B: Manual WiFi Setup
1. **Update ESP32 Firmware**
   - Modify WiFi credentials in firmware:
   ```cpp
   String wifi_ssid = "YourWiFiName";
   String wifi_password = "YourWiFiPassword";
   String server_ip = "192.168.1.XXX";  // Your computer's IP
   ```

2. **Upload and Connect**
   - Flash modified firmware to ESP32
   - ESP32 will auto-connect to WiFi
   - Device should appear in web interface

### WiFi Troubleshooting
- **ESP32 not connecting**: Check WiFi credentials and signal strength
- **No data received**: Verify server IP address and firewall settings
- **Connection drops**: Ensure stable WiFi signal and power supply
- **Cannot access setup page**: Try `192.168.4.1` when connected to ESP32 AP

## 🔵 **Bluetooth Connection Setup**

### Prerequisites
- ESP32 with Bluetooth Low Energy (BLE) support
- Computer with Bluetooth capability
- Bluetooth enabled in system settings

### Connection Steps
1. **Enable Bluetooth**
   - In Device Setup, toggle "Bluetooth Enabled" switch
   - System will automatically enable Bluetooth scanning

2. **Scan for Devices**
   - Click "Scan Bluetooth Devices"
   - Wait for nearby BLE devices to be discovered
   - Look for devices named "BadgerPen Smart Pen" or "ESP32 SmartPen BLE"

3. **Pair and Connect**
   - Click "Pair & Connect" next to your device
   - Enter PIN if prompted (usually `1234` or `0000`)
   - Device should appear as "Connected" with signal strength

4. **Verify Connection**
   - Device appears in Connected Devices list
   - Signal strength indicator shows connection quality
   - Data should start flowing via BLE notifications

### Bluetooth Troubleshooting
- **No devices found**: Ensure ESP32 is in pairing mode and close to computer
- **Pairing failed**: Clear Bluetooth cache and retry
- **Connection unstable**: Move devices closer together
- **No data**: Check BLE characteristic subscriptions

## ⚙️ **Device Setup Interface Guide**

### Connected Devices Overview
- **Real-time status** of all connected devices
- **Connection type indicators**: USB, WiFi, Bluetooth icons
- **Signal strength** and **battery level** (when available)
- **Quick disconnect** buttons for each device

### Connection Tabs

#### 🔌 USB/Wired Tab
- **USB Port Scanner**: Detects all compatible serial devices
- **Port Selection**: Choose correct COM/tty port
- **Baud Rate**: Configure communication speed (115200 recommended)
- **Connection Guide**: Step-by-step USB setup instructions

#### 📶 WiFi Tab
- **Network Scanner**: Discovers available WiFi networks
- **Signal Strength**: Shows WiFi signal quality
- **Password Dialog**: Secure credential entry
- **Setup Instructions**: WiFi configuration guidelines

#### 🔵 Bluetooth Tab
- **Device Scanner**: Finds nearby BLE devices
- **Pairing Status**: Shows paired and connected devices
- **Signal Strength**: RSSI indicator for connection quality
- **Setup Guide**: Bluetooth pairing instructions

## 🔄 **Multi-Mode Operation**

### Simultaneous Connections
The system supports **multiple simultaneous connections**:
- **USB + WiFi**: Maximum reliability with wireless backup
- **USB + Bluetooth**: Wired primary with BLE secondary
- **WiFi + Bluetooth**: Full wireless operation
- **All Three**: Ultimate redundancy and data collection

### Data Priority
1. **USB**: Highest priority, most reliable (10Hz)
2. **WiFi**: Medium priority, good bandwidth (5Hz)  
3. **Bluetooth**: Lower priority, power efficient (4Hz)

### Automatic Failover
- If primary connection fails, system automatically switches to backup
- Data continues flowing through available connections
- Connection status updates in real-time

## 📊 **Data Verification**

### Real-Time Monitoring
- **Live Data Counter**: Shows incoming sensor readings
- **Connection Status**: Green/Red indicators for each mode
- **3D Visualization**: Real-time pen movement display
- **Export Function**: Download collected sensor data

### Data Format
All connections provide consistent sensor data:
```json
{
  "gyro_x": 1.374,    // Gyroscope X (°/s)
  "gyro_y": -0.847,   // Gyroscope Y (°/s)  
  "gyro_z": 0.229,    // Gyroscope Z (°/s)
  "accel_x": 8.943,   // Accelerometer X (m/s²)
  "accel_y": -1.521,  // Accelerometer Y (m/s²)
  "accel_z": 3.264,   // Accelerometer Z (m/s²)
  "timestamp": "2024-01-18T19:55:52Z",
  "deviceId": "BadgerPen_ESP32_v3_WiFi",
  "connectionType": "wifi"
}
```

## 🛠️ **Advanced Configuration**

### ESP32 Firmware Modes
The ESP32 firmware supports **7 connection modes**:
- `0`: USB Only
- `1`: WiFi Only  
- `2`: Bluetooth Only
- `3`: USB + WiFi (default)
- `4`: USB + Bluetooth
- `5`: WiFi + Bluetooth
- `6`: All Three

### Button Controls on ESP32
- **Short Press**: Toggle data streaming ON/OFF
- **Long Press (3s)**: Cycle through connection modes
- **LED Indicators**: 
  - Fast blink: Streaming active
  - Slow blink: Streaming paused
  - Rapid blinks: Mode change

### Server Configuration
Backend server automatically handles all connection types:
```javascript
// Endpoints for each connection mode
GET  /api/devices/scan-usb        // USB device discovery
GET  /api/devices/scan-wifi       // WiFi network discovery  
GET  /api/devices/scan-bluetooth  // Bluetooth device discovery
POST /api/devices/connect-usb     // USB connection
POST /api/devices/connect-wifi    // WiFi connection
POST /api/devices/connect-bluetooth // Bluetooth connection
```

## 🔍 **Troubleshooting Common Issues**

### General Connection Issues
1. **No devices detected**: Check hardware connections and drivers
2. **Data not flowing**: Verify device streaming is enabled
3. **Connection unstable**: Check power supply and signal strength
4. **Multiple connections conflict**: Use different device IDs

### Performance Optimization
- **USB**: Use high-quality cables, avoid USB hubs
- **WiFi**: Ensure strong signal, minimal interference  
- **Bluetooth**: Keep devices close, avoid interference from other BLE devices
- **Server**: Ensure backend server has sufficient resources

### Debug Information
Enable debug mode in browser console to see:
- API request/response details
- Connection establishment logs
- Data flow statistics
- Error messages and stack traces

## 📱 **Mobile and Cross-Platform Support**

### Web-Based Interface
- **Responsive design** works on desktop, tablet, and mobile
- **Touch-friendly** controls for mobile device setup
- **Progressive Web App** capabilities for offline use

### Platform Compatibility
- **Windows**: USB (COM ports), WiFi, Bluetooth
- **macOS**: USB (/dev/tty.*), WiFi, Bluetooth  
- **Linux**: USB (/dev/ttyUSB*), WiFi, Bluetooth
- **Mobile**: WiFi and Bluetooth only (no USB)

## 🚀 **Next Steps**

After successful connection setup:
1. **Verify Data Flow**: Check real-time sensor readings
2. **Test 3D Visualization**: Move device and observe virtual pen movement
3. **Export Data**: Download sensor data for analysis
4. **Configure Alerts**: Set up connection monitoring
5. **Optimize Performance**: Fine-tune connection settings for your environment

## 📞 **Support and Resources**

- **Documentation**: `/docs` folder for technical details
- **API Reference**: `/backend/API_REFERENCE.md`
- **Firmware Source**: `/esp32/src/main.cpp`
- **Frontend Components**: `/frontend/src/pages/Patient/DeviceSetup.tsx`

---

**🦡 BadgerPen Team** - Advanced Parkinson's Disease Monitoring System 