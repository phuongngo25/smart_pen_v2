# 🚀 Smart Pen v2.0 - Complete Installation Guide

## 📋 What You'll Install

This guide will help you install and set up:
1. **Node.js** - JavaScript runtime for frontend and backend
2. **npm** - Package manager (comes with Node.js)
3. **PostgreSQL** - Database (via Docker)
4. **All project dependencies** - React, Express, etc.
5. **Development tools** - Code editor, extensions

---

## 🛠️ Step-by-Step Installation

### Step 1: Install Node.js (Required)

**What is Node.js?** 
Node.js lets you run JavaScript on your computer (not just in browsers). Both our frontend (React) and backend (Express) need it.

#### On macOS (using Homebrew - recommended):
```bash
# Install Node.js and npm
brew install node

# Verify installation
node --version    # Should show v18.0.0 or higher
npm --version     # Should show v8.0.0 or higher
```

#### Alternative: Download from Official Website
1. Go to [nodejs.org](https://nodejs.org/)
2. Download the LTS version (Long Term Support)
3. Run the installer
4. Follow the setup wizard

### Step 2: Verify Your Installation

Let's check if everything is installed correctly:

```bash
# Check Node.js version
node --version

# Check npm version  
npm --version

# Check Docker version (should already be installed)
docker --version

# Check if you're in the right directory
pwd  # Should show: /Users/phuongngo/Code/Smart_pen_v2.0
```

### Step 3: Install Project Dependencies

Now we'll install all the libraries and packages our project needs:

#### Install Backend Dependencies
```bash
# Go to backend folder
cd backend

# Install all backend packages (Express, TypeScript, etc.)
npm install

# This installs ~50 packages including:
# - express (web server)
# - typescript (type safety)
# - prisma (database tools)
# - bcryptjs (password security)
# - And many more...

# Go back to main directory
cd ..
```

#### Install Frontend Dependencies
```bash
# Go to frontend folder
cd frontend

# Install all frontend packages (React, Material-UI, etc.)
npm install

# This installs ~80 packages including:
# - react (UI framework)
# - @mui/material (UI components)
# - typescript (type safety)
# - chart.js (data visualization)
# - And many more...

# Go back to main directory
cd ..
```

### Step 4: Set Up the Database

We'll use Docker to run PostgreSQL (no need to install it manually):

```bash
# Start PostgreSQL database using Docker
docker-compose up -d postgres

# Check if database is running
docker ps

# You should see a container named "smartpen_db"
```

### Step 5: Configure the Database

```bash
# Go to backend folder
cd backend

# Generate database client code
npm run db:generate

# Create database tables
npm run db:migrate

# Go back to main directory
cd ..
```

### Step 6: Start the Development Servers

Now let's start everything:

#### Terminal 1 - Start Backend API
```bash
cd backend
npm run dev

# You should see:
# "🚀 Server running on port 5000"
# "✅ Database connected"
```

#### Terminal 2 - Start Frontend Application
```bash
# Open a new terminal tab/window
cd frontend
npm run dev

# You should see:
# "Local: http://localhost:3000"
# "Ready in XXXms"
```

#### Terminal 3 - Check Everything Works
```bash
# Test backend API
curl http://localhost:5000/api/health

# Open frontend in browser
open http://localhost:3000
```

---

## 🎯 Quick Setup Script

Here's a complete script that does everything automatically:

```bash
#!/bin/bash
echo "🚀 Setting up Smart Pen v2.0..."

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo "❌ Node.js not found. Installing..."
    brew install node
fi

# Install backend dependencies
echo "📦 Installing backend dependencies..."
cd backend
npm install
echo "✅ Backend dependencies installed"

# Install frontend dependencies
echo "📦 Installing frontend dependencies..."
cd ../frontend
npm install
echo "✅ Frontend dependencies installed"

# Setup database
echo "🗄️ Setting up database..."
cd ..
docker-compose up -d postgres
sleep 10  # Wait for database to start

cd backend
npm run db:generate
npm run db:migrate
echo "✅ Database configured"

cd ..
echo "🎉 Setup complete!"
echo ""
echo "🚀 To start the application:"
echo "Terminal 1: cd backend && npm run dev"
echo "Terminal 2: cd frontend && npm run dev"
echo "Then open: http://localhost:3000"
```

---

## 🔧 Development Tools (Optional but Recommended)

### Visual Studio Code (Code Editor)
```bash
# Install VS Code using Homebrew
brew install --cask visual-studio-code

# Or download from: https://code.visualstudio.com/
```

### Useful VS Code Extensions
After installing VS Code, install these extensions:
1. **ES7+ React/Redux/React-Native snippets** - React shortcuts
2. **TypeScript Importer** - Auto import TypeScript
3. **Prettier** - Code formatting
4. **ESLint** - Code quality
5. **Thunder Client** - API testing (like Postman)

---

## 📱 What Each Component Does

### Frontend (React Application)
- **Location**: `frontend/` folder
- **What it is**: Web application users interact with
- **Technology**: React + TypeScript + Material-UI
- **Access**: http://localhost:3000
- **Features**: Dashboard, login, charts, device management

### Backend (Express API)
- **Location**: `backend/` folder  
- **What it is**: Server that handles data and business logic
- **Technology**: Node.js + Express + TypeScript
- **Access**: http://localhost:5000
- **Features**: User authentication, data processing, API endpoints

### Database (PostgreSQL)
- **Location**: Docker container
- **What it is**: Stores all application data
- **Technology**: PostgreSQL 15
- **Access**: localhost:5432
- **Features**: User data, sensor readings, device info

---

## 🐛 Troubleshooting Common Issues

### Issue 1: "command not found: node"
**Solution**: Install Node.js
```bash
brew install node
```

### Issue 2: "npm ERR! EACCES: permission denied"
**Solution**: Fix npm permissions
```bash
sudo chown -R $(whoami) ~/.npm
```

### Issue 3: "Port 3000 is already in use"
**Solution**: Kill the process using that port
```bash
# Find what's using port 3000
lsof -ti:3000

# Kill the process (replace XXXX with the process ID)
kill -9 XXXX
```

### Issue 4: Database connection error
**Solution**: Make sure Docker is running
```bash
# Check Docker status
docker ps

# Restart database
docker-compose restart postgres
```

### Issue 5: "Cannot find module" errors
**Solution**: Reinstall dependencies
```bash
# Delete node_modules and reinstall
rm -rf node_modules
npm install
```

---

## 🎯 Testing Your Installation

After everything is installed, test these:

### Test 1: Backend API
```bash
# Should return {"status": "healthy"}
curl http://localhost:5000/api/health
```

### Test 2: Frontend Application
1. Open browser to http://localhost:3000
2. You should see the Smart Pen login page
3. Try logging in with demo credentials

### Test 3: Database Connection
```bash
# Should show database tables
docker exec smartpen_db psql -U smartpen_user -d smartpen_db -c "\dt"
```

---

## 📚 Next Steps

After installation:

1. **Explore the Application**
   - Login with demo credentials
   - Check the dashboard
   - View analytics and settings

2. **Set Up ESP32 (Optional)**
   - Install PlatformIO
   - Configure WiFi credentials
   - Upload firmware to ESP32

3. **Development**
   - Make changes to the code
   - Both servers auto-reload on changes
   - Check browser console for errors

4. **Production Deployment**
   - Use Docker Compose for production
   - Configure environment variables
   - Set up SSL certificates

---

## 🆘 Need Help?

If you encounter issues:

1. **Check the logs** in terminal where servers are running
2. **Check browser console** (F12 → Console tab)
3. **Verify all services are running**:
   ```bash
   # Check running processes
   docker ps  # Database
   lsof -i :3000  # Frontend
   lsof -i :5000  # Backend
   ```

---

## ✅ Success Checklist

- [ ] Node.js installed (v18+)
- [ ] npm working
- [ ] Docker running
- [ ] Backend dependencies installed
- [ ] Frontend dependencies installed
- [ ] Database created and migrated
- [ ] Backend server running on port 5000
- [ ] Frontend server running on port 3000
- [ ] Can access application at http://localhost:3000
- [ ] API health check returns success

**🎉 Congratulations! Your Smart Pen v2.0 system is ready!** 