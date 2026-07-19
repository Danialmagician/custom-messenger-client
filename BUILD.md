# Building Karagram

## Prerequisites

### Desktop Client

- Windows: Visual Studio 2022 with C++ workload
- CMake 3.25+
- Python 3.11+
- Qt 6.7+ (installed via prepare.py)

### Backend

- Python 3.11+
- Docker & Docker Compose (recommended)
- PostgreSQL 16 (if not using Docker)
- Redis 7 (if not using Docker)

## Quick Start

### 1. Backend

```bash
cd backend

# Using Docker Compose
docker compose up -d

# Run migrations
docker compose exec app alembic upgrade head

# Verify
curl http://localhost:8000/health
```

### 2. Desktop Client

#### Windows (Native Tools Command Prompt x64)

```bash
# Clone with submodules
git clone --recursive https://github.com/your-org/Karagram.git
cd Karagram

# Prepare dependencies
cd Telegram
python build/prepare/prepare.py

# Configure with custom backend
call configure.bat -D DESKTOP_APP_CUSTOM_BACKEND=ON -D TDESKTOP_API_TEST=ON

# Build
cmake --build ..\out --config Debug --target Telegram
```

#### Using WSL

```bash
cd Telegram
Telegram/build/docker/centos_env/build_debug.sh
```

## Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `DESKTOP_APP_CUSTOM_BACKEND` | Enable custom backend | OFF |
| `TDESKTOP_API_TEST` | Use test API credentials | OFF |
| `DESKTOP_APP_DISABLE_AUTOUPDATE` | Disable auto-updates | OFF |
| `DESKTOP_APP_DISABLE_CRASH_REPORTS` | Disable crash reporting | OFF |
| `CMAKE_CONFIGURATION_TYPES` | Build type | Release |

## CI/CD

GitHub Actions workflow at `.github/workflows/karagram-win.yml`:

- Builds Windows x64 with custom backend
- Verifies backend Python modules
- Uploads artifacts

```bash
# Trigger manually
gh workflow run karagram-win.yml
```

## Troubleshooting

### Libraries not found

Ensure the repository is at the correct path relative to Libraries:
```
L:\Telegram\Karagram\
L:\Telegram\win64\Libraries\    (for x64)
```

### Build fails with "wrong command prompt"

Use the correct Visual Studio Native Tools Command Prompt for your target architecture.

### Backend connection refused

Ensure Docker services are running:
```bash
docker compose ps
docker compose logs app
```
