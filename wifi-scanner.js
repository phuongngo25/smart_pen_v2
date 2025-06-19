#!/usr/bin/env node

/*
 * Smart Pen WiFi Network Scanner
 * Scans the local network to find ESP32 devices and other Smart Pen devices
 */

const { exec } = require('child_process');
const { promisify } = require('util');
const execAsync = promisify(exec);
const os = require('os');

class WiFiNetworkScanner {
    constructor() {
        this.devices = new Map();
        this.scanResults = [];
        this.localIP = this.getLocalIP();
        this.networkBase = this.getNetworkBase();
    }

    // Get local IP address
    getLocalIP() {
        const interfaces = os.networkInterfaces();
        for (const devName in interfaces) {
            const iface = interfaces[devName];
            for (let i = 0; i < iface.length; i++) {
                const alias = iface[i];
                if (alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal) {
                    return alias.address;
                }
            }
        }
        return '192.168.1.100'; // fallback
    }

    // Get network base (e.g., 192.168.1)
    getNetworkBase() {
        return this.localIP.split('.').slice(0, 3).join('.');
    }

    // Ping a single IP address
    async pingIP(ip) {
        try {
            const { stdout } = await execAsync(`ping -c 1 -W 1000 ${ip}`);
            return stdout.includes('1 packets transmitted, 1 received');
        } catch (error) {
            return false;
        }
    }

    // Get device info using ARP
    async getDeviceInfo(ip) {
        try {
            // Try to get MAC address
            const { stdout: arpOutput } = await execAsync(`arp -n ${ip}`);
            const arpLines = arpOutput.split('\n');
            let macAddress = 'Unknown';
            
            for (const line of arpLines) {
                if (line.includes(ip)) {
                    const parts = line.split(/\s+/);
                    for (const part of parts) {
                        if (/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/.test(part)) {
                            macAddress = part.toLowerCase();
                            break;
                        }
                    }
                }
            }

            // Try to get hostname
            let hostname = 'Unknown';
            try {
                const { stdout: hostOutput } = await execAsync(`nslookup ${ip} | grep 'name ='`);
                if (hostOutput) {
                    hostname = hostOutput.split('name = ')[1]?.trim().replace('.', '') || 'Unknown';
                }
            } catch (e) {
                // Hostname lookup failed, that's okay
            }

            return { ip, macAddress, hostname };
        } catch (error) {
            return { ip, macAddress: 'Unknown', hostname: 'Unknown' };
        }
    }

    // Check if device is likely an ESP32
    isESP32Device(deviceInfo) {
        const { macAddress, hostname } = deviceInfo;
        
        // ESP32 MAC address prefixes (Espressif Systems)
        const esp32MacPrefixes = [
            '24:6f:28', '30:ae:a4', '8c:aa:b5', '9c:9c:1f',
            'a4:cf:12', 'b4:e6:2d', 'cc:50:e3', 'd8:a0:1d',
            'dc:a6:32', 'e8:db:84', 'ec:94:cb', 'f0:08:d1'
        ];

        // Check MAC address
        for (const prefix of esp32MacPrefixes) {
            if (macAddress.startsWith(prefix)) {
                return true;
            }
        }

        // Check hostname patterns
        const esp32HostnamePatterns = [
            /esp32/i, /smartpen/i, /badgerpen/i, /espressif/i
        ];

        for (const pattern of esp32HostnamePatterns) {
            if (pattern.test(hostname)) {
                return true;
            }
        }

        return false;
    }

    // Test if device responds to Smart Pen API
    async testSmartPenAPI(ip) {
        try {
            const { stdout } = await execAsync(`curl -s --connect-timeout 2 http://${ip}/health || curl -s --connect-timeout 2 http://${ip}:3005/health || curl -s --connect-timeout 2 http://${ip}:80/status`);
            
            if (stdout.includes('smartpen') || stdout.includes('esp32') || stdout.includes('sensor')) {
                return true;
            }
        } catch (error) {
            // API test failed
        }
        return false;
    }

