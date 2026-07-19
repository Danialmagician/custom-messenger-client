from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.database import get_db
from app.models.user import User
from app.models.chat import Chat, ChatType
from app.models.membership import Membership, MemberRole
from app.auth.router import get_current_user
from app.auth.schemas import ChatResponse, CreateChatRequest

router = APIRouter(prefix="/chats", tags=["chats"])


@router.get("/", response_model=list[ChatResponse])
async def list_chats(
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(
        select(Chat)
        .join(Membership, Membership.chat_id == Chat.id)
        .where(Membership.user_id == current_user.id)
        .order_by(Chat.updated_at.desc())
    )
    chats = result.scalars().all()
    return [
        ChatResponse(
            id=c.id, title=c.title, description=c.description,
            chat_type=c.chat_type.value, avatar_url=c.avatar_url,
            created_by=c.created_by, created_at=c.created_at.isoformat(),
        )
        for c in chats
    ]


@router.post("/", response_model=ChatResponse, status_code=status.HTTP_201_CREATED)
async def create_chat(
    req: CreateChatRequest,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    chat = Chat(
        title=req.title,
        description=req.description,
        chat_type=ChatType(req.chat_type),
        created_by=current_user.id,
    )
    db.add(chat)
    await db.flush()
    owner_membership = Membership(user_id=current_user.id, chat_id=chat.id, role=MemberRole.owner)
    db.add(owner_membership)
    for mid in req.member_ids:
        if mid != current_user.id:
            db.add(Membership(user_id=mid, chat_id=chat.id, role=MemberRole.member))
    await db.commit()
    await db.refresh(chat)
    return ChatResponse(
        id=chat.id, title=chat.title, description=chat.description,
        chat_type=chat.chat_type.value, avatar_url=chat.avatar_url,
        created_by=chat.created_by, created_at=chat.created_at.isoformat(),
    )


@router.get("/{chat_id}", response_model=ChatResponse)
async def get_chat(
    chat_id: str,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(select(Chat).where(Chat.id == chat_id))
    chat = result.scalar_one_or_none()
    if not chat:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Chat not found")
    membership = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    if not membership.scalar_one_or_none():
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Not a member")
    return ChatResponse(
        id=chat.id, title=chat.title, description=chat.description,
        chat_type=chat.chat_type.value, avatar_url=chat.avatar_url,
        created_by=chat.created_by, created_at=chat.created_at.isoformat(),
    )


@router.patch("/{chat_id}", response_model=ChatResponse)
async def update_chat(
    chat_id: str,
    title: str | None = None,
    description: str | None = None,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(select(Chat).where(Chat.id == chat_id))
    chat = result.scalar_one_or_none()
    if not chat:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Chat not found")
    membership = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    mem = membership.scalar_one_or_none()
    if not mem or mem.role not in (MemberRole.admin, MemberRole.owner):
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Insufficient permissions")
    if title is not None:
        chat.title = title
    if description is not None:
        chat.description = description
    await db.commit()
    await db.refresh(chat)
    return ChatResponse(
        id=chat.id, title=chat.title, description=chat.description,
        chat_type=chat.chat_type.value, avatar_url=chat.avatar_url,
        created_by=chat.created_by, created_at=chat.created_at.isoformat(),
    )


@router.delete("/{chat_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_chat(
    chat_id: str,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(select(Chat).where(Chat.id == chat_id))
    chat = result.scalar_one_or_none()
    if not chat:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Chat not found")
    if chat.created_by != current_user.id:
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Only creator can delete")
    await db.delete(chat)
    await db.commit()
