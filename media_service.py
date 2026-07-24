import asyncio

from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)

from lyrics_service import lyrics


class MediaService:

    def __init__(self):

        self.last_song = ""

        self.song = {
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

    async def update(self):

        try:

            sessions = await MediaManager.request_async()

            session = sessions.get_current_session()

            if session is None:
                self.song["playing"] = False
                return

            info = await session.try_get_media_properties_async()

            playback = session.get_playback_info()

            timeline = session.get_timeline_properties()

            title = info.title
            artist = info.artist
            album = info.album_title

            progress = int(timeline.position.seconds * 1000)
            duration = int(timeline.end_time.seconds * 1000)

            playing = playback.playback_status.name == "PLAYING"

            current_key = f"{artist}::{title}"

            # New song?
            if current_key != self.last_song:

                self.last_song = current_key

                print(f"Fetching lyrics: {artist} - {title}")

                lyrics.fetch(title, artist)

            current, nxt, has = lyrics.current(progress)

            self.song = {

                "title": title,
                "artist": artist,
                "album": album,

                "duration": duration,
                "progress": progress,

                "playing": playing,

                "current_lyric": current,
                "next_lyric": nxt,
                "has_lyrics": has
            }

        except Exception as e:

            print(e)

    async def loop(self):

        while True:

            await self.update()

            await asyncio.sleep(0.5)


media = MediaService()