    // Scan network range
    async scanNetwork() {
        console.log('🔍 Starting Smart Pen WiFi Network Scanner...');
        console.log(`📍 Local IP: ${this.localIP}`);
        console.log(`🌐 Scanning network: ${this.networkBase}.1-254`);
        console.log('');

        const promises = [];
        const activeIPs = [];

        // Ping all IPs in the network range
        for (let i = 1; i <= 254; i++) {
            const ip = `${this.networkBase}.${i}`;
            promises.push(
                this.pingIP(ip).then(isActive => {
                    if (isActive) {
                        activeIPs.push(ip);
                        process.stdout.write('.');
                    }
                })
            );
        }

        await Promise.all(promises);
        console.log(`\n\n✅ Found ${activeIPs.length} active devices on network`);
        console.log('');

        // Get detailed info for each active device
        const devicePromises = activeIPs.map(ip => this.getDeviceInfo(ip));
        const devices = await Promise.all(devicePromises);

        // Test for Smart Pen API and categorize devices
        for (const device of devices) {
            const isESP32 = this.isESP32Device(device);
            const hasSmartPenAPI = await this.testSmartPenAPI(device.ip);
            
            device.deviceType = 'Unknown';
            device.isSmartPen = false;
            device.confidence = 0;

            if (hasSmartPenAPI) {
                device.deviceType = 'Smart Pen Device';
                device.isSmartPen = true;
                device.confidence = 100;
            } else if (isESP32) {
                device.deviceType = 'ESP32 Device';
                device.confidence = 80;
            } else if (device.hostname !== 'Unknown' && device.hostname.toLowerCase().includes('esp')) {
                device.deviceType = 'Possible ESP Device';
                device.confidence = 60;
            } else if (device.ip === this.localIP) {
                device.deviceType = 'Your Computer';
                device.confidence = 100;
            } else {
                device.confidence = 20;
            }

            this.scanResults.push(device);
        }

        return this.scanResults;
    }

    // Display scan results
    displayResults() {
        console.log('📊 NETWORK SCAN RESULTS:');
        console.log('═══════════════════════════════════════════════════════════════════');
        
        // Sort by confidence and Smart Pen devices first
        const sortedResults = this.scanResults.sort((a, b) => {
            if (a.isSmartPen && !b.isSmartPen) return -1;
            if (!a.isSmartPen && b.isSmartPen) return 1;
            return b.confidence - a.confidence;
        });

        sortedResults.forEach((device, index) => {
            const icon = device.isSmartPen ? '🖊️' : 
                        device.deviceType.includes('ESP32') ? '📡' : 
                        device.deviceType.includes('Computer') ? '💻' : '📱';
            
            console.log(`${icon} Device ${index + 1}:`);
            console.log(`   IP Address: ${device.ip}`);
            console.log(`   MAC Address: ${device.macAddress}`);
            console.log(`   Hostname: ${device.hostname}`);
            console.log(`   Device Type: ${device.deviceType}`);
            console.log(`   Confidence: ${device.confidence}%`);
            
            if (device.isSmartPen) {
                console.log(`   🎯 SMART PEN DETECTED! Ready to connect.`);
            }
            console.log('');
        });

        // Summary
        const smartPenDevices = sortedResults.filter(d => d.isSmartPen).length;
        const esp32Devices = sortedResults.filter(d => d.deviceType.includes('ESP32')).length;
        
        console.log('📈 SUMMARY:');
        console.log(`   🖊️  Smart Pen Devices: ${smartPenDevices}`);
        console.log(`   📡 ESP32 Devices: ${esp32Devices}`);
        console.log(`   📱 Total Devices: ${sortedResults.length}`);
        console.log('');

        if (smartPenDevices > 0) {
            console.log('🎉 Smart Pen devices found! You can connect to them now.');
        } else if (esp32Devices > 0) {
            console.log('💡 ESP32 devices found. Flash Smart Pen firmware to use them.');
        } else {
            console.log('ℹ️  No Smart Pen devices found. Upload firmware to your ESP32.');
        }
    }

    // Get connection instructions for Smart Pen devices
    getConnectionInstructions() {
        const smartPenDevices = this.scanResults.filter(d => d.isSmartPen);
        
        if (smartPenDevices.length > 0) {
            console.log('');
            console.log('🔧 CONNECTION INSTRUCTIONS:');
            console.log('═══════════════════════════════════════════════════════════════════');
            
            smartPenDevices.forEach((device, index) => {
                console.log(`📡 Smart Pen Device ${index + 1}:`);
                console.log(`   WebSocket URL: ws://${device.ip}:3005`);
                console.log(`   HTTP API: http://${device.ip}:3005/health`);
                console.log(`   Add to your backend configuration:`);
                console.log(`   DEVICE_${index + 1}_IP=${device.ip}`);
                console.log('');
            });
        }
    }
}

// CLI Usage
async function main() {
    console.log('🖊️  SMART PEN WIFI NETWORK SCANNER');
    console.log('═══════════════════════════════════════════════════════════════════');
    console.log('Scanning for ESP32 and Smart Pen devices on your WiFi network...');
    console.log('');

    const scanner = new WiFiNetworkScanner();
    
    try {
        await scanner.scanNetwork();
        scanner.displayResults();
        scanner.getConnectionInstructions();
        
        console.log('✅ Network scan completed successfully!');
        
        // Return results for API usage
        return scanner.scanResults;
    } catch (error) {
        console.error('❌ Error during network scan:', error.message);
        process.exit(1);
    }
}

// Export for API usage
module.exports = { WiFiNetworkScanner };

// Run if called directly
if (require.main === module) {
    main();
} 