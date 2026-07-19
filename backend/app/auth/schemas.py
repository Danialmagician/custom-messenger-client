from pydantic import BaseModel, EmailStr, Field


class RegisterRequest(BaseModel):
    email: EmailStr
    username: str = Field(min_length=3, max_length=64)
    display_name: str = Field(min_length=1, max_length=128)
    password: str = Field(min_length=8, max_length=128)


class LoginRequest(BaseModel):
    email: EmailStr
    password: str
    remember_me: bool = False


class RefreshRequest(BaseModel):
    refresh_token: str


class TokenResponse(BaseModel):
    access_token: str
    refresh_token: str
    token_type: str = "bearer"


class UserResponse(BaseModel):
    id: str
    email: str
    username: str
    display_name: str
    avatar_url: str | None = None
    is_active: bool
    created_at: str

    class Config:
        from_attributes = True


class MessageResponse(BaseModel):
    id: str
    chat_id: str
    sender_id: str
    content: str
    message_type: str
    media_url: str | None = None
    reply_to_id: str | None = None
    is_edited: bool = False
    is_deleted: bool = False
    created_at: str

    class Config:
        from_attributes = True


class ChatResponse(BaseModel):
    id: str
    title: str
    description: str | None = None
    chat_type: str
    avatar_url: str | None = None
    created_by: str
    created_at: str
    unread_count: int = 0

    class Config:
        from_attributes = True


class SendMessageRequest(BaseModel):
    content: str
    message_type: str = "text"
    media_url: str | None = None
    reply_to_id: str | None = None


class UpdateMessageRequest(BaseModel):
    content: str


class CreateChatRequest(BaseModel):
    title: str
    description: str | None = None
    chat_type: str = "group"
    member_ids: list[str] = []


class MemberRequest(BaseModel):
    user_id: str
    role: str = "member"
