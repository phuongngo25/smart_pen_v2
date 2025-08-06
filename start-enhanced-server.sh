#!/bin/bash

# Smart Pen v2.0 Enhanced Server Startup Script
# Includes MPU6050 sensor data processing and Parkinson's disease detection

echo "🦡 Starting Smart Pen v2.0 Enhanced Server..."
echo "================================================"
echo "Features:"
echo "  ✅ MPU6050 6-axis sensor data processing"
echo "  ✅ Real-time 3D pen movement tracking"
echo "  ✅ AI-powered tremor analysis"
echo "  ✅ Parkinson's disease detection"
echo "  ✅ Clinical reporting and analytics"
echo "================================================"

# Set environment variables
export NODE_ENV=development
export PORT=3001

# Navigate to backend directory
cd backend

# Check if enhanced server exists
if [ ! -f "enhanced-simple-server.js" ]; then
    echo "❌ Enhanced server file not found!"
    echo "   Please ensure enhanced-simple-server.js exists in the backend directory"
    exit 1
fi

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
    echo "📦 Installing dependencies..."
    npm install
fi

# Start the enhanced server
echo "🚀 Starting enhanced server on port $PORT..."
node enhanced-simple-server.js 