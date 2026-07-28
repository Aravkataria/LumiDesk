import asyncio
import threading
import time

import requests
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from dotenv import load_dotenv
import os

from media_service import media

API_VERSION = 2

# ----------------------------
# Weather
# ----------------------------
load_dotenv()
WEATHER_LAT = os.getenv("LAT")
WEATHER_LON = os.getenv("LON")

WEATHER_CACHE_SECONDS = 300   # matches the 5-min refresh in WeatherManager

_weather_cache = {
    "temperature": 0.0,
    "feelsLike": 0.0,
    "humidity": 0,
    "condition": "--",
    "icon": "",
    "valid": False,
}
_weather_cached_at = 0.0

# WMO weather codes -> short label for the OLED
_WEATHER_CODES = {
    0: "Clear", 1: "Mostly Clear", 2: "Partly Cloudy", 3: "Cloudy",
    45: "Fog", 48: "Fog",
    51: "Drizzle", 53: "Drizzle", 55: "Drizzle",
    61: "Light Rain", 63: "Rain", 65: "Heavy Rain",
    71: "Light Snow", 73: "Snow", 75: "Heavy Snow",
    80: "Rain Showers", 81: "Rain Showers", 82: "Heavy Showers",
    95: "Thunderstorm", 96: "Thunderstorm", 99: "Thunderstorm",
}


def _fetch_weather():

    response = requests.get(
        "https://api.open-meteo.com/v1/forecast",
        params={
            "latitude": WEATHER_LAT,
            "longitude": WEATHER_LON,
            "current": "temperature_2m,apparent_temperature,relative_humidity_2m,weather_code",
        },
        timeout=5,
    )

    response.raise_for_status()

    current = response.json()["current"]
    code = current.get("weather_code", -1)

    return {
        "temperature": current.get("temperature_2m", 0.0),
        "feelsLike": current.get("apparent_temperature", 0.0),
        "humidity": current.get("relative_humidity_2m", 0),
        "condition": _WEATHER_CODES.get(code, "--"),
        "icon": str(code),
        "valid": True,
    }

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


@app.get("/weather")
def weather():

    global _weather_cache, _weather_cached_at

    now = time.time()

    if now - _weather_cached_at < WEATHER_CACHE_SECONDS and _weather_cache["valid"]:
        return _weather_cache

    try:
        _weather_cache = _fetch_weather()
        _weather_cached_at = now
    except Exception:
        # Keep serving the last known-good reading rather than
        # sending the ESP a blank/invalid payload on a transient error.
        pass

    return _weather_cache


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