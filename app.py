from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

import asyncio
import threading

from media_service import media


app = FastAPI(
    title="Spotify OLED Backend",
    version="3.0"
)


# Allow requests from any device on your network
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
    daemon=True
).start()


# ----------------------------
# Routes
# ----------------------------

@app.get("/")
def root():
    return {
        "status": "running",
        "service": "Spotify OLED Backend",
        "version": "3.0"
    }


@app.get("/spotify")
def spotify():
    return media.song


@app.get("/health")
def health():
    return {
        "status": "ok",
        "playing": media.song["playing"]
    }