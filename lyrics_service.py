import re
import requests
from unidecode import unidecode


class LyricsService:

    def __init__(self):

        self.cached_song = ""
        self.cached_lyrics = []
        self.has_lyrics = False

    # -------------------------
    # Convert Unicode -> ASCII
    # -------------------------

    def clean_text(self, text):

        if not text:
            return ""

        text = text.strip()

        # Romanize
        text = unidecode(text)

        # Remove unsupported OLED chars
        text = "".join(
            c for c in text
            if 32 <= ord(c) <= 126
        )

        text = re.sub(r"\s+", " ", text)

        text = text.strip()

        if not text:
            return ""

        return text

    # -------------------------

    def _time_to_ms(self, stamp):

        m = re.match(r"\[(\d+):(\d+)\.(\d+)\]", stamp)

        if not m:
            return None

        minute = int(m.group(1))
        second = int(m.group(2))
        hundredth = int(m.group(3))

        return (
            minute * 60000 +
            second * 1000 +
            hundredth * 10
        )

    # -------------------------

    def fetch(self, title, artist):

        key = f"{artist}::{title}"

        if key == self.cached_song:
            return

        self.cached_song = key
        self.cached_lyrics = []
        self.has_lyrics = False

        print(f"Loading lyrics: {artist} - {title}")

        try:

            response = requests.get(
                "https://lrclib.net/api/get",
                params={
                    "track_name": title,
                    "artist_name": artist
                },
                timeout=5
            )

            if response.status_code != 200:
                print("Lyrics not found.")
                return

            data = response.json()

            synced = data.get("syncedLyrics", "")

            if not synced:
                print("No synced lyrics.")
                return

            last_time = -1

            for line in synced.splitlines():

                line = line.strip()

                if not line:
                    continue

                m = re.match(
                    r"(\[\d+:\d+\.\d+\])(.*)",
                    line
                )

                if not m:
                    continue

                timestamp = self._time_to_ms(m.group(1))

                if timestamp is None:
                    continue

                if timestamp == last_time:
                    continue

                last_time = timestamp

                lyric = self.clean_text(
                    m.group(2)
                )

                # Ignore empty lines
                if lyric == "":
                    continue

                self.cached_lyrics.append(
                    {
                        "time": timestamp,
                        "text": lyric
                    }
                )

            self.has_lyrics = len(self.cached_lyrics) > 0

            print(
                f"Loaded {len(self.cached_lyrics)} lyric lines."
            )

        except Exception as e:

            print("Lyrics Error:", e)

    # -------------------------

    def current(self, progress):

        if not self.has_lyrics:
            return (
                "No synced lyrics",
                "",
                False
            )

        current = ""
        nxt = ""

        for i, line in enumerate(self.cached_lyrics):

            if progress < line["time"]:
                break

            current = line["text"]

            if i + 1 < len(self.cached_lyrics):
                nxt = self.cached_lyrics[i + 1]["text"]

        # Limit OLED width
        current = current[:28]
        nxt = nxt[:28]

        return (
            current,
            nxt,
            True
        )


lyrics = LyricsService()