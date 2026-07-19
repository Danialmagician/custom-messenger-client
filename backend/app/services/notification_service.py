from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, update
from app.models.notification import Notification
from app.websocket.manager import manager


async def create_notification(db: AsyncSession, user_id: str, chat_id: str, notif_type: str, content: str, message_id: str | None = None) -> Notification:
    notif = Notification(
        user_id=user_id, chat_id=chat_id, message_id=message_id,
        type=notif_type, content=content,
    )
    db.add(notif)
    await db.commit()
    await db.refresh(notif)
    await manager.send_personal(user_id, {
        "type": "notification",
        "notification": {
            "id": notif.id,
            "chat_id": notif.chat_id,
            "message_id": notif.message_id,
            "type": notif.type,
            "content": notif.content,
            "created_at": notif.created_at.isoformat(),
        },
    })
    return notif


async def get_user_notifications(db: AsyncSession, user_id: str) -> list[Notification]:
    result = await db.execute(
        select(Notification)
        .where(Notification.user_id == user_id)
        .order_by(Notification.created_at.desc())
        .limit(100)
    )
    return list(result.scalars().all())


async def mark_notification_read(db: AsyncSession, notification_id: str) -> bool:
    result = await db.execute(select(Notification).where(Notification.id == notification_id))
    notif = result.scalar_one_or_none()
    if not notif:
        return False
    notif.is_read = True
    await db.commit()
    return True


async def mark_all_read(db: AsyncSession, user_id: str) -> int:
    result = await db.execute(
        update(Notification)
        .where(Notification.user_id == user_id, Notification.is_read == False)
        .values(is_read=True)
    )
    await db.commit()
    return result.rowcount
