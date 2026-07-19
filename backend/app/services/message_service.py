from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func, and_
from app.models.message import Message, MessageType
from app.models.read_receipt import ReadReceipt


async def get_chat_messages(db: AsyncSession, chat_id: str, offset: int = 0, limit: int = 50) -> list[Message]:
    result = await db.execute(
        select(Message)
        .where(Message.chat_id == chat_id)
        .order_by(Message.created_at.desc())
        .offset(offset)
        .limit(limit)
    )
    return list(reversed(result.scalars().all()))


async def send_message(db: AsyncSession, chat_id: str, sender_id: str, content: str, message_type: MessageType = MessageType.text, media_url: str | None = None, reply_to_id: str | None = None) -> Message:
    msg = Message(
        chat_id=chat_id, sender_id=sender_id, content=content,
        message_type=message_type, media_url=media_url, reply_to_id=reply_to_id,
    )
    db.add(msg)
    await db.commit()
    await db.refresh(msg)
    return msg


async def edit_message(db: AsyncSession, message_id: str, user_id: str, content: str) -> Message | None:
    result = await db.execute(select(Message).where(Message.id == message_id))
    msg = result.scalar_one_or_none()
    if not msg or msg.sender_id != user_id:
        return None
    msg.content = content
    msg.is_edited = True
    await db.commit()
    await db.refresh(msg)
    return msg


async def delete_message(db: AsyncSession, message_id: str, user_id: str) -> bool:
    result = await db.execute(select(Message).where(Message.id == message_id))
    msg = result.scalar_one_or_none()
    if not msg or msg.sender_id != user_id:
        return False
    msg.is_deleted = True
    msg.content = "Message deleted"
    await db.commit()
    return True


async def mark_read(db: AsyncSession, user_id: str, chat_id: str, message_id: str) -> ReadReceipt:
    existing = await db.execute(
        select(ReadReceipt).where(ReadReceipt.user_id == user_id, ReadReceipt.message_id == message_id)
    )
    if existing.scalar_one_or_none():
        return existing.scalar_one_or_none()
    receipt = ReadReceipt(user_id=user_id, message_id=message_id, chat_id=chat_id)
    db.add(receipt)
    await db.commit()
    await db.refresh(receipt)
    return receipt


async def get_unread_count(db: AsyncSession, user_id: str, chat_id: str) -> int:
    last_read = await db.execute(
        select(ReadReceipt)
        .where(ReadReceipt.user_id == user_id, ReadReceipt.chat_id == chat_id)
        .order_by(ReadReceipt.read_at.desc())
        .limit(1)
    )
    receipt = last_read.scalar_one_or_none()
    if not receipt:
        result = await db.execute(
            select(func.count(Message.id)).where(Message.chat_id == chat_id)
        )
        return result.scalar() or 0
    result = await db.execute(
        select(func.count(Message.id)).where(
            Message.chat_id == chat_id,
            Message.created_at > receipt.read_at,
        )
    )
    return result.scalar() or 0
