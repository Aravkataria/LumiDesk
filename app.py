import asyncio
import threading

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from media_service import media

API_VERSION = 2

app = FastAPI(
    title="Spotify OLED Backend",
    version="4.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ----------------------------
# Background Media Thread
# ----------------------------

def media_worker():
    asyncio.run(media.loop())


threading.Thread(
    target=media_worker,
    daemon=True,
    name="MediaService"
).start()


# ----------------------------
# Routes
# ----------------------------

@app.get("/")
def root():

    return {
        "status": "running",
        "service": "Spotify OLED Backend",
        "api_version": API_VERSION
    }


@app.get("/spotify")
def spotify():

    return media.song


@app.get("/health")
def health():

    return {
        "status": "ok",
        "playing": media.song.get("playing", False),
        "song": media.song.get("title", ""),
        "artist": media.song.get("artist", ""),
        "has_lyrics": media.song.get("has_lyrics", False),
        "api_version": API_VERSION
    }