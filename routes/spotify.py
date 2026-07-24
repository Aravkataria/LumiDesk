from fastapi import APIRouter

from services.media_service import media

router = APIRouter(
    prefix="/api",
    tags=["Spotify"]
)


@router.get("/spotify")
def spotify():

    return media.get_song()