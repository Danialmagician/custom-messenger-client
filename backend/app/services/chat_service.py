from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.models.chat import Chat, ChatType
from app.models.membership import Membership, MemberRole


async def get_user_chats(db: AsyncSession, user_id: str) -> list[Chat]:
    result = await db.execute(
        select(Chat)
        .join(Membership, Membership.chat_id == Chat.id)
        .where(Membership.user_id == user_id)
        .order_by(Chat.updated_at.desc())
    )
    return list(result.scalars().all())


async def create_chat(db: AsyncSession, title: str, creator_id: str, chat_type: ChatType = ChatType.group, description: str | None = None) -> Chat:
    chat = Chat(title=title, description=description, chat_type=chat_type, created_by=creator_id)
    db.add(chat)
    await db.flush()
    db.add(Membership(user_id=creator_id, chat_id=chat.id, role=MemberRole.owner))
    await db.commit()
    await db.refresh(chat)
    return chat


async def add_member(db: AsyncSession, chat_id: str, user_id: str, role: MemberRole = MemberRole.member) -> Membership:
    membership = Membership(user_id=user_id, chat_id=chat_id, role=role)
    db.add(membership)
    await db.commit()
    return membership


async def is_member(db: AsyncSession, chat_id: str, user_id: str) -> bool:
    result = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == user_id)
    )
    return result.scalar_one_or_none() is not None


async def get_chat_members(db: AsyncSession, chat_id: str) -> list[Membership]:
    result = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id)
    )
    return list(result.scalars().all())
