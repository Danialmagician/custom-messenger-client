from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from app.auth.security import verify_token
from app.websocket.manager import manager
import json
import logging

logger = logging.getLogger(__name__)
router = APIRouter()


@router.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket, token: str = ""):
    if not token:
        await websocket.close(code=4001, reason="Missing token")
        return
    payload = verify_token(token)
    if not payload or payload.get("type") != "access":
        await websocket.close(code=4001, reason="Invalid token")
        return
    user_id = payload.get("sub")
    if not user_id:
        await websocket.close(code=4001, reason="Invalid token payload")
        return

    await manager.connect(websocket, user_id)
    try:
        while True:
            data = await websocket.receive_json()
            event_type = data.get("type")

            if event_type == "join_chat":
                chat_id = data.get("chat_id")
                if chat_id:
                    manager.join_room(websocket, chat_id)
                    await manager.broadcast_to_room(chat_id, {
                        "type": "user_joined",
                        "user_id": user_id,
                    }, exclude=websocket)

            elif event_type == "leave_chat":
                chat_id = data.get("chat_id")
                if chat_id:
                    manager.leave_room(websocket, chat_id)
                    await manager.broadcast_to_room(chat_id, {
                        "type": "user_left",
                        "user_id": user_id,
                    })

            elif event_type == "typing":
                chat_id = data.get("chat_id")
                if chat_id:
                    await manager.broadcast_to_room(chat_id, {
                        "type": "typing",
                        "user_id": user_id,
                        "chat_id": chat_id,
                    }, exclude=websocket)

            elif event_type == "read_receipt":
                chat_id = data.get("chat_id")
                message_id = data.get("message_id")
                if chat_id and message_id:
                    await manager.broadcast_to_room(chat_id, {
                        "type": "read_receipt",
                        "user_id": user_id,
                        "chat_id": chat_id,
                        "message_id": message_id,
                    }, exclude=websocket)

            elif event_type == "presence":
                await manager.send_personal(user_id, {
                    "type": "presence",
                    "user_id": user_id,
                    "status": data.get("status", "online"),
                })

            elif event_type == "ping":
                await websocket.send_json({"type": "pong"})

    except WebSocketDisconnect:
        manager.disconnect(websocket, user_id)
        for room_id in list(manager._connections.keys()):
            await manager.broadcast_to_room(room_id, {
                "type": "user_left",
                "user_id": user_id,
            })
    except Exception as e:
        logger.error(f"WebSocket error for user {user_id}: {e}")
        manager.disconnect(websocket, user_id)
