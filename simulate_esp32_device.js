#!/usr/bin/env node

/**
 * ESP32 Smart Pen Simulator
 * Simulates a real ESP32 device sending continuous sensor data
 */

const http = require('http');

const SERVER_HOST = '192.168.2.13';
const SERVER_PORT = 3005;
const DEVICE_ID = 'ESP32_SmartPen_Simulator';
const DATA_RATE = 100; // 10Hz (100ms interval)

console.log('🦡 ESP32 Smart Pen Simulator Starting...');
console.log(`📡 Connecting to: http://${SERVER_HOST}:${SERVER_PORT}`);
console.log(`📱 Device ID: ${DEVICE_ID}`);
console.log(`⏱️  Data Rate: ${1000/DATA_RATE}Hz (${DATA_RATE}ms interval)`);

let dataCount = 0;
let startTime = Date.now();

// Simulate realistic tremor patterns
function generateTremorData() {
  const time = (Date.now() - startTime) / 1000.0;
  
  // Base tremor at 4.5Hz (Parkinson's characteristic frequency)
  const tremorFreq = 4.5;
  const tremorAmplitude = 0.8;
  
  // Add some noise and variation
  const noise = () => (Math.random() - 0.5) * 0.1;
  
  return {
    deviceId: DEVICE_ID,
    timestamp: Date.now(),
    
    // Gyroscope data (degrees/second) - tremor simulation
    gyro_x: tremorAmplitude * Math.sin(2 * Math.PI * tremorFreq * time) + noise(),
    gyro_y: tremorAmplitude * Math.cos(2 * Math.PI * (tremorFreq + 0.3) * time) + noise(),
    gyro_z: 0.3 * Math.sin(2 * Math.PI * (tremorFreq - 0.2) * time) + noise(),
    
    // Accelerometer data (m/s²) - gravity + movement
    accel_x: 0.2 * Math.sin(2 * Math.PI * tremorFreq * time) + noise(),
    accel_y: 0.15 * Math.cos(2 * Math.PI * tremorFreq * time) + noise(),
    accel_z: 9.81 + 0.1 * Math.sin(2 * Math.PI * 3.2 * time) + noise(),
    
    // Additional metadata
    sessionId: 'realtime_session_001',
    movement_type: 'TREMOR'
  };
}

function sendSensorData() {
  const data = generateTremorData();
  const postData = JSON.stringify(data);
  
  const options = {
    hostname: SERVER_HOST,
    port: SERVER_PORT,
    path: '/api/mpu6050/readings',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(postData)
    }
  };
  
  const req = http.request(options, (res) => {
    if (res.statusCode === 201) {
      dataCount++;
      if (dataCount % 10 === 0) {
        console.log(`📊 Sent ${dataCount} readings - Tremor: gyro(${data.gyro_x.toFixed(2)}, ${data.gyro_y.toFixed(2)}, ${data.gyro_z.toFixed(2)}) accel(${data.accel_x.toFixed(2)}, ${data.accel_y.toFixed(2)}, ${data.accel_z.toFixed(2)})`);
      }
    } else {
      console.error(`❌ Error: HTTP ${res.statusCode}`);
    }
  });
  
  req.on('error', (err) => {
    console.error(`❌ Connection error: ${err.message}`);
  });
  
  req.write(postData);
  req.end();
}

// Start continuous data transmission
console.log('🚀 Starting real-time data transmission...');
console.log('📈 Watch the web dashboard for live tremor data!');
console.log('🌐 Open: http://localhost:3000');
console.log('');

const interval = setInterval(sendSensorData, DATA_RATE);

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\n⏹️  Stopping ESP32 simulator...');
  clearInterval(interval);
  console.log(`📊 Total readings sent: ${dataCount}`);
  console.log('✅ ESP32 simulator stopped');
  process.exit(0);
});

// Send initial data immediately
sendSensorData(); 