# Smart Pen Project Makefile
# ===========================

.PHONY: help setup install start dev stop clean logs test build deploy

# Default target
.DEFAULT_GOAL := help

# Colors for terminal output
GREEN := \033[0;32m
YELLOW := \033[0;33m
RED := \033[0;31m
NC := \033[0m # No Color

# Project variables
PROJECT_NAME := smart-pen
DOCKER_COMPOSE := docker-compose
NODE_VERSION := 18

## Display help information
help:
	@echo "$(GREEN)Smart Pen Project Management$(NC)"
	@echo "============================="
	@echo ""
	@echo "Available commands:"
	@echo ""
	@grep -E '^## .*$$' $(MAKEFILE_LIST) | sed 's/##//' | awk 'BEGIN {FS = ":"}; {printf "$(YELLOW)%-20s$(NC) %s\n", $$1, $$2}'
	@echo ""

## Setup the entire project (first time)
setup:
	@echo "$(GREEN)Setting up Smart Pen project...$(NC)"
	@chmod +x scripts/setup.sh
	@./scripts/setup.sh

## Install dependencies for all services
install:
	@echo "$(GREEN)Installing dependencies...$(NC)"
	@echo "Installing backend dependencies..."
	@cd backend && npm install
	@echo "Installing frontend dependencies..."
	@cd frontend && npm install
	@echo "$(GREEN)Dependencies installed successfully!$(NC)"

## Start all services in development mode
dev:
	@echo "$(GREEN)Starting development environment...$(NC)"
	@$(DOCKER_COMPOSE) up -d postgres redis
	@sleep 5
	@cd backend && npm run prisma:generate && npm run prisma:migrate
	@$(DOCKER_COMPOSE) up -d --build backend frontend
	@echo "$(GREEN)Development environment is running!$(NC)"
	@echo "Frontend: http://localhost:3000"
	@echo "Backend:  http://localhost:5000"
	@echo "API Docs: http://localhost:5000/api-docs"

## Start all services in production mode
start:
	@echo "$(GREEN)Starting production environment...$(NC)"
	@$(DOCKER_COMPOSE) -f docker-compose.yml -f docker-compose.prod.yml up -d --build
	@echo "$(GREEN)Production environment is running!$(NC)"

## Stop all services
stop:
	@echo "$(YELLOW)Stopping all services...$(NC)"
	@$(DOCKER_COMPOSE) down
	@echo "$(GREEN)All services stopped!$(NC)"

## Stop all services and remove volumes
clean:
	@echo "$(RED)Cleaning up all services and data...$(NC)"
	@$(DOCKER_COMPOSE) down -v --remove-orphans
	@docker system prune -f
	@echo "$(GREEN)Cleanup completed!$(NC)"

## View logs for all services
logs:
	@$(DOCKER_COMPOSE) logs -f

## View logs for specific service
logs-backend:
	@$(DOCKER_COMPOSE) logs -f backend

logs-frontend:
	@$(DOCKER_COMPOSE) logs -f frontend

logs-db:
	@$(DOCKER_COMPOSE) logs -f postgres

## Run backend tests
test-backend:
	@echo "$(GREEN)Running backend tests...$(NC)"
	@cd backend && npm test

## Run frontend tests
test-frontend:
	@echo "$(GREEN)Running frontend tests...$(NC)"
	@cd frontend && npm test

## Run all tests
test: test-backend test-frontend
	@echo "$(GREEN)All tests completed!$(NC)"

## Build all services
build:
	@echo "$(GREEN)Building all services...$(NC)"
	@$(DOCKER_COMPOSE) build --no-cache
	@echo "$(GREEN)Build completed!$(NC)"

## Database operations
db-migrate:
	@echo "$(GREEN)Running database migrations...$(NC)"
	@cd backend && npm run prisma:migrate

db-seed:
	@echo "$(GREEN)Seeding database...$(NC)"
	@cd backend && npm run prisma:seed

db-reset:
	@echo "$(RED)Resetting database...$(NC)"
	@cd backend && npm run prisma:reset

db-studio:
	@echo "$(GREEN)Opening Prisma Studio...$(NC)"
	@cd backend && npm run prisma:studio

## ESP32 operations
esp32-build:
	@echo "$(GREEN)Building ESP32 firmware...$(NC)"
	@cd esp32 && pio run

esp32-upload:
	@echo "$(GREEN)Uploading firmware to ESP32...$(NC)"
	@cd esp32 && pio run --target upload

esp32-monitor:
	@echo "$(GREEN)Monitoring ESP32 serial output...$(NC)"
	@cd esp32 && pio device monitor

esp32-clean:
	@echo "$(GREEN)Cleaning ESP32 build files...$(NC)"
	@cd esp32 && pio run --target clean

## Linting and formatting
lint:
	@echo "$(GREEN)Running linters...$(NC)"
	@cd backend && npm run lint
	@cd frontend && npm run lint

format:
	@echo "$(GREEN)Formatting code...$(NC)"
	@cd backend && npm run format
	@cd frontend && npm run format

## Check project health
health:
	@echo "$(GREEN)Checking project health...$(NC)"
	@curl -f http://localhost:5000/api/health || echo "$(RED)Backend is not responding$(NC)"
	@curl -f http://localhost:3000 || echo "$(RED)Frontend is not responding$(NC)"

## Generate SSL certificates for production
ssl-cert:
	@echo "$(GREEN)Generating SSL certificates...$(NC)"
	@mkdir -p docker/nginx/ssl
	@openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
		-keyout docker/nginx/ssl/smartpen.key \
		-out docker/nginx/ssl/smartpen.crt \
		-subj "/C=US/ST=State/L=City/O=SmartPen/CN=smartpen.local"
	@echo "$(GREEN)SSL certificates generated!$(NC)"

## Backup database
backup:
	@echo "$(GREEN)Creating database backup...$(NC)"
	@mkdir -p backups
	@docker exec -t $(PROJECT_NAME)_postgres_1 pg_dump -U smartpen_user smartpen_db > backups/backup_$(shell date +%Y%m%d_%H%M%S).sql
	@echo "$(GREEN)Backup created in backups/ directory$(NC)"

## Restore database from backup
restore:
	@echo "$(RED)This will restore database from the latest backup$(NC)"
	@echo "$(RED)All current data will be lost!$(NC)"
	@read -p "Are you sure? (y/N) " confirm && [ "$$confirm" = "y" ]
	@docker exec -i $(PROJECT_NAME)_postgres_1 psql -U smartpen_user -d smartpen_db < $(shell ls -t backups/*.sql | head -1)
	@echo "$(GREEN)Database restored!$(NC)"

## Deploy to production (customize as needed)
deploy:
	@echo "$(GREEN)Deploying to production...$(NC)"
	@echo "$(YELLOW)Configure your production deployment process here$(NC)"
	@git push origin main
	@echo "$(GREEN)Deployment initiated!$(NC)"

## Quick development reset
reset-dev: stop clean install dev
	@echo "$(GREEN)Development environment reset completed!$(NC)"

## Show project status
status:
	@echo "$(GREEN)Project Status$(NC)"
	@echo "=============="
	@echo ""
	@echo "$(YELLOW)Docker Containers:$(NC)"
	@$(DOCKER_COMPOSE) ps
	@echo ""
	@echo "$(YELLOW)Docker Images:$(NC)"
	@docker images | grep smartpen
	@echo ""
	@echo "$(YELLOW)Network Ports:$(NC)"
	@netstat -tlpn 2>/dev/null | grep -E "(3000|5000|5432|6379)" || echo "No ports found"

## Show project info
info:
	@echo "$(GREEN)Smart Pen Project Information$(NC)"
	@echo "================================"
	@echo ""
	@echo "$(YELLOW)Project Structure:$(NC)"
	@echo "├── backend/          Node.js + TypeScript + Prisma"
	@echo "├── frontend/         React + TypeScript + Vite"
	@echo "├── esp32/            ESP32 firmware (PlatformIO)"
	@echo "├── database/         PostgreSQL setup"
	@echo "├── docker/           Docker configurations"
	@echo "├── docs/             Documentation"
	@echo "└── scripts/          Utility scripts"
	@echo ""
	@echo "$(YELLOW)Key URLs:$(NC)"
	@echo "Frontend:     http://localhost:3000"
	@echo "Backend API:  http://localhost:5000/api"
	@echo "API Docs:     http://localhost:5000/api-docs"
	@echo "Database:     postgresql://localhost:5432/smartpen_db"
	@echo "Redis:        redis://localhost:6379"
	@echo ""
	@echo "$(YELLOW)Default Credentials:$(NC)"
	@echo "Admin:        admin@smartpen.com / admin123"
	@echo "Doctor:       doctor@smartpen.com / doctor123"
	@echo "Patient:      patient@smartpen.com / patient123"

## Update all dependencies
update:
	@echo "$(GREEN)Updating dependencies...$(NC)"
	@cd backend && npm update
	@cd frontend && npm update
	@echo "$(GREEN)Dependencies updated!$(NC)"

## Security audit
audit:
	@echo "$(GREEN)Running security audit...$(NC)"
	@cd backend && npm audit
	@cd frontend && npm audit 