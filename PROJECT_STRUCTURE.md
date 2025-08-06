# 🏗️ Smart Pen v2.0 - Complete Project Structure

## 📁 Project Overview

This document provides a comprehensive overview of the Smart Pen v2.0 project structure, including all components, their purposes, and relationships.

```
Smart_pen_v2.0/
├── 🖥️ backend/                    # Node.js + TypeScript Backend
├── 🌐 frontend/                   # React + TypeScript Frontend  
├── 🗄️ database/                   # Database Configuration
├── 🔧 esp32/                      # ESP32 Firmware (Arduino/PlatformIO)
├── 🐳 docker/                     # Docker & Nginx Configuration
├── 📚 docs/                       # Documentation
├── 🔨 scripts/                    # Automation Scripts
├── 📄 Configuration Files         # Docker, Environment, etc.
└── 📖 Documentation Files         # README, Guides, etc.
```

---

## 🖥️ Backend Structure (`backend/`)

### Core Architecture
```
backend/
├── 📝 prisma/                     # Database Schema & Migrations
│   ├── schema.prisma              # Prisma database schema
│   └── migrations/                # Database migration files
├── 🚀 src/
│   ├── 🎮 controllers/            # Request handlers
│   │   ├── authController.ts      # Authentication endpoints
│   │   ├── deviceController.ts    # Device management
│   │   ├── sensorController.ts    # Sensor data handling
│   │   ├── analyticsController.ts # Data analysis endpoints
│   │   └── userController.ts      # User management
│   ├── 🛡️ middleware/             # Express middleware
│   │   ├── auth.ts               # JWT authentication
│   │   ├── rateLimit.ts          # API rate limiting
│   │   ├── errorHandler.ts       # Error handling
│   │   ├── validation.ts         # Request validation
│   │   └── logging.ts            # Request logging
│   ├── 🛣️ routes/                 # API route definitions
│   │   ├── auth.ts               # Authentication routes
│   │   ├── devices.ts            # Device management routes
│   │   ├── sensor.ts             # Sensor data routes
│   │   ├── analytics.ts          # Analytics routes
│   │   └── users.ts              # User management routes
│   ├── 🔧 services/               # Business logic layer
│   │   ├── authService.ts        # Authentication logic
│   │   ├── deviceService.ts      # Device operations
│   │   ├── sensorService.ts      # Sensor data processing
│   │   ├── analysisService.ts    # Tremor analysis algorithms
│   │   ├── sseService.ts         # Server-Sent Events
│   │   └── emailService.ts       # Email notifications
│   ├── 🔧 utils/                  # Utility functions
│   │   ├── fftAnalyzer.ts        # Fast Fourier Transform
│   │   ├── tremorAnalyzer.ts     # Medical tremor analysis
│   │   ├── logger.ts             # Logging utility
│   │   ├── encryption.ts         # Data encryption
│   │   └── validators.ts         # Data validation
│   ├── ⚙️ config/                 # Configuration files
│   │   ├── database.ts           # Database connection
│   │   ├── redis.ts              # Redis configuration
│   │   └── email.ts              # Email configuration
│   ├── 📊 types/                  # TypeScript type definitions
│   │   ├── user.ts               # User-related types
│   │   ├── device.ts             # Device types
│   │   ├── sensor.ts             # Sensor data types
│   │   └── api.ts                # API response types
│   └── 🚀 index.ts                # Application entry point
├── 📦 package.json                # Dependencies & scripts
├── 🔧 tsconfig.json               # TypeScript configuration
└── 🌍 .env                        # Environment variables
```

### Key Backend Features
- **🔐 JWT Authentication** with refresh tokens
- **📊 Real-time Data Processing** using Server-Sent Events
- **🧠 Advanced Tremor Analysis** with FFT algorithms
- **🗄️ PostgreSQL Database** with Prisma ORM
- **🚦 Rate Limiting** and security middleware
- **📧 Email Notifications** for alerts
- **🔍 Comprehensive Logging** and error handling

---

## 🌐 Frontend Structure (`frontend/`)

