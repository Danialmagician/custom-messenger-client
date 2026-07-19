# Karagram Deployment

## Docker Compose (Recommended)

### 1. Clone and configure

```bash
git clone --recursive https://github.com/your-org/Karagram.git
cd Karagram/backend
cp .env.example .env
# Edit .env with your settings
```

### 2. Start services

```bash
docker compose up -d
```

### 3. Run migrations

```bash
docker compose exec app alembic upgrade head
```

### 4. Verify

```bash
curl http://localhost:8000/health
# {"status": "ok", "service": "karagram-backend"}
```

## Production Configuration

### Environment Variables

```env
DATABASE_URL=postgresql+asyncpg://karagram:STRONG_PASSWORD@db:5432/karagram
REDIS_URL=redis://redis:6379/0
MINIO_ENDPOINT=minio:9000
MINIO_ACCESS_KEY=MINIO_USER
MINIO_SECRET_KEY=MINIO_STRONG_PASSWORD
MINIO_BUCKET=karagram-files
JWT_SECRET_KEY=GENERATE_64_CHAR_RANDOM_STRING
JWT_ALGORITHM=HS256
ACCESS_TOKEN_EXPIRE_MINUTES=30
REFRESH_TOKEN_EXPIRE_DAYS=7
CORS_ORIGINS=["https://your-domain.com"]
```

### Generate JWT Secret

```bash
python -c "import secrets; print(secrets.token_hex(32))"
```

### SSL/TLS

Place behind a reverse proxy (nginx/caddy) with TLS termination:

```nginx
server {
    listen 443 ssl;
    server_name api.karagram.dev;

    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;

    location / {
        proxy_pass http://localhost:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    location /ws {
        proxy_pass http://localhost:8000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

## Desktop Client Deployment

### Build Release

```bash
cmake -B out -D DESKTOP_APP_CUSTOM_BACKEND=ON -D CMAKE_CONFIGURATION_TYPES=Release
cmake --build out --config Release
```

### Configure Backend URL

Set the backend URL in the app settings or via environment variable:

```bash
KARAGRAM_BACKEND_URL=https://api.karagram.dev ./Telegram.exe
```

## Monitoring

### Health Check

```bash
curl http://localhost:8000/health
```

### Logs

```bash
docker compose logs -f app
```

### Redis Cache

```bash
docker compose exec redis redis-cli
```

## Backup

### PostgreSQL

```bash
docker compose exec db pg_dump -U karagram karagram > backup.sql
```

### MinIO

```bash
docker compose exec minio mc mirror /data /backup/minio
```
