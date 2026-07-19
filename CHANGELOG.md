# Changelog

## [1.0.0] - 2026-07-19

### Added

- FastAPI backend with PostgreSQL, Redis, MinIO
- Email/password authentication with JWT tokens
- WebSocket real-time messaging
- File upload/download via MinIO
- Desktop notification system
- custom_network C++ module for Telegram Desktop
- REST client with auto-retry and token refresh
- WebSocket client with auto-reconnect
- Session manager with secure token storage
- Data adapter for JSON ↔ Qt conversion
- GitHub Actions CI for Windows x64
- Complete API documentation
- Database schema documentation
- Deployment guide with Docker Compose

### Modified

- CMakeLists.txt: Added DESKTOP_APP_CUSTOM_BACKEND option
- Telegram/CMakeLists.txt: Added custom_network conditional build

### Architecture

- Backend: FastAPI + SQLAlchemy + Alembic + PostgreSQL + Redis + MinIO
- Desktop: Qt6/C++ with custom_network module
- Communication: REST API + WebSocket
- Authentication: JWT + Argon2 password hashing
- File Storage: MinIO (S3-compatible)