### React Application Architecture
```
frontend/
├── 📁 public/                     # Static files
│   ├── index.html                # Main HTML template
│   ├── favicon.ico               # Application icon
│   └── manifest.json             # PWA manifest
├── 📁 src/
│   ├── 🧩 components/             # Reusable React components
│   │   ├── 📋 Dashboard/          # Dashboard-specific components
│   │   │   ├── RealTimeMonitor.tsx   # Live data monitoring
│   │   │   └── TremorChart.tsx       # Tremor visualization
│   │   ├── 🎨 Layout/             # Layout components
│   │   │   ├── Navbar.tsx            # Navigation bar
│   │   │   └── ProtectedRoute.tsx    # Route protection
│   │   └── 🎪 UI/                 # Reusable UI components
│   │       ├── LoadingSpinner.tsx    # Loading indicators
│   │       ├── ErrorBoundary.tsx     # Error handling
│   │       └── SkeletonLoader.tsx    # Loading skeletons
│   ├── 📄 pages/                   # Application pages
│   │   ├── Login.tsx              # Authentication page
│   │   ├── Register.tsx           # User registration
│   │   ├── Dashboard.tsx          # Main dashboard
│   │   ├── Analytics.tsx          # Data analytics
│   │   ├── Settings.tsx           # User settings
│   │   ├── Profile.tsx            # User profile
│   │   ├── Devices.tsx            # Device management
│   │   └── Monitoring.tsx         # Real-time monitoring
│   ├── 🔗 hooks/                   # Custom React hooks
│   │   └── useApi.ts              # API data fetching hooks
│   ├── 🏪 store/                   # State management (Zustand)
│   │   ├── authStore.ts           # Authentication state
│   │   └── notificationStore.ts   # Notifications state
│   ├── 🌐 api/                     # API integration
│   │   └── client.ts              # Axios configuration
│   ├── 🔧 utils/                   # Utility functions
│   │   ├── dateHelpers.ts         # Date formatting
│   │   └── validation.ts          # Form validation
│   ├── 📝 types/                   # TypeScript definitions
│   │   └── index.ts               # All type definitions
│   ├── 🎨 styles/                  # Global styles
│   │   └── index.css              # Main stylesheet
│   ├── 📱 App.tsx                  # Main application component
│   └── 🚀 main.tsx                 # Application entry point
├── 📦 package.json                # Dependencies & scripts
├── ⚙️ vite.config.ts              # Vite build configuration
└── 🔧 tsconfig.json               # TypeScript configuration
```

### Frontend Technologies
- **⚛️ React 18** with TypeScript
- **🎨 Material-UI (MUI)** for components
- **📊 Chart.js** for data visualization
- **🔄 Zustand** for state management
- **🏗️ Vite** for build tooling
- **📱 Responsive Design** for all devices

---

## 🗄️ Database Structure (`database/`)

### PostgreSQL Schema
```
database/
├── 📁 init/                       # Database initialization
│   ├── 01-init.sql               # Database setup script
│   └── 02-seed.sql               # Sample data
└── 📊 Schema Overview:
    ├── 👥 Users                   # User accounts (Admin, Doctor, Patient)
    ├── 📱 Devices                 # IoT device registry
    ├── 📊 SensorReadings          # Time-series sensor data
    ├── 🔍 MonitoringSessions      # Monitoring periods
    ├── 🧠 TremorAnalysis          # Processed tremor data
    ├── 👨‍⚕️ DoctorPatient           # Doctor-patient relationships
    └── 📋 AuditLogs               # System audit trail
```

### Database Features
- **📈 Time-series Data** optimized for sensor readings
- **🔍 Full-text Search** capabilities
- **📊 Analytics Views** for reporting
- **🔐 Row-level Security** for data isolation
- **📝 Audit Logging** for compliance
- **⚡ Performance Indexes** for fast queries

---

## 🔧 ESP32 Firmware (`esp32/`)

### Embedded System Architecture
```
esp32/
├── 📁 include/                    # Header files
│   ├── config.h                 # Configuration constants
│   ├── sensors.h                # Sensor interfaces
│   └── communication.h          # WiFi/HTTP headers
├── 📁 src/                       # Source code
│   ├── main.cpp                 # Main program loop
│   ├── sensors.cpp              # MPU6050 sensor handling
│   ├── wifi_manager.cpp         # WiFi connectivity
│   ├── data_processor.cpp       # Signal processing
│   └── http_client.cpp          # API communication
├── 📁 lib/                       # External libraries
│   ├── MPU6050/                 # Sensor library
│   └── ArduinoJson/             # JSON handling
├── ⚙️ platformio.ini             # PlatformIO configuration
└── 📊 Features:
    ├── 📡 Real-time Data Collection (100Hz sampling)
    ├── 🔋 Power Management (optimized for battery)
    ├── 📶 WiFi Auto-reconnection
    ├── 🎛️ Sensor Calibration
    └── 🔄 OTA Firmware Updates
```

---

## 🐳 Docker Infrastructure (`docker/`)

### Containerized Deployment
```
docker/
├── 📁 nginx/                     # Reverse proxy configuration
│   ├── nginx.conf               # Main Nginx config
│   ├── ssl/                     # SSL certificates
│   └── sites-enabled/           # Virtual hosts
├── 🐳 Containers:
│   ├── 🗄️ PostgreSQL 15         # Database server
│   ├── 🔴 Redis 7               # Caching & sessions
│   ├── 🖥️ Backend API           # Node.js application
│   ├── 🌐 Frontend             # React application
│   └── 🌍 Nginx                # Reverse proxy & SSL
└── 📋 docker-compose.yml        # Multi-container orchestration
```

---

## 📚 Documentation (`docs/`)

### Comprehensive Documentation
```
docs/
├── 📖 api/                       # API documentation
│   ├── authentication.md        # Auth endpoints
│   ├── devices.md               # Device management
│   └── analytics.md             # Analytics API
├── 🔧 hardware/                  # Hardware documentation
│   ├── assembly.md              # Device assembly guide
│   ├── calibration.md           # Sensor calibration
│   └── troubleshooting.md       # Hardware issues
├── 🖥️ deployment/                # Deployment guides
│   ├── production.md            # Production setup
│   ├── ssl-setup.md             # HTTPS configuration
│   └── monitoring.md            # System monitoring
└── 👥 user/                      # User documentation
    ├── getting-started.md       # Quick start guide
    ├── dashboard.md             # Dashboard usage
    └── analytics.md             # Analytics features
```

