# Karagram Architecture

## Overview

Karagram is a custom messenger built on the Telegram Desktop UI, completely independent from Telegram servers. It uses a custom FastAPI backend with PostgreSQL, Redis, and MinIO for file storage.

## System Architecture

```
┌─────────────────────────────────────────┐
│           Karagram Desktop (Qt6/C++)     │
│                                          │
│  ┌──────────────┐  ┌─────────────────┐  │
│  │  Telegram UI  │  │ custom_network  │  │
│  │  (unchanged)  │  │   (REST/WS)     │  │
│  └──────┬───────┘  └───────┬─────────┘  │
│         │                   │            │
│  ┌──────┴───────────────────┴─────────┐  │
│  │        Qt6 Network Layer           │  │
│  └────────────────┬───────────────────┘  │
└───────────────────┼──────────────────────┘
                    │ HTTPS/WSS
┌───────────────────┼──────────────────────┐
│           Karagram Backend (FastAPI)      │
│                    │                     │
│  ┌─────────────────┴──────────────────┐  │
│  │           REST API / WebSocket      │  │
│  └──┬──────────┬──────────┬───────────┘  │
│     │          │          │              │
│  ┌──┴──┐  ┌───┴───┐  ┌───┴────┐        │
│  │ PG  │  │ Redis │  │ MinIO  │        │
│  └─────┘  └───────┘  └────────┘        │
└──────────────────────────────────────────┘
```

## Components

### Desktop Client

The desktop client is a fork of Telegram Desktop with the following modifications:

- **custom_network/** module added under `Telegram/SourceFiles/`
- **CMake flag**: `DESKTOP_APP_CUSTOM_BACKEND=ON` enables the custom backend
- **No MTProto**: When CUSTOM_BACKEND is ON, no connection to Telegram servers is made
- **UI preserved**: All Telegram UI components remain unchanged

### custom_network Module

| File | Purpose |
|------|---------|
| `custom_api.h/cpp` | Main API facade with all REST operations |
| `rest_client.h/cpp` | Low-level HTTP client (GET/POST/PUT/PATCH/DELETE) |
| `websocket_client.h/cpp` | WebSocket client with auto-reconnect |
| `session_manager.h/cpp` | JWT token storage and refresh |
| `custom_auth.h/cpp` | Email/password authentication |
| `data_adapter.h/cpp` | JSON ↔ Qt data model conversion |
| `notification_handler.h/cpp` | Desktop notification management |
| `file_transfer.h/cpp` | File upload/download with progress |

### Backend

| Component | Technology |
|-----------|-----------|
| Framework | FastAPI (Python 3.11) |
| Database | PostgreSQL 16 |
| Cache | Redis 7 |
| File Storage | MinIO (S3-compatible) |
| Auth | JWT + Argon2 |
| WebSocket | FastAPI WebSocket |
| Migrations | Alembic |
| Container | Docker Compose |

## Data Flow

### Authentication

```
User → Email/Password → POST /auth/login → JWT tokens → WebSocket auth
```

### Messaging

```
User types → POST /api/v1/chats/{id}/messages → Backend stores → WebSocket broadcast → Recipients receive
```

### File Transfer

```
User selects file → POST /api/v1/files/upload → MinIO storage → Returns URL → Message with media_url
```

### Real-time Updates

```
WebSocket connection → Backend broadcasts → Typing indicators, read receipts, presence, notifications
```

## Build System

```bash
# Configure with custom backend
cmake -B out -D DESKTOP_APP_CUSTOM_BACKEND=ON -D TDESKTOP_API_TEST=ON

# Build
cmake --build out --config Debug --target Telegram
```

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `DATABASE_URL` | PostgreSQL connection string | `postgresql+asyncpg://karagram:karagram_secret@localhost:5432/karagram` |
| `REDIS_URL` | Redis connection string | `redis://localhost:6379/0` |
| `MINIO_ENDPOINT` | MinIO server address | `localhost:9000` |
| `JWT_SECRET_KEY` | JWT signing key | Required |
| `CORS_ORIGINS` | Allowed CORS origins | `["http://localhost:3000"]` |
