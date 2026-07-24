import requests
import re


class LyricsService:
    def __init__(self):
        self.cached_song = None
        self.cached_lyrics = []
        self.has_lyrics = False

    def _time_to_ms(self, text):
        """
        Converts [mm:ss.xx] -> milliseconds
        """
        m = re.match(r"\[(\d+):(\d+)\.(\d+)\]", text)

        if not m:
            return 0

        minute = int(m.group(1))
        second = int(m.group(2))
        hundredths = int(m.group(3))

        return (
            minute * 60000 +
            second * 1000 +
            hundredths * 10
        )

    def fetch(self, title, artist):

        key = f"{artist}::{title}"

        if key == self.cached_song:
            return

        self.cached_song = key
        self.cached_lyrics = []
        self.has_lyrics = False

        try:

            url = "https://lrclib.net/api/get"

            params = {
                "track_name": title,
                "artist_name": artist
            }

            r = requests.get(
                url,
                params=params,
                timeout=5
            )

            if r.status_code != 200:
                return

            data = r.json()

            synced = data.get("syncedLyrics")

            if not synced:
                return

            for line in synced.splitlines():

                if "]" not in line:
                    continue

                t = line[:line.index("]") + 1]
                lyric = line[line.index("]") + 1:]

                self.cached_lyrics.append({
                    "time": self._time_to_ms(t),
                    "text": lyric
                })

            self.has_lyrics = len(self.cached_lyrics) > 0

        except Exception as e:
            print("Lyrics:", e)

    def current(self, progress):

        if not self.has_lyrics:
            return "", "", False

        current = ""
        nxt = ""

        for i, line in enumerate(self.cached_lyrics):

            if progress >= line["time"]:

                current = line["text"]

                if i + 1 < len(self.cached_lyrics):
                    nxt = self.cached_lyrics[i + 1]["text"]

        return current, nxt, True


lyrics = LyricsService()