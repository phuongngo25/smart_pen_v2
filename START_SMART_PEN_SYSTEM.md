# 🦡 Smart Pen System - Quick Start Guide

## 🚀 **Step 1: Start the Backend Server**

```bash
# Navigate to project directory
cd /Users/phuongngo/Code/Smart_pen_v2.0

# Start the enhanced backend server
node backend/enhanced-simple-server.js

# Expected output:
# 🌐 Laptop IP Address: 192.168.2.13:3005
# 📡 ESP32 WiFi devices should send data to: http://192.168.2.13:3005/api/mpu6050/readings
# 🦡 Enhanced Smart Pen Server running on:
#    Local:    http://localhost:3005
#    Network:  http://192.168.2.13:3005
```

## 🔥 **Step 2: Configure macOS Firewall**

```bash
# Option A: Using System Preferences
# 1. Open System Preferences → Security & Privacy → Firewall
# 2. Click "Firewall Options"
# 3. Add "node" or your server application
# 4. Set to "Allow incoming connections"

# Option B: Using Terminal (if firewall is enabled)
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --add /usr/local/bin/node
sudo /usr/libexec/ApplicationFirewall/socketfilterfw --unblockapp /usr/local/bin/node
```

## 📱 **Step 3: Configure ESP32 Device**

### **Option A: Using Web Setup (Recommended)**
1. Flash the ESP32 with updated firmware (see below)
2. Connect to WiFi network: `ESP32_SmartPen_Setup`
3. Password: `smartpen2024`
4. Open browser to: `http://192.168.4.1`
5. Enter your WiFi credentials and laptop IP: `192.168.2.13`

### **Option B: Update Firmware Manually**
Update the ESP32 code with correct IP:

```cpp
// In esp32_wifi_mode.ino, change line ~22:
String LAPTOP_IP = "192.168.2.13";  // Your actual IP
String WIFI_SSID = "YourActualWiFiName";  // Your WiFi network name

// Verify the common IPs array includes your subnet:
String commonIPs[] = {
  "192.168.2.13", "192.168.2.100", // ✅ Your subnet
  "192.168.1.100", "192.168.1.101", 
  // ... rest
};
```

## 🔧 **Step 4: Test Connection**

```bash
# Test 1: Check if server responds
curl http://192.168.2.13:3005/api/health

# Expected response:
# {"status":"ok","message":"Enhanced Smart Pen API with Parkinson's Detection is running perfectly!"}

# Test 2: Check device scanning
curl http://192.168.2.13:3005/api/devices/scan-usb

# Test 3: Send test sensor data (simulate ESP32)
curl -X POST http://192.168.2.13:3005/api/mpu6050/readings \
  -H "Content-Type: application/json" \
  -d '{"device_id":"ESP32_Test","gyro_x":1.0,"gyro_y":2.0,"gyro_z":3.0,"accel_x":0.1,"accel_y":0.2,"accel_z":9.8}'
```

## 🌐 **Step 5: Start Frontend (Optional)**

```bash
# In a new terminal:
cd frontend
npm start

# Open browser to: http://localhost:3000
# Login with: patient@badgerpen.com / badger123
```

## 📊 **Step 6: Verify Complete System**

### **WiFi Connection Test:**
1. ESP32 sends data → Backend receives → Frontend displays
2. Check backend logs for: `📊 WiFi sensor data from ESP32_WiFi_SmartPen`
3. Check frontend Device Manager for connected device

### **USB Connection Test:**
1. Connect ESP32 via USB cable
2. Backend should detect: `✅ Found 1 VERIFIED REAL devices`
3. Click "Connect" in frontend Device Manager

### **Troubleshooting:**

**Problem**: ESP32 not connecting to WiFi
```
Solution:
1. Check WiFi credentials in ESP32 code
2. Ensure ESP32 and laptop on same network
3. Check router settings (AP isolation disabled)
```

**Problem**: "Connection refused" errors
```
Solution:
1. Verify backend server is running on port 3005
2. Check firewall settings
3. Confirm IP address matches ifconfig output
```

**Problem**: Data not appearing in frontend
```
Solution:
1. Check browser console for errors
2. Verify WebSocket connections in DevTools
3. Confirm user is logged in with correct role
```

## 🎯 **Expected Results After Setup:**

✅ **Backend**: Server running on `http://192.168.2.13:3005`
✅ **ESP32 WiFi**: Connected and sending data every 100ms
✅ **ESP32 USB**: Detected and streaming via serial port
✅ **Frontend**: Real-time data visualization and device management
✅ **Authentication**: Role-based access working properly
✅ **Data Flow**: Sensor → Backend → Database → Frontend → User

## 🔄 **Quick Commands Summary:**

```bash
# Start everything:
node backend/enhanced-simple-server.js &
cd frontend && npm start &

# Check connections:
curl http://192.168.2.13:3005/api/health
netstat -an | grep ":3005"

# View real-time data:
curl http://192.168.2.13:3005/api/mpu6050/readings
``` 