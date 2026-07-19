from fastapi import APIRouter, Depends, HTTPException, status, Query
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, or_
from app.database import get_db
from app.models.user import User
from app.auth.router import get_current_user
from app.auth.schemas import UserResponse

router = APIRouter(prefix="/users", tags=["users"])


@router.get("/me", response_model=UserResponse)
async def get_me(current_user: User = Depends(get_current_user)):
    return UserResponse(
        id=current_user.id, email=current_user.email, username=current_user.username,
        display_name=current_user.display_name, avatar_url=current_user.avatar_url,
        is_active=current_user.is_active, created_at=current_user.created_at.isoformat(),
    )


@router.patch("/me", response_model=UserResponse)
async def update_me(
    display_name: str | None = None,
    avatar_url: str | None = None,
    current_user: User = Depends(get_current_user),
    db: AsyncSession = Depends(get_db),
):
    if display_name is not None:
        current_user.display_name = display_name
    if avatar_url is not None:
        current_user.avatar_url = avatar_url
    await db.commit()
    await db.refresh(current_user)
    return UserResponse(
        id=current_user.id, email=current_user.email, username=current_user.username,
        display_name=current_user.display_name, avatar_url=current_user.avatar_url,
        is_active=current_user.is_active, created_at=current_user.created_at.isoformat(),
    )


@router.get("/{user_id}", response_model=UserResponse)
async def get_user(user_id: str, db: AsyncSession = Depends(get_db), _current: User = Depends(get_current_user)):
    result = await db.execute(select(User).where(User.id == user_id))
    user = result.scalar_one_or_none()
    if not user:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="User not found")
    return UserResponse(
        id=user.id, email=user.email, username=user.username,
        display_name=user.display_name, avatar_url=user.avatar_url,
        is_active=user.is_active, created_at=user.created_at.isoformat(),
    )


@router.get("/search/", response_model=list[UserResponse])
async def search_users(
    q: str = Query(..., min_length=1),
    db: AsyncSession = Depends(get_db),
    _current: User = Depends(get_current_user),
):
    pattern = f"%{q}%"
    result = await db.execute(
        select(User).where(or_(User.username.ilike(pattern), User.display_name.ilike(pattern))).limit(50)
    )
    users = result.scalars().all()
    return [
        UserResponse(
            id=u.id, email=u.email, username=u.username,
            display_name=u.display_name, avatar_url=u.avatar_url,
            is_active=u.is_active, created_at=u.created_at.isoformat(),
        )
        for u in users
    ]
