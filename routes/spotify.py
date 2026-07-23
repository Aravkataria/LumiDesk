from fastapi import APIRouter

from spotify_service import spotify


router = APIRouter(
    prefix="/api",
    tags=["Spotify"]
)


@router.get("/spotify")
def current_song():

    return spotify.get_json()