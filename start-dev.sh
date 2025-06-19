#!/bin/bash

echo "🚀 Starting Smart Pen v2.0 Development Environment"
echo "================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if a port is in use
port_in_use() {
    lsof -i:$1 >/dev/null 2>&1
}

# Check prerequisites
echo -e "${BLUE}Checking prerequisites...${NC}"

if ! command_exists node; then
    echo -e "${RED}❌ Node.js is not installed${NC}"
    echo "Please install Node.js from https://nodejs.org/"
    exit 1
fi

if ! command_exists npm; then
    echo -e "${RED}❌ npm is not installed${NC}"
    echo "Please install npm (usually comes with Node.js)"
    exit 1
fi

if ! command_exists docker; then
    echo -e "${YELLOW}⚠️  Docker is not installed - database may not work${NC}"
fi

echo -e "${GREEN}✅ Prerequisites check passed${NC}"

# Check Node.js version
NODE_VERSION=$(node --version)
echo -e "${BLUE}Node.js version: ${NODE_VERSION}${NC}"

# Check if ports are available
echo -e "${BLUE}Checking port availability...${NC}"

if port_in_use 5001; then
    echo -e "${YELLOW}⚠️  Port 5001 (backend) is already in use${NC}"
    echo "Trying to stop existing process..."
    pkill -f "node.*5001" || true
    sleep 2
fi

if port_in_use 3000; then
    echo -e "${YELLOW}⚠️  Port 3000 (frontend) is already in use${NC}"
    echo "Trying to stop existing process..."
    pkill -f "vite.*3000" || true
    sleep 2
fi

# Start database if Docker is available
if command_exists docker && command_exists docker-compose; then
    echo -e "${BLUE}Starting database...${NC}"
    if [ -f "docker-compose.yml" ]; then
        docker-compose up -d postgres redis
        echo -e "${GREEN}✅ Database services started${NC}"
        sleep 3
    else
        echo -e "${YELLOW}⚠️  docker-compose.yml not found, skipping database startup${NC}"
    fi
fi

# Install dependencies if needed
echo -e "${BLUE}Checking dependencies...${NC}"

if [ ! -d "backend/node_modules" ]; then
    echo -e "${YELLOW}Installing backend dependencies...${NC}"
    cd backend && npm install && cd ..
fi

if [ ! -d "frontend/node_modules" ]; then
    echo -e "${YELLOW}Installing frontend dependencies...${NC}"
    cd frontend && npm install && cd ..
fi

# Create environment files if they don't exist
if [ ! -f "backend/.env" ]; then
    echo -e "${YELLOW}Creating backend .env file...${NC}"
    cp backend/.env.example backend/.env 2>/dev/null || cat > backend/.env << EOF
NODE_ENV=development
PORT=5001
DATABASE_URL=postgresql://smart_pen_user:smart_pen_password@localhost:5432/smart_pen_db
JWT_SECRET=your-super-secret-jwt-key-change-this-in-production
JWT_EXPIRES_IN=7d
CORS_ORIGIN=http://localhost:3000
REDIS_URL=redis://localhost:6379
EOF
fi

if [ ! -f "frontend/.env" ]; then
    echo -e "${YELLOW}Creating frontend .env file...${NC}"
    cat > frontend/.env << EOF
VITE_API_URL=http://localhost:5001/api
VITE_WS_URL=ws://localhost:5001
VITE_APP_NAME=Smart Pen Monitor
VITE_APP_VERSION=2.0.0
VITE_ENVIRONMENT=development
EOF
fi

# Function to start services with proper logging
start_backend() {
    echo -e "${BLUE}🔧 Starting backend server on port 5001...${NC}"
    cd backend
    npm run dev > ../logs/backend.log 2>&1 &
    BACKEND_PID=$!
    cd ..
    echo $BACKEND_PID > .backend.pid
    echo -e "${GREEN}✅ Backend started (PID: $BACKEND_PID)${NC}"
}

