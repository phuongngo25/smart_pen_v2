#!/bin/bash

# Smart Pen v2.0 Enhanced Startup Script
# 🦡 Badger Team - Advanced Parkinson's Disease Detection System

echo "🦡 Starting Smart Pen v2.0 Enhanced with Parkinson's Detection..."
echo "================================================="

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to check if port is in use
check_port() {
    if lsof -Pi :$1 -sTCP:LISTEN -t >/dev/null ; then
        echo -e "${RED}❌ Port $1 is already in use${NC}"
        return 1
    else
        echo -e "${GREEN}✅ Port $1 is available${NC}"
        return 0
    fi
}

# Function to kill processes on specific ports
cleanup_ports() {
    echo -e "${YELLOW}🧹 Cleaning up existing processes...${NC}"
    
    # Kill processes on our target ports
    lsof -ti:3005 | xargs kill -9 2>/dev/null || true
    lsof -ti:3004 | xargs kill -9 2>/dev/null || true
    lsof -ti:3000 | xargs kill -9 2>/dev/null || true
    
    # Kill any existing Smart Pen processes
    pkill -f "enhanced-simple-server" 2>/dev/null || true
    pkill -f "npm.*dev" 2>/dev/null || true
    pkill -f "vite" 2>/dev/null || true
    
    sleep 2
    echo -e "${GREEN}✅ Cleanup complete${NC}"
}

# Cleanup first
cleanup_ports

# Check required ports
echo -e "${BLUE}🔍 Checking port availability...${NC}"
check_port 3005 # Enhanced Backend
check_port 3004 # Frontend (mapped)

# Start Enhanced Backend Server
echo -e "${BLUE}🚀 Starting Enhanced Backend Server on port 3005...${NC}"
cd backend
# Set environment variables for the enhanced server
export NODE_ENV=development
export PORT=3005
export CORS_ORIGIN="http://localhost:3004,http://localhost:3000"

# Start enhanced server in background
node enhanced-simple-server.js > ../logs/backend-enhanced.log 2>&1 &
BACKEND_PID=$!

# Wait for backend to start
echo -e "${YELLOW}⏳ Waiting for backend to start...${NC}"
sleep 5

# Check if backend is running
if curl -s http://localhost:3005/health > /dev/null; then
    echo -e "${GREEN}✅ Enhanced Backend started successfully on port 3005${NC}"
    echo -e "${GREEN}   📊 MPU6050 Sensor Data Processing${NC}"
    echo -e "${GREEN}   🎯 3D Movement Tracking${NC}"
    echo -e "${GREEN}   🧠 AI Tremor Analysis${NC}"
    echo -e "${GREEN}   🔬 Parkinson's Disease Detection${NC}"
else
    echo -e "${RED}❌ Backend failed to start${NC}"
    echo "Backend logs:"
    tail -20 ../logs/backend-enhanced.log
    exit 1
fi

# Start Frontend Server
echo -e "${BLUE}🚀 Starting Frontend Server...${NC}"
cd ../frontend

# Set environment variables for frontend
export VITE_API_URL=http://localhost:3005/api
export VITE_WS_URL=ws://localhost:3005
export VITE_SSE_URL=http://localhost:3005/api/stream

# Install dependencies if needed
if [ ! -d "node_modules" ]; then
    echo -e "${YELLOW}📦 Installing frontend dependencies...${NC}"
    npm install
fi

# Start frontend development server
npm run dev > ../logs/frontend.log 2>&1 &
FRONTEND_PID=$!

# Wait for frontend to start
echo -e "${YELLOW}⏳ Waiting for frontend to start...${NC}"
sleep 10

# Check if frontend is running
if curl -s http://localhost:3000/ > /dev/null; then
    echo -e "${GREEN}✅ Frontend started successfully${NC}"
    FRONTEND_PORT="3000"
elif curl -s http://localhost:3004/ > /dev/null; then
    echo -e "${GREEN}✅ Frontend started successfully${NC}"  
    FRONTEND_PORT="3004"
else
    echo -e "${RED}❌ Frontend failed to start${NC}"
    echo "Frontend logs:"
    tail -20 ../logs/frontend.log
    exit 1
fi

# Create logs directory if it doesn't exist
mkdir -p ../logs

# Success message
echo -e "${GREEN}"
echo "🎉 Smart Pen v2.0 Enhanced System Started Successfully!"
echo "================================================="
echo "🦡 Badger Team - Advanced Parkinson's Detection"
echo ""
echo "📱 Frontend (Badger UI):     http://localhost:${FRONTEND_PORT}"
echo "🔧 Enhanced Backend API:    http://localhost:3005"
echo "📊 API Health Check:        http://localhost:3005/health"
echo "🔄 Real-time Stream:        http://localhost:3005/api/stream/mpu6050/device1"
echo ""
echo "🔐 Demo Login Credentials:"
echo "   👤 Patient: patient@example.com / password123"
echo "   👩‍⚕️ Doctor:  doctor@example.com / password123"
echo "   👨‍💼 Admin:   admin@example.com / password123"
echo ""
echo "🆕 Features in Enhanced Version:"
echo "   • 🔬 Real-time Parkinson's Disease Detection"
echo "   • 📈 Advanced MPU6050 6-axis Sensor Processing" 
echo "   • 🎯 3D Pen Movement Visualization"
echo "   • 🧠 AI-powered Tremor Analysis"
echo "   • 📊 Clinical Reporting & Scoring"
echo "   • 🎨 Beautiful Badger-themed UI"
echo ""
echo "📝 Process IDs:"
echo "   Backend PID: $BACKEND_PID"
echo "   Frontend PID: $FRONTEND_PID"
echo ""
echo "🛑 To stop all services: ./stop-smart-pen-enhanced.sh"
echo "📋 Logs are saved in: ./logs/"
echo -e "${NC}"

# Save PIDs for cleanup script
echo "$BACKEND_PID" > ../logs/backend.pid
echo "$FRONTEND_PID" > ../logs/frontend.pid
echo "3005" > ../logs/backend.port
echo "${FRONTEND_PORT}" > ../logs/frontend.port

echo -e "${BLUE}🔍 System is ready! Open http://localhost:${FRONTEND_PORT} in your browser${NC}"
echo -e "${YELLOW}💡 Tip: Check ./logs/ for detailed server logs${NC}" 