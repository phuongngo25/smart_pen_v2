# Smart Pen v2.0 - Parkinson's Disease Monitoring System

## Overview
A comprehensive IoT-based smart pen system designed for real-time monitoring and analysis of hand tremors in Parkinson's disease patients. The system combines ESP32 microcontroller technology with advanced web applications to provide healthcare professionals and patients with valuable insights into tremor patterns.

## Features

### Core Functionality
- **Real-time Tremor Detection**: ESP32 + MPU6050 sensor for precise hand movement tracking
- **FFT Analysis**: Advanced frequency analysis for tremor characterization
- **Web Dashboard**: Interactive React-based interface for data visualization
- **Multi-role Access**: Admin, Doctor, and Patient role management
- **Real-time Communication**: Server-Sent Events for live data streaming
- **Historical Analysis**: Comprehensive data storage and trend analysis

### Technical Features
- **High Performance**: <100ms API response time, <50ms WebSocket latency
- **Scalability**: Support for 100+ concurrent users
- **Security**: JWT authentication, role-based access control
- **Containerization**: Full Docker deployment with Nginx
- **Database**: PostgreSQL with optimized time-series data handling
- **API Documentation**: Comprehensive OpenAPI/Swagger documentation

## Architecture

### System Components
- **ESP32 Firmware**: Sensor data collection and wireless transmission
- **Backend API**: Node.js + TypeScript + Prisma ORM
- **Frontend**: React + TypeScript + Material-UI
- **Database**: PostgreSQL with Redis caching
- **Infrastructure**: Docker + Nginx + SSL

### Tech Stack
- **Hardware**: ESP32, MPU6050 gyroscope/accelerometer
- **Backend**: Node.js, TypeScript, Express, Prisma, PostgreSQL
- **Frontend**: React 18, TypeScript, Vite, Material-UI, Chart.js
- **DevOps**: Docker, Docker Compose, Nginx, SSL/TLS
- **Testing**: Jest, React Testing Library, Supertest
- **Monitoring**: Winston logging, health checks

## Quick Start

### Prerequisites
- Docker & Docker Compose
- Node.js 18+ (for development)
- PlatformIO (for ESP32 development)

### Installation
1. Clone the repository
2. Run the setup script: `./scripts/setup.sh`
3. Start services: `docker-compose up -d`
4. Access the application at `https://localhost`

### Development Setup
```bash
# Install dependencies
cd backend && npm install
cd ../frontend && npm install

# Start development servers
npm run dev:backend
npm run dev:frontend
```

## Project Structure
```
Smart_pen_v2.0/
├── backend/                 # Node.js API server
├── frontend/               # React web application
├── esp32/                  # ESP32 firmware (PlatformIO)
├── database/               # Database configurations
├── docker/                 # Docker configurations
├── docs/                   # Documentation
├── scripts/                # Setup and utility scripts
└── docker-compose.yml      # Service orchestration
```

## Documentation
- [Setup Guide](docs/SETUP_GUIDE.md)
- [API Documentation](docs/API_DOCUMENTATION.md)
- [Project Structure](docs/PROJECT_STRUCTURE.md)

## Performance Targets
- API Response Time: <100ms
- WebSocket Latency: <50ms
- Concurrent Users: 100+
- Data Accuracy: >99%
- Uptime: >99.9%

## License
MIT License - see LICENSE file for details

## Contributing
Please read CONTRIBUTING.md for contribution guidelines.

## Support
For support, please contact the development team or create an issue in the repository. 