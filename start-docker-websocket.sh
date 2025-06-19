#!/bin/bash

# Smart Pen v2.0 WebSocket System - Docker Startup Script
# This script starts the complete WebSocket-enabled Smart Pen system using Docker

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${GREEN}
╔═══════════════════════════════════════════════════════════════════════════════╗
║                    Smart Pen v2.0 WebSocket System - Docker                  ║
║                           Complete System Startup                            ║
╚═══════════════════════════════════════════════════════════════════════════════╝${NC}"
}

# Main execution
main() {
    print_header
    
    print_status "Starting Smart Pen v2.0 WebSocket System with Docker..."
    
    # Check if Docker is running
    if ! docker info > /dev/null 2>&1; then
        print_error "Docker is not running. Please start Docker Desktop and try again."
        exit 1
    fi
    
    print_success "Docker is running"
    
    # Check if docker-compose is available
    if ! command -v docker-compose &> /dev/null; then
        print_warning "docker-compose not found, trying 'docker compose'..."
        DOCKER_COMPOSE_CMD="docker compose"
    else
        DOCKER_COMPOSE_CMD="docker-compose"
    fi
    
    # Stop any existing containers
    print_status "Stopping any existing containers..."
    $DOCKER_COMPOSE_CMD down --remove-orphans || true
    
    # Remove old volumes if needed (optional)
    if [ "$1" = "--clean" ]; then
        print_warning "Cleaning up old volumes..."
        docker volume rm smartpen_postgres_data smartpen_redis_data 2>/dev/null || true
    fi
    
    # Build and start the services
    print_status "Building and starting WebSocket-enabled services..."
    print_status "This may take a few minutes on first run..."
    
    # Start database services first
    print_status "Starting database services..."
    $DOCKER_COMPOSE_CMD up -d postgres redis
    
    # Wait for databases to be ready
    print_status "Waiting for databases to be ready..."
    sleep 10
    
    # Start backend service
    print_status "Starting backend with WebSocket support..."
    $DOCKER_COMPOSE_CMD up -d backend
    
    # Wait for backend to be ready
    print_status "Waiting for backend to start..."
    sleep 15
    
    # Start frontend service
    print_status "Starting frontend with WebSocket client..."
    $DOCKER_COMPOSE_CMD up -d frontend
    
    # Start nginx proxy
    print_status "Starting nginx reverse proxy..."
    $DOCKER_COMPOSE_CMD up -d nginx
    
    # Show status
    print_status "Checking service status..."
    $DOCKER_COMPOSE_CMD ps
    
    # Wait a moment for all services to stabilize
    sleep 10
    
    # Check health endpoints
    print_status "Checking service health..."
    
    # Check backend health
    if curl -f http://localhost:3005/health > /dev/null 2>&1; then
        print_success "Backend is healthy at http://localhost:3005"
    else
        print_warning "Backend health check failed"
    fi
    
    # Check frontend health
    if curl -f http://localhost:3004 > /dev/null 2>&1; then
        print_success "Frontend is accessible at http://localhost:3004"
    else
        print_warning "Frontend health check failed"
    fi
    
    # Check nginx proxy
    if curl -f http://localhost/health > /dev/null 2>&1; then
        print_success "Nginx proxy is working at http://localhost"
    else
        print_warning "Nginx proxy health check failed"
    fi
    
    print_success "Smart Pen WebSocket System is starting up!"
    
    echo -e "\n${GREEN}🌐 Access Points:${NC}"
    echo -e "  • Frontend (Direct):     ${BLUE}http://localhost:3004${NC}"
    echo -e "  • Backend API (Direct):  ${BLUE}http://localhost:3005${NC}"
    echo -e "  • Full System (Nginx):   ${BLUE}http://localhost${NC}"
    echo -e "  • WebSocket Endpoint:    ${BLUE}ws://localhost:3005/ws${NC}"
    echo -e "  • WebSocket via Nginx:   ${BLUE}ws://localhost/ws${NC}"
    
    echo -e "\n${GREEN}📱 ESP32 Configuration:${NC}"
    echo -e "  • WiFi Mode Backend URL: ${BLUE}http://$(hostname -I | awk '{print $1}' || echo 'YOUR_HOST_IP'):3005${NC}"
    echo -e "  • WebSocket Server:      ${BLUE}ws://$(hostname -I | awk '{print $1}' || echo 'YOUR_HOST_IP'):3005/ws${NC}"
    
    echo -e "\n${GREEN}🔧 Management Commands:${NC}"
    echo -e "  • View logs:             ${YELLOW}$DOCKER_COMPOSE_CMD logs -f [service]${NC}"
    echo -e "  • Stop system:           ${YELLOW}$DOCKER_COMPOSE_CMD down${NC}"
    echo -e "  • Restart service:       ${YELLOW}$DOCKER_COMPOSE_CMD restart [service]${NC}"
    echo -e "  • View status:           ${YELLOW}$DOCKER_COMPOSE_CMD ps${NC}"
    
    echo -e "\n${GREEN}📊 Database:${NC}"
    echo -e "  • PostgreSQL:            ${BLUE}localhost:5432${NC}"
    echo -e "  • Redis:                 ${BLUE}localhost:6379${NC}"
    
    print_status "Showing recent logs from all services..."
    $DOCKER_COMPOSE_CMD logs --tail=20
    
    echo -e "\n${GREEN}✅ System startup complete!${NC}"
    echo -e "\nTo monitor the system:"
    echo -e "  ${YELLOW}$DOCKER_COMPOSE_CMD logs -f${NC}    # All services"
    echo -e "  ${YELLOW}$DOCKER_COMPOSE_CMD logs -f backend${NC}   # Backend only"
    echo -e "  ${YELLOW}$DOCKER_COMPOSE_CMD logs -f frontend${NC}  # Frontend only"
    
    echo -e "\nTo stop the system:"
    echo -e "  ${YELLOW}$DOCKER_COMPOSE_CMD down${NC}"
    
    echo -e "\n${BLUE}Press Ctrl+C to stop monitoring logs, or run the above commands in a new terminal.${NC}"
    
    # Follow logs
    $DOCKER_COMPOSE_CMD logs -f
}

# Check for help flag
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "Smart Pen v2.0 WebSocket System - Docker Startup Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --clean    Clean up old volumes before starting"
    echo "  --help     Show this help message"
    echo ""
    echo "This script will start the complete WebSocket-enabled Smart Pen system:"
    echo "  • PostgreSQL database"
    echo "  • Redis cache"
    echo "  • Backend API server with WebSocket support"
    echo "  • Frontend React application"
    echo "  • Nginx reverse proxy"
    echo ""
    exit 0
fi

# Run main function
main "$@" 