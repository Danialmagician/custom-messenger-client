"""Initial migration - create all tables

Revision ID: 001_initial
Revises:
Create Date: 2026-01-01 00:00:00
"""
from typing import Sequence, Union
from alembic import op
import sqlalchemy as sa

revision: str = "001_initial"
down_revision: Union[str, None] = None
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    chat_type = sa.Enum("private", "group", "channel", "supergroup", name="chattype")
    member_role = sa.Enum("member", "admin", "owner", name="memberrole")
    message_type = sa.Enum("text", "photo", "video", "audio", "document", "sticker", "system", name="messagetype")

    chat_type.create(op.get_bind(), checkfirst=True)
    member_role.create(op.get_bind(), checkfirst=True)
    message_type.create(op.get_bind(), checkfirst=True)

    op.create_table(
        "users",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("email", sa.String(255), unique=True, nullable=False, index=True),
        sa.Column("username", sa.String(64), unique=True, nullable=False, index=True),
        sa.Column("display_name", sa.String(128), nullable=False),
        sa.Column("avatar_url", sa.String(512), nullable=True),
        sa.Column("password_hash", sa.String(255), nullable=False),
        sa.Column("is_active", sa.Boolean(), default=True),
        sa.Column("last_seen", sa.DateTime(timezone=True), nullable=True),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )

    op.create_table(
        "chats",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("title", sa.String(255), nullable=False),
        sa.Column("description", sa.Text(), nullable=True),
        sa.Column("chat_type", chat_type, nullable=False),
        sa.Column("avatar_url", sa.String(512), nullable=True),
        sa.Column("created_by", sa.String(36), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )

    op.create_table(
        "memberships",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("user_id", sa.String(36), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("chat_id", sa.String(36), sa.ForeignKey("chats.id"), nullable=False),
        sa.Column("role", member_role, default="member"),
        sa.Column("is_muted", sa.Boolean(), default=False),
        sa.Column("is_pinned", sa.Boolean(), default=False),
        sa.Column("joined_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )

    op.create_table(
        "messages",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("chat_id", sa.String(36), sa.ForeignKey("chats.id"), nullable=False, index=True),
        sa.Column("sender_id", sa.String(36), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("content", sa.Text(), nullable=False),
        sa.Column("message_type", message_type, default="text"),
        sa.Column("media_url", sa.String(512), nullable=True),
        sa.Column("reply_to_id", sa.String(36), nullable=True),
        sa.Column("is_edited", sa.Boolean(), default=False),
        sa.Column("is_deleted", sa.Boolean(), default=False),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now(), index=True),
        sa.Column("updated_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )

    op.create_table(
        "read_receipts",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("user_id", sa.String(36), sa.ForeignKey("users.id"), nullable=False),
        sa.Column("message_id", sa.String(36), sa.ForeignKey("messages.id"), nullable=False),
        sa.Column("chat_id", sa.String(36), sa.ForeignKey("chats.id"), nullable=False),
        sa.Column("read_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
        sa.UniqueConstraint("user_id", "message_id", name="uq_user_message"),
    )

    op.create_table(
        "notifications",
        sa.Column("id", sa.String(36), primary_key=True),
        sa.Column("user_id", sa.String(36), sa.ForeignKey("users.id"), nullable=False, index=True),
        sa.Column("chat_id", sa.String(36), sa.ForeignKey("chats.id"), nullable=False),
        sa.Column("message_id", sa.String(36), sa.ForeignKey("messages.id"), nullable=True),
        sa.Column("type", sa.String(50), nullable=False),
        sa.Column("content", sa.Text(), nullable=False),
        sa.Column("is_read", sa.Boolean(), default=False),
        sa.Column("created_at", sa.DateTime(timezone=True), server_default=sa.func.now()),
    )


def downgrade() -> None:
    op.drop_table("notifications")
    op.drop_table("read_receipts")
    op.drop_table("messages")
    op.drop_table("memberships")
    op.drop_table("chats")
    op.drop_table("users")
    sa.Enum(name="messagetype").drop(op.get_bind(), checkfirst=True)
    sa.Enum(name="memberrole").drop(op.get_bind(), checkfirst=True)
    sa.Enum(name="chattype").drop(op.get_bind(), checkfirst=True)
