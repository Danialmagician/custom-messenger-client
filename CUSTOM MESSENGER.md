# Custom Messenger - Telegram Desktop Fork

A Telegram Desktop fork transformed into an independent messenger with a custom backend.

## Architecture Overview

This project forks the official Telegram Desktop (tdesktop) client and replaces all Telegram infrastructure with a custom backend.

### What Was Modified

1. **MTProto Layer Replaced** → Custom REST/WebSocket networking
2. **Telegram API Replaced** → Custom REST API calls
3. **Authentication Replaced** → Email/password instead of phone numbers
4. **Server Communication** → All requests go to your custom backend

### What Was Preserved

- Complete Telegram UI/UX
- All animations and transitions
- Chat interface and media viewer
- Settings and preferences
- Theming system
- Contact management UI
- Message formatting and display

## Project Structure

```
messenger/
├── tdesktop/                    # Forked Telegram Desktop
│   ├── Telegram/
│   │   ├── SourceFiles/
│   │   │   ├── custom_network/  # NEW: Custom networking layer
│   │   │   │   ├── rest_client.h/cpp      # HTTP REST client
│   │   │   │   ├── websocket_client.h/cpp # WebSocket for real-time
│   │   │   │   ├── api_facade.h/cpp       # API interface
│   │   │   │   └── auth_module.h/cpp      # Custom authentication
│   │   │   ├── mtproto/         # ORIGINAL: MTProto (disabled)
│   │   │   ├── api/             # ORIGINAL: Telegram API (disabled)
│   │   │   ├── intro/           # MODIFIED: Login screens
│   │   │   ├── ui/              # PRESERVED: All UI components
│   │   │   ├── main.cpp         # MODIFIED: Entry point
│   │   │   └── ...
│   │   ├── Resources/           # PRESERVED: Icons, themes, strings
│   │   └── CMakeLists.txt       # MODIFIED: Build configuration
│   └── ...
├── backend/                     # Custom backend (Python/FastAPI)
│   └── ...
└── README.md
```

## Backend API Endpoints

The client communicates with these endpoints:

### Authentication
- `POST /api/v1/auth/register` - Register new account
- `POST /api/v1/auth/login` - Login
- `POST /api/v1/auth/refresh` - Refresh JWT tokens
- `GET /api/v1/auth/me` - Get current user
- `PATCH /api/v1/auth/me` - Update profile

### Messages
- `POST /api/v1/messages` - Send message
- `GET /api/v1/messages/direct/{userId}` - Get direct messages
- `GET /api/v1/messages/group/{groupId}` - Get group messages
- `GET /api/v1/messages/channel/{channelId}` - Get channel messages
- `PATCH /api/v1/messages/{id}` - Edit message
- `DELETE /api/v1/messages/{id}` - Delete message

### Groups
- `POST /api/v1/groups` - Create group
- `GET /api/v1/groups` - List my groups
- `POST /api/v1/groups/{id}/members` - Add member
- `DELETE /api/v1/groups/{id}/members/{userId}` - Remove member

### Channels
- `POST /api/v1/channels` - Create channel
- `GET /api/v1/channels` - List my channels
- `POST /api/v1/channels/{id}/subscribe` - Subscribe
- `POST /api/v1/channels/{id}/unsubscribe` - Unsubscribe

### Files
- `POST /api/v1/files/upload` - Upload file
- `GET /api/v1/files/{id}/download` - Download file

### WebSocket
- `ws://localhost:8000/ws?token={jwt}` - Real-time messaging

## Building

### Prerequisites

- Visual Studio 2022 (Windows)
- Qt 6.x
- CMake 3.x

### Build Commands

```powershell
# From Visual Studio Native Tools Command Prompt
cd tdesktop
cmake --build out --config Debug --target Telegram
```

The executable will be at `out/Debug/Telegram.exe`.

## Configuration

### Backend URL

The client connects to the backend at:
- REST API: `http://localhost:8000/api/v1`
- WebSocket: `ws://localhost:8000/ws`

These can be configured in the settings or via environment variables.

## What Was Removed

1. **MTProto Protocol** - All encryption and handshake code
2. **Telegram API Calls** - All `MTP::` prefixed functions
3. **Phone Number Auth** - Replaced with email/password
4. **DC Connection Management** - No longer needed
5. **Telegram Server Dependencies** - All removed

## What Was Added

1. **REST Client** - Standard HTTP client for API calls
2. **WebSocket Client** - Real-time messaging support
3. **API Facade** - Clean interface for all backend operations
4. **Auth Module** - Email/password authentication with JWT
5. **Custom Backend** - Python/FastAPI server

## Development

### Adding New Features

1. Add API endpoint to backend
2. Add method to `ApiFacade`
3. Wire up to existing UI or create new UI component

### Testing

```bash
cd backend
pytest tests/ -v
```

## License

This project inherits the Telegram Desktop license. See `LEGAL` file.
