# Karagram Database Schema

## Tables

### users

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| email | VARCHAR(255) | UNIQUE, NOT NULL, INDEX |
| username | VARCHAR(64) | UNIQUE, NOT NULL, INDEX |
| display_name | VARCHAR(128) | NOT NULL |
| avatar_url | VARCHAR(512) | NULLABLE |
| password_hash | VARCHAR(255) | NOT NULL |
| is_active | BOOLEAN | DEFAULT TRUE |
| last_seen | TIMESTAMP WITH TIME ZONE | NULLABLE |
| created_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |
| updated_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |

### chats

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| title | VARCHAR(255) | NOT NULL |
| description | TEXT | NULLABLE |
| chat_type | ENUM(private, group, channel, supergroup) | NOT NULL |
| avatar_url | VARCHAR(512) | NULLABLE |
| created_by | VARCHAR(36) | FK → users.id |
| created_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |
| updated_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |

### memberships

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| user_id | VARCHAR(36) | FK → users.id |
| chat_id | VARCHAR(36) | FK → chats.id |
| role | ENUM(member, admin, owner) | DEFAULT member |
| is_muted | BOOLEAN | DEFAULT FALSE |
| is_pinned | BOOLEAN | DEFAULT FALSE |
| joined_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |

### messages

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| chat_id | VARCHAR(36) | FK → chats.id, INDEX |
| sender_id | VARCHAR(36) | FK → users.id |
| content | TEXT | NOT NULL |
| message_type | ENUM(text, photo, video, audio, document, sticker, system) | DEFAULT text |
| media_url | VARCHAR(512) | NULLABLE |
| reply_to_id | VARCHAR(36) | NULLABLE |
| is_edited | BOOLEAN | DEFAULT FALSE |
| is_deleted | BOOLEAN | DEFAULT FALSE |
| created_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW(), INDEX |
| updated_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |

### read_receipts

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| user_id | VARCHAR(36) | FK → users.id |
| message_id | VARCHAR(36) | FK → messages.id |
| chat_id | VARCHAR(36) | FK → chats.id |
| read_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |
| | | UNIQUE(user_id, message_id) |

### notifications

| Column | Type | Constraints |
|--------|------|-------------|
| id | VARCHAR(36) | PRIMARY KEY |
| user_id | VARCHAR(36) | FK → users.id, INDEX |
| chat_id | VARCHAR(36) | FK → chats.id |
| message_id | VARCHAR(36) | FK → messages.id, NULLABLE |
| type | VARCHAR(50) | NOT NULL |
| content | TEXT | NOT NULL |
| is_read | BOOLEAN | DEFAULT FALSE |
| created_at | TIMESTAMP WITH TIME ZONE | DEFAULT NOW() |

## Relationships

```
users 1──N memberships N──1 chats
users 1──N messages
chats 1──N messages
users 1──N read_receipts
messages 1──N read_receipts
users 1──N notifications
chats 1──N notifications
```

## Migrations

```bash
# Run all migrations
alembic upgrade head

# Create new migration
alembic revision --autogenerate -m "description"

# Rollback one step
alembic downgrade -1
```
