#!/bin/bash

# Smart Pen v2.0 Enhanced Stop Script
# 🦡 Badger Team - Advanced Parkinson's Disease Detection System

echo "🦡 Stopping Smart Pen v2.0 Enhanced System..."
echo "============================================="

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Function to stop process by PID
stop_process() {
    local pid=$1
    local name=$2
    
    if [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null; then
        echo -e "${YELLOW}🛑 Stopping $name (PID: $pid)...${NC}"
        kill "$pid" 2>/dev/null || kill -9 "$pid" 2>/dev/null
        sleep 2
        
        if kill -0 "$pid" 2>/dev/null; then
            echo -e "${RED}❌ Force killing $name (PID: $pid)...${NC}"
            kill -9 "$pid" 2>/dev/null
        else
            echo -e "${GREEN}✅ $name stopped successfully${NC}"
        fi
    else
        echo -e "${BLUE}ℹ️  $name is not running${NC}"
    fi
}

# Stop processes using saved PIDs
if [ -f "logs/backend.pid" ]; then
    BACKEND_PID=$(cat logs/backend.pid)
    stop_process "$BACKEND_PID" "Enhanced Backend Server"
    rm -f logs/backend.pid
fi

if [ -f "logs/frontend.pid" ]; then
    FRONTEND_PID=$(cat logs/frontend.pid)
    stop_process "$FRONTEND_PID" "Frontend Server"
    rm -f logs/frontend.pid
fi

# Kill any remaining processes by name
echo -e "${YELLOW}🧹 Cleaning up remaining processes...${NC}"

pkill -f "enhanced-simple-server" 2>/dev/null && echo -e "${GREEN}✅ Stopped enhanced-simple-server${NC}" || true
pkill -f "npm.*dev" 2>/dev/null && echo -e "${GREEN}✅ Stopped npm dev servers${NC}" || true
pkill -f "vite" 2>/dev/null && echo -e "${GREEN}✅ Stopped Vite servers${NC}" || true

# Kill processes on specific ports
for port in 3005 3004 3000; do
    if lsof -ti:$port >/dev/null 2>&1; then
        echo -e "${YELLOW}🛑 Stopping process on port $port...${NC}"
        lsof -ti:$port | xargs kill -9 2>/dev/null || true
        echo -e "${GREEN}✅ Port $port freed${NC}"
    fi
done

# Clean up port files
rm -f logs/backend.port logs/frontend.port

sleep 2

echo -e "${GREEN}"
echo "🎉 Smart Pen v2.0 Enhanced System Stopped Successfully!"
echo "============================================="
echo "🦡 All Badger Team services have been stopped"
echo ""
echo "📋 Log files preserved in: ./logs/"
echo "🚀 To restart: ./start-smart-pen-enhanced.sh"
echo -e "${NC}"

# Verify all processes are stopped
echo -e "${BLUE}🔍 Verifying all processes are stopped...${NC}"
remaining=$(ps aux | grep -E "(enhanced-simple-server|npm.*dev|vite)" | grep -v grep | wc -l)

if [ "$remaining" -eq 0 ]; then
    echo -e "${GREEN}✅ All Smart Pen processes stopped${NC}"
else
    echo -e "${YELLOW}⚠️  Some processes may still be running:${NC}"
    ps aux | grep -E "(enhanced-simple-server|npm.*dev|vite)" | grep -v grep || true
fi 