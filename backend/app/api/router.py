from fastapi import APIRouter
from app.api import users, chats, messages, members, files, notifications

router = APIRouter(prefix="/api/v1")
router.include_router(users.router)
router.include_router(chats.router)
router.include_router(messages.router)
router.include_router(members.router)
router.include_router(files.router)
router.include_router(notifications.router)
