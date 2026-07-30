import asyncio
import re
import time
import unicodedata

from unidecode import unidecode
from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)

from lyrics_service import lyrics


class MediaService:

    # Browsers commonly drop their media session for a moment right
    # after pausing, even though the tab/video is still there. Give
    # it this long before treating it as truly "nothing playing" -
    # otherwise the idle screen jumps in instantly on every pause
    # instead of going through the normal pause-timeout behavior.
    SESSION_GRACE_SECONDS = 30

    def __init__(self):

        self.last_song = ""
        self.last_source = "Media"
        self.last_active_at = 0

        self.song = {
            "api_version": 2,

            "title": "",
            "artist": "",
            "album": "",
            "source": "Spotify",

            "duration": 0,
            "progress": 0,

            "playing": False,
            "active": False,

            "current_lyric": "",
            "next_lyric": "",
            "has_lyrics": False
        }

    # ------------------------------------------------
    # Figure out what's actually playing the media -
    # Spotify, YouTube (in a browser), or some other
    # browser/app we can't identify further.
    # ------------------------------------------------

    def resolve_source(self, source_app_id):

        app_id = (source_app_id or "").lower()

        if "spotify" in app_id:
            return "Spotify"

        browser_names = {
            "chrome": "Chrome",
            "msedge": "Edge",
            "firefox": "Firefox",
            "brave": "Brave",
            "opera": "Opera",
        }

        for key, name in browser_names.items():
            if key in app_id:
                return name

        return "Media"

    # ------------------------------------------------

    def clean_text(self, text, fallback=""):

        if text is None:
            return fallback

        text = str(text).strip()

        if not text:
            return fallback

        text = unicodedata.normalize("NFKC", text)

        text = unidecode(text)

        text = re.sub(r"[^\x20-\x7E]", "", text)

        text = re.sub(r"\s+", " ", text)

        text = text.strip()

        return text if text else fallback

    # ------------------------------------------------

    async def update(self):

        try:

            manager = await MediaManager.request_async()

            session = manager.get_current_session()

            if session is None:

                if time.time() - self.last_active_at < self.SESSION_GRACE_SECONDS:
                    # Likely just a brief drop (e.g. paused in a
                    # browser) - keep showing the last known state
                    # so the normal pause-timeout can still run.
                    return

                self.song["playing"] = False
                self.song["active"] = False
                self.song["title"] = "Nothing Playing"
                self.song["artist"] = ""
                self.song["album"] = ""
                self.song["source"] = "Media"
                self.song["progress"] = 0
                self.song["duration"] = 0
                self.song["current_lyric"] = ""
                self.song["next_lyric"] = ""
                self.song["has_lyrics"] = False

                return

            self.last_active_at = time.time()

            info = await session.try_get_media_properties_async()

            playback = session.get_playback_info()

            timeline = session.get_timeline_properties()

            title = self.clean_text(
                info.title,
                "Unknown Title"
            )

            album = self.clean_text(
                info.album_title,
                ""
            )

            try:
                source_app_id = session.source_app_user_model_id
                source = self.resolve_source(source_app_id)
                self.last_source = source
            except Exception:
                # A single failed read shouldn't wipe cached lyrics
                # or misdetect Spotify as something else - just keep
                # whatever we last resolved successfully.
                source = self.last_source

            is_spotify = source == "Spotify"

            # Show whatever creator name the site/app actually
            # reports (e.g. a YouTube channel) - just don't fall
            # back to a placeholder when there isn't one.
            artist = self.clean_text(
                info.artist,
                ""
            )

            progress = int(
                timeline.position.seconds * 1000
            )

            duration = max(
                1,
                int(timeline.end_time.seconds * 1000)
            )

            playing = (
                playback.playback_status.name == "PLAYING"
            )

            # Synced lyrics only exist for actual songs on
            # Spotify - LRCLIB has nothing for a YouTube video
            # or a random site, so don't even try there.
            if is_spotify:

                current_key = f"{artist}::{title}"

                if current_key != self.last_song:

                    self.last_song = current_key

                    print(
                        f"Loading lyrics: {artist} - {title}"
                    )

                    lyrics.fetch(
                        title,
                        artist
                    )

                current, nxt, has = lyrics.current(progress)

                current = self.clean_text(current)
                nxt = self.clean_text(nxt)

            else:

                # Reset so a future Spotify track re-triggers a fetch.
                self.last_song = ""
                current, nxt, has = "", "", False

            self.song.update({

                "api_version": 2,

                "title": title,
                "artist": artist,
                "album": album,
                "source": source,

                "duration": duration,
                "progress": progress,

                "playing": playing,
                "active": True,

                "current_lyric": current,
                "next_lyric": nxt,
                "has_lyrics": has

            })

        except Exception as e:

            print("[MediaService]", e)

    # ------------------------------------------------

    async def loop(self):

        while True:

            await self.update()

            await asyncio.sleep(0.1)


media = MediaService()