---

## 🔨 Automation Scripts (`scripts/`)

### Development & Deployment Tools
```
scripts/
├── 🚀 setup.sh                  # Initial project setup
├── 🔧 build.sh                  # Build all components
├── 🧪 test.sh                   # Run all tests
├── 📦 deploy.sh                 # Production deployment
├── 🗄️ backup.sh                 # Database backup
└── 🔄 update.sh                 # System updates
```

---

## ⚙️ Configuration Files

### Project Configuration
```
Smart_pen_v2.0/
├── 🐳 docker-compose.yml         # Development environment
├── 🐳 docker-compose.prod.yml    # Production environment
├── 🌍 env.example                # Environment template
├── 🔨 Makefile                   # Build automation
├── 📋 SYSTEM_REQUIREMENTS.md     # System requirements
├── 📖 INSTALLATION_GUIDE.md      # Setup instructions
├── 📊 DEVELOPMENT_STATUS.md       # Development progress
└── 📝 README.md                  # Main project readme
```

---

## 🔄 Data Flow Architecture

### System Data Flow
```
📱 ESP32 Device
    ↓ (HTTP POST - 100Hz)
🌐 Nginx Reverse Proxy
    ↓
🖥️ Backend API (Node.js)
    ↓
🧠 Tremor Analysis Engine (FFT)
    ↓
🗄️ PostgreSQL Database
    ↑↓
📊 Real-time Analytics
    ↓ (Server-Sent Events)
🌐 React Frontend
    ↓
👤 User Interface
```

---

## 🚀 Key Features Implemented

### Core Functionality
- ✅ **Real-time Tremor Detection** with FFT analysis
- ✅ **Multi-user Support** (Admin, Doctor, Patient roles)
- ✅ **Device Management** with auto-discovery
- ✅ **Live Data Streaming** via Server-Sent Events
- ✅ **Advanced Analytics** with interactive charts
- ✅ **Secure Authentication** with JWT tokens
- ✅ **Responsive Web Interface** for all devices
- ✅ **Production-ready Deployment** with Docker
- ✅ **Comprehensive API** with 40+ endpoints
- ✅ **Medical-grade Security** with encryption

### Technology Stack
- **Backend**: Node.js, TypeScript, Express, Prisma, PostgreSQL
- **Frontend**: React, TypeScript, Material-UI, Chart.js, Vite
- **Hardware**: ESP32, MPU6050, Arduino Framework
- **Infrastructure**: Docker, Nginx, Redis, SSL/TLS
- **Analytics**: FFT, Signal Processing, Machine Learning Ready

---

## 📈 Development Status

### ✅ Completed Components
- [x] Complete backend API with authentication
- [x] React frontend with real-time capabilities
- [x] ESP32 firmware with sensor integration
- [x] Docker containerization
- [x] Database schema and migrations
- [x] Security implementation
- [x] Documentation and guides

### 🔄 Production Ready Features
- [x] SSL/HTTPS support
- [x] Production Docker compose
- [x] Database optimization
- [x] Error handling and logging
- [x] Performance monitoring
- [x] Backup and recovery procedures

---

## 🛠️ Development Workflow

### Getting Started
1. **Clone Repository**: `git clone [repository]`
2. **Install Dependencies**: `make install`
3. **Start Development**: `make dev`
4. **Run Tests**: `make test`
5. **Deploy Production**: `make deploy-prod`

### Development Commands
```bash
# Development
make dev              # Start all services
make frontend         # Frontend only
make backend          # Backend only
make database         # Database only

# Testing
make test            # Run all tests
make test-frontend   # Frontend tests
make test-backend    # Backend tests

# Production
make build           # Build for production
make deploy          # Deploy to production
make backup          # Backup database
```

---

## 📊 Performance Targets

### System Performance
- **API Response Time**: < 100ms average
- **Real-time Latency**: < 50ms for live data
- **Concurrent Users**: 100+ simultaneous
- **Data Throughput**: 1000+ sensor readings/second
- **Database Queries**: < 10ms for most operations
- **Frontend Load Time**: < 3 seconds initial load

---

## 🔐 Security Features

### Implemented Security
- **🔑 JWT Authentication** with refresh tokens
- **🛡️ Rate Limiting** to prevent abuse
- **🔒 HTTPS/TLS** encryption in transit
- **🗄️ Database Encryption** for sensitive data
- **👤 Role-based Access Control** (RBAC)
- **📝 Audit Logging** for compliance
- **🚫 Input Validation** and sanitization
- **🔐 Password Hashing** with bcrypt

---

This comprehensive project structure provides a production-ready foundation for the Smart Pen v2.0 Parkinson's monitoring system with scalability, security, and maintainability at its core. 