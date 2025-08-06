# ESP32 Backend Connection Setup Guide

This guide will help you set up an ESP32 device to send data to your Smart Pen backend server.

## 🛠️ Prerequisites

### Hardware Required
- ESP32 Development Board (ESP32-DevKitC, WROOM, or similar)
- USB Cable (Type-C or Micro-USB depending on your board)
- Computer with Arduino IDE
- WiFi network

### Software Required
- Arduino IDE (latest version)
- ArduinoJson library (version 6.x)
- ESP32 board package for Arduino IDE

## 📦 Arduino IDE Setup

### 1. Install ESP32 Board Package
1. Open Arduino IDE
2. Go to **File > Preferences**
3. Add this URL to "Additional Boards Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to **Tools > Board > Boards Manager**
5. Search for "ESP32" and install "ESP32 by Espressif Systems"

### 2. Install Required Libraries
1. Go to **Tools > Manage Libraries**
2. Search and install:
   - **ArduinoJson** (by Benoit Blanchon) - Version 6.x
   - **WiFi** (usually pre-installed with ESP32 package)

### 3. Select Board Configuration
1. Go to **Tools > Board**
2. Select your ESP32 board (e.g., "ESP32 Dev Module")
3. Set the following settings:
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB
   - Partition Scheme: Default 4MB with spiffs

## 🔧 Hardware Setup

### Pin Connections
The code uses these pins by default:
- **LED_PIN (GPIO 2)**: Built-in LED for status indication
- **BUTTON_PIN (GPIO 0)**: Boot button for starting/stopping data stream

### LED Status Indicators
- **Fast Blink (200ms)**: No WiFi connection
- **Medium Blink (500ms)**: WiFi connected, but device not registered
- **Slow Blink (2000ms)**: Connected and registered, ready to stream
- **Very Fast Blink (100ms)**: Actively streaming data

## 💻 Backend Server Setup

### 1. Start the Backend Server
Make sure your backend server is running on your computer:

```bash
# Navigate to backend directory
cd backend

# Install dependencies (if not done already)
npm install

# Start the development server
npm run dev
```

The server should start on port 3005. You can verify it's running by visiting:
```
http://localhost:3005/health
```

### 2. Find Your Computer's IP Address

#### On Windows:
```cmd
ipconfig
```
Look for "IPv4 Address" under your WiFi adapter.

#### On Mac/Linux:
```bash
ifconfig
```
Look for your WiFi interface (usually `en0` on Mac or `wlan0` on Linux).

**Example:** If your IP is `192.168.1.105`, that's what you'll use in the ESP32 code.

## 📝 ESP32 Code Configuration

### 1. Download the Code
Use the file `smart_pen_wifi_sender.ino` provided.

### 2. Update Configuration
Open the file and modify these settings at the top:

```cpp
// WiFi Settings
const char* WIFI_SSID = "YOUR_WIFI_NAME";        // Your WiFi network name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password

// Backend Server Settings  
const char* SERVER_IP = "192.168.1.105";         // Your computer's IP address
const int SERVER_PORT = 3005;                    // Backend server port (usually 3005)
```

**Important:** 
- Replace `YOUR_WIFI_NAME` with your actual WiFi network name
- Replace `YOUR_WIFI_PASSWORD` with your actual WiFi password
- Replace `192.168.1.105` with your computer's actual IP address

### 3. Upload to ESP32
1. Connect ESP32 to computer via USB
2. Select correct COM port in **Tools > Port**
3. Click **Upload** button (arrow icon)
4. Wait for "Done uploading" message

## 🚀 Testing the Connection

### 1. Monitor Serial Output
1. Open **Tools > Serial Monitor**
2. Set baud rate to **115200**
3. Reset the ESP32 (press EN button)
4. You should see startup messages like:

```
🦡 Smart Pen ESP32 - Backend Sender
Firmware: 2.1.5
==================================================

📋 Configuration:
WiFi SSID: YourWiFiName
Server IP: 192.168.1.105
Server Port: 3005
Backend URL: http://192.168.1.105:3005
Device ID: SmartPen_Test_001_A1B2C3
Serial Number: SPA1B2C3

🌐 Connecting to WiFi: YourWiFiName
......
✅ WiFi connected!
IP address: 192.168.1.200
Signal strength: -45 dBm

📝 Registering device with backend...
✅ Device registered successfully!

✅ Setup complete!
Press the BOOT button to start/stop data streaming
```

