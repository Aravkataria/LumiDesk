from pathlib import Path
import os
from dotenv import load_dotenv

BASE_DIR = Path(__file__).resolve().parent

load_dotenv(BASE_DIR / ".env")


class Config:

    # =====================
    # Spotify
    # =====================

    CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID")
    CLIENT_SECRET = os.getenv("SPOTIFY_CLIENT_SECRET")

    REDIRECT_URI = "http://127.0.0.1:8000/callback"

    SCOPES = [
        "user-read-playback-state",
        "user-read-currently-playing",
    ]

    # =====================
    # Server
    # =====================

    HOST = "0.0.0.0"
    PORT = 8000

    # =====================
    # Files
    # =====================

    TOKEN_FILE = BASE_DIR / "tokens" / "spotify_token.json"

    CACHE_FOLDER = BASE_DIR / "cache"

    TOKEN_FILE.parent.mkdir(exist_ok=True)

    CACHE_FOLDER.mkdir(exist_ok=True)