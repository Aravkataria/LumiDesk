# esp-Spotify-Player

## Now Playing OLED Display

A small ESP32 + OLED display that mirrors whatever's currently playing on your PC — title, artist, album, a live animated progress bar, and time-synced lyrics — updated in near real time over your local network.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Motivation](#motivation)
3. [Workflow Overview](#workflow-overview)
4. [Tech Stack](#tech-stack)
5. [Key Features](#key-features)
6. [Architecture & Methods](#architecture--methods)
   - [Firmware Architecture](#firmware-architecture)
   - [Backend Architecture](#backend-architecture)
   - [Communication Protocol](#communication-protocol)
   - [Lyrics Sync](#lyrics-sync)
7. [Repo Structure](#repo-structure)
8. [Usage / Running the Project](#usage--running-the-project)
   - [Backend Setup](#backend-setup)
   - [Firmware Setup](#firmware-setup)
   - [Running It](#running-it)
9. [Features Explained](#features-explained)
   - [Now Playing Display](#now-playing-display)
   - [Animated Progress Bar](#animated-progress-bar)
   - [Marquee Scrolling](#marquee-scrolling)
   - [Track-Change Notifications](#track-change-notifications)
   - [Synced Lyrics](#synced-lyrics)
10. [Future Enhancements](#future-enhancements)
11. [Key Learnings](#key-learnings)
12. [Installation Instructions](#installation-instructions)
13. [Limitations](#limitations)

---

## Project Overview

This project turns a cheap ESP32 + OLED into a dedicated "now playing" display for your PC. Instead of a phone lock screen or a browser tab, you get a small always-on screen showing exactly what's playing, how far into the track you are, and the current lyric line — no app switching required.

The interesting part is *how* it gets the track info: rather than authenticating against the Spotify Web API (OAuth flow, client IDs, token refresh, rate limits), it reads directly from Windows' built-in media session API. That means it works for Spotify, YouTube, browser tabs, or literally anything else that reports playback to Windows — with zero API keys involved.

## Motivation

- Phone/browser now-playing UIs are one more thing to glance at and unlock. A dedicated screen next to the keyboard is always visible, no interaction needed.
- Going through Spotify's official API means OAuth setup, token refresh handling, and being limited to Spotify specifically. Reading the OS media session sidesteps all of that and works with any player.
- It's also a good excuse to build something with a proper separation of concerns on the embedded side — instead of one big `.ino` file, the firmware is split into single-responsibility manager classes, which made it much easier to add lyrics and animations later without breaking the display logic.

## Workflow Overview

The system is two independent programs talking over local HTTP:

1. **Capture stage:** the Python backend polls the Windows `GlobalSystemMediaTransportControlsSessionManager` every 500ms for the current track, position, and playback state.
2. **Enrichment stage:** when the track changes, the backend fetches time-synced lyrics for it from LRCLIB and caches them.
3. **Serve stage:** FastAPI exposes the current song + current lyric line as JSON on `/spotify`.
4. **Display stage:** the ESP32 polls that endpoint, smooths the progress bar toward the real value, and hands the song state to the screen/animation/marquee/notification managers to render at ~30fps.

```
Windows Media Session → media_service.py → lyrics_service.py (LRCLIB)
                                 │
                                 ▼
                         FastAPI (/spotify)
                                 │
                          Wi-Fi (local network)
                                 │
                                 ▼
                    ESP32 (SpotifyClient → ScreenManager)
                                 │
                                 ▼
                        SH1106 OLED (128x64)
```

## Tech Stack

- **Firmware:** C++ (Arduino framework), ESP32
- **Backend:** Python 3, FastAPI, uvicorn
- **Media capture:** `winsdk` (Windows Runtime bindings for Python) — reads the OS-level "now playing" session
- **Lyrics:** [LRCLIB](https://lrclib.net) public API (synced lyrics, no auth required)
- **Display:** SH1106 OLED, 128x64, I2C
- **Secondary tooling:** a small standalone C# console app (`spotify-media-helper/`) used to sanity-check the Windows media session independent of the Python backend

## Key Features

- Real-time track info (title, artist, album) pulled from whatever's actually playing on the PC — not limited to Spotify.
- Smoothly animated progress bar that eases toward the real playback position rather than snapping.
- Marquee scrolling for titles/artists too long to fit on a 128px-wide screen.
- Pop-up notification banner whenever the track changes.
- Time-synced lyrics, advancing in step with playback position.
- No API keys, OAuth setup, or cloud accounts required — everything runs on the local network.

## Architecture & Methods

### Firmware Architecture

The `.ino` file is intentionally thin — it just wires together a set of manager classes, each responsible for one part of the display:

- **`SpotifyClient`** — connects to Wi-Fi, polls the backend's `/spotify` endpoint over plain HTTP, and parses the JSON response into a `SongInfo` struct.
- **`DisplayManager`** — low-level wrapper around the SH1106 driver; handles frame begin/end.
- **`ScreenManager`** — owns the current `SongInfo` state and decides what gets drawn each frame (player screen, connecting screen, etc.).
- **`AnimationManager`** — handles the eased progress bar and any transition animations.
- **`MarqueeManager`** — scrolls text horizontally when it's wider than the screen.
- **`NotificationManager`** — draws a temporary overlay banner (e.g. "Now Playing: <title>") that fades out after a track change.

The main loop runs at a fixed ~30fps (`FRAME_TIME = 33ms`), polling the backend independently of the render loop so the display stays smooth even if a network request is slow.

### Backend Architecture

- **`app.py`** — the FastAPI entrypoint. Starts a background thread running the media polling loop and exposes:
  - `GET /` — basic status
  - `GET /spotify` — current song JSON (title, artist, album, progress, duration, playing, current/next lyric line)
  - `GET /health` — lightweight health check
- **`media_service.py`** — the core polling loop. On every tick it asks Windows for the current media session, reads title/artist/album/position/duration/playback state, and — if the track changed — triggers a lyrics fetch.
- **`lyrics_service.py`** — queries LRCLIB for synced lyrics (`[mm:ss.xx]` formatted), parses them into a timestamp-indexed list, and exposes `current(progress_ms)` to look up the active + next line for a given playback position.
- **`models.py`** — the `SongInfo` dataclass shape used internally.
- **`routes/spotify.py`** — an alternate/legacy router-style endpoint for the same data, kept alongside the main `app.py` routes.

### Communication Protocol

Communication between the ESP32 and backend is deliberately simple: plain HTTP GET, no auth, JSON body. The ESP32 does its own minimal HTTP parsing (splitting host/port/path out of the configured server URL) rather than pulling in a heavier HTTP client library, to keep flash usage down.

### Lyrics Sync

Lyrics come from LRCLIB's public `/api/get` endpoint, matched by track + artist name — no API key required. The returned `syncedLyrics` block is parsed line-by-line into `{ time_ms, text }` pairs. On every backend tick, `lyrics_service.current(progress_ms)` walks that list to find the most recent line at-or-before the current playback position, plus whatever comes next — which is what gets serialized into `current_lyric` / `next_lyric` for the ESP32 to display.

## Repo Structure

```
esp-Spotify-Player/
├── main.ino                  # Firmware entrypoint, wires up all managers
├── SpotifyClient.h/.cpp       # Polls backend, parses song JSON
├── DisplayManager.h/.cpp      # OLED driver wrapper
├── ScreenManager.h/.cpp       # Decides what's on screen
├── AnimationManager.h/.cpp    # Progress bar easing / transitions
├── MarqueeManager.h/.cpp      # Scrolling text for long titles
├── NotificationManager.h/.cpp # Track-change popup banner
├── UIModels.h                 # SongInfo struct shared across firmware
├── app.py                     # FastAPI entrypoint
├── media_service.py           # Windows media session polling loop
├── lyrics_service.py          # LRCLIB fetch + lyric-line lookup
├── models.py                  # SongInfo dataclass (backend side)
├── test_media.py              # Standalone script to sanity-check media capture
├── routes/
│   └── spotify.py             # Alternate router-based endpoint
├── spotify-media-helper/       # Standalone C# console app for testing media capture
└── requirements.txt
```

## Usage / Running the Project

### Backend Setup

```bash
pip install -r requirements.txt
python app.py
```

This starts the FastAPI server on port 8000. Find your PC's local IP address (`ipconfig` on Windows) — you'll need it for the firmware config.

### Firmware Setup

1. Open `main.ino` in the Arduino IDE.
2. Set your Wi-Fi credentials:
   ```cpp
   const char* WIFI_SSID = "your_wifi_name";
   const char* WIFI_PASSWORD = "your_wifi_password";
   ```
3. Point `SERVER_URL` at your PC's local IP:
   ```cpp
   const char* SERVER_URL = "http://192.168.1.34:8000/spotify";
   ```
4. Flash to the ESP32.

### Running It

With the backend running and the ESP32 flashed and connected to Wi-Fi, the display should show "Connecting..." briefly, then start showing whatever's actively playing on the PC — updating within about half a second of any change.

## Features Explained

### Now Playing Display

The core screen shows title, artist, and album, refreshed every backend poll cycle (500ms). This is driven entirely by `ScreenManager`, which holds the latest `SongInfo` and re-renders it every frame.

### Animated Progress Bar

Rather than snapping straight to the real playback position (which would look jittery given the 500ms poll interval), `AnimationManager` eases the displayed progress toward the target value each frame:
```cpp
song.animatedProgress += (targetProgress - song.animatedProgress) * 0.15f;
```
This gives a smooth, continuously-moving bar even though the underlying data only updates twice a second.

### Marquee Scrolling

Long titles or artist names that don't fit the 128px-wide screen are scrolled horizontally by `MarqueeManager` rather than truncated, so nothing gets cut off.

### Track-Change Notifications

Whenever `SpotifyClient` detects the title has changed since the last poll, `NotificationManager` draws a temporary "Now Playing" banner over the current screen, which fades after a few seconds.

### Synced Lyrics

When lyrics are available for the current track, the current and next line are shown alongside the track info, advancing automatically as `lyrics_service.current()` reruns against the live playback position on the backend.

## Future Enhancements

- Cross-platform media capture (macOS/Linux equivalents to `winsdk`), since the backend is currently Windows-only.
- Album art rendering — dithered to 1-bit for the OLED.
- Multiple display "pages" (e.g. a dedicated lyrics-only view, a queue view).
- WebSocket push from the backend instead of polling, to cut latency below the current ~500ms.
- Optional direct Spotify Web API mode as a fallback for setups without a Windows PC in the loop.

## Key Learnings

- Reading the OS-level media session is a much lower-friction way to get "now playing" data than integrating a music service's official API — at the cost of being tied to that OS.
- Splitting embedded firmware into single-responsibility manager classes (display, screen, animation, marquee, notifications) made it far easier to extend than one monolithic loop.
- Easing/interpolating a low-frequency network value (progress polled at 500ms) into a smooth 30fps animation is a small trick that makes a big visual difference.
- Keeping the ESP32's HTTP handling manual and minimal (rather than pulling in a full HTTP client library) keeps flash usage low on a memory-constrained device.

## Installation Instructions

```bash
# 1. Clone the repository
git clone https://github.com/Aravkataria/esp-Spotify-Player.git
cd esp-Spotify-Player

# 2. Set up the backend environment
python -m venv venv
venv\Scripts\activate        # Windows
pip install -r requirements.txt

# 3. Run the backend
uvicorn app:app --host 0.0.0.0 --port 8000 --reload

# 4. Flash main.ino to the ESP32 via the Arduino IDE
#    (set WIFI_SSID, WIFI_PASSWORD, and SERVER_URL first)
```

## Limitations

- Windows-only for now — media capture relies on `winsdk`, which is a Windows Runtime binding.
- Backend and ESP32 must be on the same local network; there's no remote/cloud relay.
- Lyrics availability depends entirely on LRCLIB's database — not every track will have synced lyrics.
- No authentication on the `/spotify` endpoint — fine for a home network, not meant to be exposed publicly.
