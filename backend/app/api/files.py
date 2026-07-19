import uuid
from fastapi import APIRouter, Depends, HTTPException, UploadFile, File, status
from sqlalchemy.ext.asyncio import AsyncSession
from app.database import get_db
from app.models.user import User
from app.auth.router import get_current_user
from minio import Minio
from app.config import settings
from fastapi.responses import StreamingResponse
import io

router = APIRouter(prefix="/files", tags=["files"])

minio_client = Minio(
    settings.MINIO_ENDPOINT,
    access_key=settings.MINIO_ACCESS_KEY,
    secret_key=settings.MINIO_SECRET_KEY,
    secure=False,
)


def ensure_bucket():
    if not minio_client.bucket_exists(settings.MINIO_BUCKET):
        minio_client.make_bucket(settings.MINIO_BUCKET)


@router.post("/upload")
async def upload_file(
    file: UploadFile = File(...),
    current_user: User = Depends(get_current_user),
):
    ensure_bucket()
    file_id = str(uuid.uuid4())
    ext = file.filename.rsplit(".", 1)[-1] if "." in (file.filename or "") else "bin"
    object_name = f"{file_id}.{ext}"
    content = await file.read()
    minio_client.put_object(
        settings.MINIO_BUCKET,
        object_name,
        io.BytesIO(content),
        length=len(content),
        content_type=file.content_type or "application/octet-stream",
    )
    return {
        "id": file_id,
        "filename": file.filename,
        "object_name": object_name,
        "size": len(content),
        "content_type": file.content_type,
        "url": f"/api/v1/files/{file_id}/download",
    }


@router.get("/{file_id}/download")
async def download_file(
    file_id: str,
    current_user: User = Depends(get_current_user),
):
    ensure_bucket()
    objects = list(minio_client.list_objects(settings.MINIO_BUCKET, prefix=file_id))
    if not objects:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="File not found")
    obj = objects[0]
    response = minio_client.get_object(settings.MINIO_BUCKET, obj.object_name)
    return StreamingResponse(
        io.BytesIO(response.read()),
        media_type=obj.content_type or "application/octet-stream",
        headers={"Content-Disposition": f'attachment; filename="{obj.object_name}"'},
    )


@router.get("/{file_id}")
async def get_file_info(
    file_id: str,
    current_user: User = Depends(get_current_user),
):
    ensure_bucket()
    objects = list(minio_client.list_objects(settings.MINIO_BUCKET, prefix=file_id))
    if not objects:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="File not found")
    obj = objects[0]
    return {
        "id": file_id,
        "object_name": obj.object_name,
        "size": obj.size,
        "content_type": obj.content_type,
        "last_modified": obj.last_modified.isoformat() if obj.last_modified else None,
    }
