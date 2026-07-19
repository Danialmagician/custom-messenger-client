from fastapi import APIRouter, Depends, HTTPException, status, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from app.database import get_db
from app.models.user import User
from app.models.chat import Chat
from app.models.message import Message
from app.models.membership import Membership
from app.models.read_receipt import ReadReceipt
from app.auth.router import get_current_user
from app.auth.schemas import MessageResponse, SendMessageRequest, UpdateMessageRequest

router = APIRouter(tags=["messages"])


@router.get("/chats/{chat_id}/messages", response_model=list[MessageResponse])
async def list_messages(
    chat_id: str,
    offset: int = Query(0, ge=0),
    limit: int = Query(50, ge=1, le=200),
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    membership = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    if not membership.scalar_one_or_none():
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Not a member")
    result = await db.execute(
        select(Message)
        .where(Message.chat_id == chat_id)
        .order_by(Message.created_at.desc())
        .offset(offset)
        .limit(limit)
    )
    msgs = result.scalars().all()
    return [
        MessageResponse(
            id=m.id, chat_id=m.chat_id, sender_id=m.sender_id,
            content=m.content, message_type=m.message_type.value,
            media_url=m.media_url, reply_to_id=m.reply_to_id,
            is_edited=m.is_edited, is_deleted=m.is_deleted,
            created_at=m.created_at.isoformat(),
        )
        for m in reversed(msgs)
    ]


@router.post("/chats/{chat_id}/messages", response_model=MessageResponse, status_code=status.HTTP_201_CREATED)
async def send_message(
    chat_id: str,
    req: SendMessageRequest,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    membership = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    if not membership.scalar_one_or_none():
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Not a member")
    msg = Message(
        chat_id=chat_id,
        sender_id=current_user.id,
        content=req.content,
        message_type=req.message_type,
        media_url=req.media_url,
        reply_to_id=req.reply_to_id,
    )
    db.add(msg)
    await db.commit()
    await db.refresh(msg)
    return MessageResponse(
        id=msg.id, chat_id=msg.chat_id, sender_id=msg.sender_id,
        content=msg.content, message_type=msg.message_type.value,
        media_url=msg.media_url, reply_to_id=msg.reply_to_id,
        is_edited=msg.is_edited, is_deleted=msg.is_deleted,
        created_at=msg.created_at.isoformat(),
    )


@router.patch("/messages/{message_id}", response_model=MessageResponse)
async def edit_message(
    message_id: str,
    req: UpdateMessageRequest,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(select(Message).where(Message.id == message_id))
    msg = result.scalar_one_or_none()
    if not msg:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Message not found")
    if msg.sender_id != current_user.id:
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Cannot edit others' messages")
    msg.content = req.content
    msg.is_edited = True
    await db.commit()
    await db.refresh(msg)
    return MessageResponse(
        id=msg.id, chat_id=msg.chat_id, sender_id=msg.sender_id,
        content=msg.content, message_type=msg.message_type.value,
        media_url=msg.media_url, reply_to_id=msg.reply_to_id,
        is_edited=msg.is_edited, is_deleted=msg.is_deleted,
        created_at=msg.created_at.isoformat(),
    )


@router.delete("/messages/{message_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_message(
    message_id: str,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    result = await db.execute(select(Message).where(Message.id == message_id))
    msg = result.scalar_one_or_none()
    if not msg:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Message not found")
    if msg.sender_id != current_user.id:
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Cannot delete others' messages")
    msg.is_deleted = True
    msg.content = "Message deleted"
    await db.commit()
