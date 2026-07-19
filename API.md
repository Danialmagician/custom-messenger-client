# Karagram API Documentation

## Base URL

```
http://localhost:8000
```

## Authentication

All API endpoints (except auth) require a Bearer token:

```
Authorization: Bearer <access_token>
```

### POST /auth/register

Register a new user.

**Request:**
```json
{
  "email": "user@example.com",
  "username": "johndoe",
  "display_name": "John Doe",
  "password": "securepassword123"
}
```

**Response (201):**
```json
{
  "id": "uuid",
  "email": "user@example.com",
  "username": "johndoe",
  "display_name": "John Doe",
  "avatar_url": null,
  "is_active": true,
  "created_at": "2026-01-01T00:00:00Z"
}
```

### POST /auth/login

Login with email and password.

**Request:**
```json
{
  "email": "user@example.com",
  "password": "securepassword123",
  "remember_me": true
}
```

**Response (200):**
```json
{
  "access_token": "jwt...",
  "refresh_token": "jwt...",
  "token_type": "bearer"
}
```

### POST /auth/refresh

Refresh access token.

**Request:**
```json
{
  "refresh_token": "jwt..."
}
```

### POST /auth/logout

Logout (invalidate tokens).

### GET /auth/me

Get current user profile.

## Users

### GET /api/v1/users/me

Get current user profile.

### PATCH /api/v1/users/me

Update current user profile.

**Query Parameters:**
- `display_name` (optional)
- `avatar_url` (optional)

### GET /api/v1/users/{user_id}

Get user by ID.

### GET /api/v1/users/search/?q={query}

Search users by username or display name.

## Chats

### GET /api/v1/chats/

List all chats for current user.

### POST /api/v1/chats/

Create a new chat.

**Request:**
```json
{
  "title": "My Group",
  "description": "A group chat",
  "chat_type": "group",
  "member_ids": ["user-uuid-1", "user-uuid-2"]
}
```

### GET /api/v1/chats/{chat_id}

Get chat details.

### PATCH /api/v1/chats/{chat_id}

Update chat (admin/owner only).

### DELETE /api/v1/chats/{chat_id}

Delete chat (creator only).

## Messages

### GET /api/v1/chats/{chat_id}/messages

Get messages for a chat (paginated).

**Query Parameters:**
- `offset` (default: 0)
- `limit` (default: 50, max: 200)

### POST /api/v1/chats/{chat_id}/messages

Send a message.

**Request:**
```json
{
  "content": "Hello, world!",
  "message_type": "text",
  "media_url": null,
  "reply_to_id": null
}
```

### PATCH /api/v1/messages/{message_id}

Edit a message (sender only).

### DELETE /api/v1/messages/{message_id}

Delete a message (sender only, soft delete).

## Members

### POST /api/v1/chats/{chat_id}/members

Add a member (admin/owner only).

### DELETE /api/v1/chats/{chat_id}/members/{user_id}

Remove a member (admin/owner only).

### PATCH /api/v1/chats/{chat_id}/members/{user_id}?role={role}

Update member role (owner only).

## Files

### POST /api/v1/files/upload

Upload a file (multipart/form-data).

**Response:**
```json
{
  "id": "file-uuid",
  "filename": "image.png",
  "object_name": "uuid.png",
  "size": 12345,
  "content_type": "image/png",
  "url": "/api/v1/files/file-uuid/download"
}
```

### GET /api/v1/files/{file_id}

Get file metadata.

### GET /api/v1/files/{file_id}/download

Download file content.

## Notifications

### GET /api/v1/notifications/

List notifications (last 100).

### PATCH /api/v1/notifications/{notification_id}/read

Mark notification as read.

### POST /api/v1/notifications/read-all

Mark all notifications as read.

## WebSocket

Connect to `ws://localhost:8000/ws?token={access_token}`

### Events

**Client → Server:**
```json
{"type": "join_chat", "chat_id": "chat-uuid"}
{"type": "leave_chat", "chat_id": "chat-uuid"}
{"type": "typing", "chat_id": "chat-uuid"}
{"type": "read_receipt", "chat_id": "chat-uuid", "message_id": "msg-uuid"}
{"type": "presence", "status": "online"}
{"type": "ping"}
```

**Server → Client:**
```json
{"type": "message", "id": "...", "chat_id": "...", "sender_id": "...", "content": "...", "message_type": "text"}
{"type": "typing", "user_id": "...", "chat_id": "..."}
{"type": "read_receipt", "user_id": "...", "chat_id": "...", "message_id": "..."}
{"type": "presence", "user_id": "...", "status": "online"}
{"type": "notification", "notification": {...}}
{"type": "pong"}
```

## Health Check

### GET /health

```json
{"status": "ok", "service": "karagram-backend"}
```
