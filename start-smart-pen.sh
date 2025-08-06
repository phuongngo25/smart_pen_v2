#!/bin/bash

echo "🚀 Starting Smart Pen v2.0 Development Environment..."

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo -e "${RED}❌ Node.js not found. Please install Node.js first.${NC}"
    echo "Run: brew install node"
    exit 1
fi

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo -e "${RED}❌ Docker is not running. Please start Docker first.${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Prerequisites checked${NC}"

# Start PostgreSQL database
echo -e "${BLUE}🗄️ Starting PostgreSQL database...${NC}"
docker-compose up -d postgres

# Wait for database to be ready
echo -e "${BLUE}⏳ Waiting for database to be ready...${NC}"
sleep 10

# Check if database is responding
while ! docker exec smartpen_db pg_isready -U smartpen_user -d smartpen_db &> /dev/null; do
    echo -e "${BLUE}⏳ Waiting for database...${NC}"
    sleep 2
done

echo -e "${GREEN}✅ Database is ready${NC}"

# Instructions for user
echo ""
echo "🎯 Next Steps:"
echo "1. Open 2 new terminal windows/tabs"
echo "2. In Terminal 1, run: cd backend && npm run dev"
echo "3. In Terminal 2, run: cd frontend && npm run dev"
echo "4. Open browser to: http://localhost:3000"
echo ""
echo "🔧 If you get database errors in the backend:"
echo "cd backend && npx prisma db push --force-reset"
echo ""
echo "📱 Demo Login Credentials:"
echo "Email: admin@smartpen.com"
echo "Password: admin123"
echo ""
echo "🆘 If port 5000 is busy, the backend will use port 5001"
echo ""

# Create environment file if not exists
if [ ! -f backend/.env ]; then
    echo -e "${BLUE}📝 Creating backend environment file...${NC}"
    cd backend
    cat > .env << 'EOF'
DATABASE_URL="postgresql://smartpen_user:smartpen_password@localhost:5432/smartpen_db"
JWT_SECRET="your-super-secret-jwt-key-change-in-production"
PORT=5001
NODE_ENV=development
CORS_ORIGIN=http://localhost:3000
EOF
    cd ..
    echo -e "${GREEN}✅ Environment file created${NC}"
fi

echo -e "${GREEN}🎉 Smart Pen v2.0 setup complete!${NC}"
echo -e "${BLUE}💡 Tip: Keep this terminal open and follow the instructions above${NC}" 