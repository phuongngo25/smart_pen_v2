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