### 2. Start Data Streaming
1. Press the **BOOT button** (GPIO 0) on your ESP32
2. You should see in Serial Monitor:
```
🚀 Starting data streaming...
📊 Data #1 sent successfully
💓 Heartbeat sent (Battery: 89%, Signal: -45 dBm)
📊 Data #2 sent successfully
📊 Data #3 sent successfully
```

### 3. Verify Backend Reception
Check your backend server logs for incoming data:
```bash
# In backend terminal, you should see:
[INFO] ESP32 device registration attempt: SPA1B2C3
[INFO] ESP32 device registered: xxx-xxx-xxx (SPA1B2C3)
[INFO] Heartbeat received from device SPA1B2C3
```

## 🔍 Troubleshooting

### WiFi Connection Issues
**Problem**: ESP32 can't connect to WiFi
**Solutions**:
- Double-check WiFi credentials (case-sensitive)
- Ensure WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Try moving ESP32 closer to router
- Check if network has MAC address filtering

### Backend Connection Issues
**Problem**: Device registration fails
**Solutions**:
1. Verify backend server is running:
   ```bash
   curl http://localhost:3005/health
   ```

2. Check if computer's IP is correct:
   ```bash
   # Test from another device on same network
   curl http://YOUR_COMPUTER_IP:3005/health
   ```

3. Check firewall settings - port 3005 should be open

4. Try connecting ESP32 to same network as your computer

### Data Not Appearing in Frontend
**Problem**: ESP32 sends data but it doesn't show in web interface
**Solutions**:
1. Check if frontend is connected to correct backend URL
2. Verify user is logged in and can see devices
3. Check browser console for errors
4. Ensure real-time updates are working (SSE connection)

### Serial Monitor Issues
**Problem**: No output in Serial Monitor
**Solutions**:
- Check baud rate is set to 115200
- Verify correct COM port is selected
- Try pressing EN (reset) button on ESP32
- Check USB cable (some are power-only)

## 📊 Backend API Endpoints Used

The ESP32 code uses these backend endpoints:

1. **Device Registration**
   - `POST /api/esp32/register`
   - Registers the device with backend

2. **Sensor Data Submission**
   - `POST /api/esp32/sensor-data`
   - Sends individual sensor readings

3. **Heartbeat**
   - `POST /api/esp32/heartbeat`
   - Sends device status and health info

4. **Health Check**
   - `GET /health`
   - Used to verify backend connectivity

## 🎯 Expected Data Flow

1. **ESP32 Startup**: Device generates unique ID based on MAC address
2. **WiFi Connection**: Connects to specified network
3. **Device Registration**: Registers with backend server
4. **Data Streaming**: When button pressed, starts sending:
   - Accelerometer data (X, Y, Z)
   - Gyroscope data (X, Y, Z)
   - Temperature readings
   - Timestamps
5. **Heartbeat**: Sends device status every 30 seconds
6. **Backend Processing**: Server stores data and forwards to frontend
7. **Real-time Display**: Data appears in web interface

## 🔄 Real-time Monitoring

### ESP32 Side
Monitor Serial output for:
- WiFi connection status
- Device registration success
- Data transmission confirmations
- Error messages

### Backend Side
Check backend logs for:
- Device registration events
- Incoming sensor data
- Heartbeat messages
- Any errors or warnings

### Frontend Side
Verify in web interface:
- Device appears in device list
- Real-time data updates
- Charts and graphs populate
- No connection errors

## 📈 Performance Optimization

### For Better Performance:
1. **Batch Data**: Use batch endpoint for multiple readings
2. **Adjust Intervals**: Modify timing based on needs
3. **Error Handling**: Implement retry logic for failed requests
4. **Power Management**: Add sleep modes for battery operation

### Monitoring Metrics:
- Data transmission success rate
- WiFi signal strength
- Memory usage
- Network latency

## 🔐 Security Considerations

For production use, consider:
1. **HTTPS**: Use secure connections
2. **Authentication**: Add device authentication
3. **Encryption**: Encrypt sensitive data
4. **Access Control**: Limit backend access
5. **Firmware Updates**: Implement OTA updates

---

## ✅ Quick Start Checklist

- [ ] Arduino IDE installed with ESP32 package
- [ ] ArduinoJson library installed
- [ ] Backend server running on port 3005
- [ ] Computer's IP address identified
- [ ] WiFi credentials updated in code
- [ ] Server IP updated in code
- [ ] Code uploaded to ESP32
- [ ] Serial Monitor shows successful connection
- [ ] Device registration successful
- [ ] Data streaming started with button press
- [ ] Backend receives data (check logs)
- [ ] Frontend shows device and data

If all checkboxes are completed, your ESP32 should be successfully sending data to your backend! 🎉 