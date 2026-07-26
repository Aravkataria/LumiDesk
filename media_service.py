import asyncio
import re
import unicodedata

from unidecode import unidecode
from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)

from lyrics_service import lyrics


class MediaService:

    def __init__(self):

        self.last_song = ""

        self.song = {
            "api_version": 2,

            "title": "",
            "artist": "",
            "album": "",

            "duration": 0,
            "progress": 0,

            "playing": False,

            "current_lyric": "",
            "next_lyric": "",
            "has_lyrics": False
        }

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

                self.song["playing"] = False
                self.song["title"] = "Nothing Playing"
                self.song["artist"] = ""
                self.song["album"] = ""
                self.song["progress"] = 0
                self.song["duration"] = 0
                self.song["current_lyric"] = ""
                self.song["next_lyric"] = ""
                self.song["has_lyrics"] = False

                return

            info = await session.try_get_media_properties_async()

            playback = session.get_playback_info()

            timeline = session.get_timeline_properties()

            title = self.clean_text(
                info.title,
                "Unknown Title"
            )

            artist = self.clean_text(
                info.artist,
                "Unknown Artist"
            )

            album = self.clean_text(
                info.album_title,
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

            self.song.update({

                "api_version": 2,

                "title": title,
                "artist": artist,
                "album": album,

                "duration": duration,
                "progress": progress,

                "playing": playing,

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

            await asyncio.sleep(0.5)


media = MediaService()