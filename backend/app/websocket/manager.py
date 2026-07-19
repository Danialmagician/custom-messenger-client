from fastapi import WebSocket
from typing import Dict, Set
import json
import logging

logger = logging.getLogger(__name__)


class ConnectionManager:
    def __init__(self):
        self._connections: Dict[str, Set[WebSocket]] = {}
        self._user_connections: Dict[str, Set[WebSocket]] = {}

    async def connect(self, websocket: WebSocket, user_id: str):
        await websocket.accept()
        if user_id not in self._user_connections:
            self._user_connections[user_id] = set()
        self._user_connections[user_id].add(websocket)
        logger.info(f"User {user_id} connected. Total connections: {sum(len(v) for v in self._user_connections.values())}")

    def disconnect(self, websocket: WebSocket, user_id: str):
        if user_id in self._user_connections:
            self._user_connections[user_id].discard(websocket)
            if not self._user_connections[user_id]:
                del self._user_connections[user_id]
        for room_id, room_conns in list(self._connections.items()):
            room_conns.discard(websocket)
        logger.info(f"User {user_id} disconnected.")

    def join_room(self, websocket: WebSocket, room_id: str):
        if room_id not in self._connections:
            self._connections[room_id] = set()
        self._connections[room_id].add(websocket)

    def leave_room(self, websocket: WebSocket, room_id: str):
        if room_id in self._connections:
            self._connections[room_id].discard(websocket)

    async def send_personal(self, user_id: str, message: dict):
        if user_id in self._user_connections:
            dead = set()
            for ws in self._user_connections[user_id]:
                try:
                    await ws.send_json(message)
                except Exception:
                    dead.add(ws)
            self._user_connections[user_id] -= dead

    async def broadcast_to_room(self, room_id: str, message: dict, exclude: WebSocket | None = None):
        if room_id in self._connections:
            dead = set()
            for ws in self._connections[room_id]:
                if ws is exclude:
                    continue
                try:
                    await ws.send_json(message)
                except Exception:
                    dead.add(ws)
            self._connections[room_id] -= dead

    def get_online_users(self) -> list[str]:
        return list(self._user_connections.keys())

    def is_online(self, user_id: str) -> bool:
        return user_id in self._user_connections and len(self._user_connections[user_id]) > 0


manager = ConnectionManager()