start_frontend() {
    echo -e "${BLUE}🎨 Starting frontend server on port 3000...${NC}"
    cd frontend
    npm run dev > ../logs/frontend.log 2>&1 &
    FRONTEND_PID=$!
    cd ..
    echo $FRONTEND_PID > .frontend.pid
    echo -e "${GREEN}✅ Frontend started (PID: $FRONTEND_PID)${NC}"
}

# Create logs directory
mkdir -p logs

# Start services
start_backend
sleep 3
start_frontend

# Wait a moment for services to start
echo -e "${BLUE}Waiting for services to initialize...${NC}"
sleep 5

# Check if services are running
echo -e "${BLUE}Checking service health...${NC}"

# Check backend
if curl -s http://localhost:5001/api/health >/dev/null 2>&1; then
    echo -e "${GREEN}✅ Backend is responding${NC}"
else
    echo -e "${YELLOW}⚠️  Backend may still be starting...${NC}"
fi

# Check frontend
if curl -s http://localhost:3000 >/dev/null 2>&1; then
    echo -e "${GREEN}✅ Frontend is responding${NC}"
else
    echo -e "${YELLOW}⚠️  Frontend may still be starting...${NC}"
fi

echo ""
echo -e "${GREEN}🎉 Smart Pen Development Environment Started!${NC}"
echo ""
echo -e "${BLUE}📍 Access Points:${NC}"
echo -e "   Frontend: ${GREEN}http://localhost:3000${NC}"
echo -e "   Backend:  ${GREEN}http://localhost:5001/api${NC}"
echo -e "   API Docs: ${GREEN}http://localhost:5001/api/docs${NC}"
echo ""
echo -e "${BLUE}📋 Available Commands:${NC}"
echo -e "   View backend logs:  ${YELLOW}tail -f logs/backend.log${NC}"
echo -e "   View frontend logs: ${YELLOW}tail -f logs/frontend.log${NC}"
echo -e "   Stop all services:  ${YELLOW}./stop-dev.sh${NC}"
echo ""
echo -e "${BLUE}🔍 Monitoring:${NC}"
echo "   Backend PID: $(cat .backend.pid 2>/dev/null || echo 'N/A')"
echo "   Frontend PID: $(cat .frontend.pid 2>/dev/null || echo 'N/A')"
echo ""

# Create stop script
cat > stop-dev.sh << 'EOF'
#!/bin/bash
echo "🛑 Stopping Smart Pen Development Environment..."

# Stop backend
if [ -f ".backend.pid" ]; then
    BACKEND_PID=$(cat .backend.pid)
    kill $BACKEND_PID 2>/dev/null && echo "✅ Backend stopped (PID: $BACKEND_PID)"
    rm .backend.pid
fi

# Stop frontend
if [ -f ".frontend.pid" ]; then
    FRONTEND_PID=$(cat .frontend.pid)
    kill $FRONTEND_PID 2>/dev/null && echo "✅ Frontend stopped (PID: $FRONTEND_PID)"
    rm .frontend.pid
fi

# Stop any remaining processes
pkill -f "node.*5001" 2>/dev/null
pkill -f "vite.*3000" 2>/dev/null

echo "🏁 All services stopped"
EOF

chmod +x stop-dev.sh

echo -e "${GREEN}Development environment is ready! 🚀${NC}"
echo -e "${BLUE}Press Ctrl+C to view logs, or run './stop-dev.sh' to stop all services${NC}"

# Keep script running and show logs
trap 'echo -e "\n${YELLOW}Stopping services...${NC}"; ./stop-dev.sh; exit 0' INT

echo ""
echo -e "${BLUE}📊 Live Logs (Ctrl+C to exit):${NC}"
echo "=================================="

# Show combined logs
tail -f logs/backend.log logs/frontend.log 2>/dev/null || {
    echo "Waiting for log files..."
    sleep 2
    tail -f logs/backend.log logs/frontend.log 2>/dev/null || echo "Log files not available yet"
} 