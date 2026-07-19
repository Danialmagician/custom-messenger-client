from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.database import get_db
from app.models.user import User
from app.models.membership import Membership, MemberRole
from app.auth.router import get_current_user
from app.auth.schemas import MemberRequest

router = APIRouter(tags=["members"])


@router.post("/chats/{chat_id}/members", status_code=status.HTTP_201_CREATED)
async def add_member(
    chat_id: str,
    req: MemberRequest,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    caller = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    caller_mem = caller.scalar_one_or_none()
    if not caller_mem or caller_mem.role not in (MemberRole.admin, MemberRole.owner):
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Insufficient permissions")
    existing = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == req.user_id)
    )
    if existing.scalar_one_or_none():
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="Already a member")
    membership = Membership(user_id=req.user_id, chat_id=chat_id, role=MemberRole(req.role))
    db.add(membership)
    await db.commit()
    return {"message": "Member added"}


@router.delete("/chats/{chat_id}/members/{user_id}", status_code=status.HTTP_204_NO_CONTENT)
async def remove_member(
    chat_id: str,
    user_id: str,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    caller = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    caller_mem = caller.scalar_one_or_none()
    if not caller_mem or caller_mem.role not in (MemberRole.admin, MemberRole.owner):
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Insufficient permissions")
    result = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == user_id)
    )
    membership = result.scalar_one_or_none()
    if not membership:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Member not found")
    if membership.role == MemberRole.owner:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Cannot remove owner")
    await db.delete(membership)
    await db.commit()


@router.patch("/chats/{chat_id}/members/{user_id}")
async def update_member_role(
    chat_id: str,
    user_id: str,
    role: str,
    db: AsyncSession = Depends(get_db),
    current_user: User = Depends(get_current_user),
):
    caller = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == current_user.id)
    )
    caller_mem = caller.scalar_one_or_none()
    if not caller_mem or caller_mem.role != MemberRole.owner:
        raise HTTPException(status_code=status.HTTP_403_FORBIDDEN, detail="Only owner can change roles")
    result = await db.execute(
        select(Membership).where(Membership.chat_id == chat_id, Membership.user_id == user_id)
    )
    membership = result.scalar_one_or_none()
    if not membership:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Member not found")
    membership.role = MemberRole(role)
    await db.commit()
    return {"message": "Role updated"}
