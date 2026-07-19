import uuid
import io
from minio import Minio
from app.config import settings


def get_minio_client() -> Minio:
    return Minio(
        settings.MINIO_ENDPOINT,
        access_key=settings.MINIO_ACCESS_KEY,
        secret_key=settings.MINIO_SECRET_KEY,
        secure=False,
    )


def ensure_bucket(client: Minio):
    if not client.bucket_exists(settings.MINIO_BUCKET):
        client.make_bucket(settings.MINIO_BUCKET)


async def upload_file(file_content: bytes, filename: str, content_type: str = "application/octet-stream") -> dict:
    client = get_minio_client()
    ensure_bucket(client)
    file_id = str(uuid.uuid4())
    ext = filename.rsplit(".", 1)[-1] if "." in filename else "bin"
    object_name = f"{file_id}.{ext}"
    client.put_object(
        settings.MINIO_BUCKET,
        object_name,
        io.BytesIO(file_content),
        length=len(file_content),
        content_type=content_type,
    )
    return {
        "id": file_id,
        "object_name": object_name,
        "filename": filename,
        "size": len(file_content),
        "content_type": content_type,
    }


def download_file(file_id: str) -> bytes | None:
    client = get_minio_client()
    ensure_bucket(client)
    objects = list(client.list_objects(settings.MINIO_BUCKET, prefix=file_id))
    if not objects:
        return None
    response = client.get_object(settings.MINIO_BUCKET, objects[0].object_name)
    return response.read()
