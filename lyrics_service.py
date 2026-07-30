import re
import time
from unidecode import unidecode
import pykakasi
from korean_romanizer.romanizer import Romanizer
from pypinyin import lazy_pinyin

from cache import LyricsCache
from providers import LRCLibProvider, NetEaseProvider, MusixmatchProvider

_HANGUL = re.compile(r"[\uac00-\ud7a3\u1100-\u11ff]")
_KANA = re.compile(r"[\u3040-\u30ff]")
_HAN = re.compile(r"[\u4e00-\u9fff]")
_NON_ASCII = re.compile(r"[^\x00-\x7f]")
_kakasi = pykakasi.kakasi()

class LyricsEngine:
    RETRY_COOLDOWN_SECONDS = 5

    def __init__(self):
        self.cached_song = ""
        self.parsed_lyrics = []
        self.has_lyrics = False
        
        self.resolved = False
        self.last_attempt = 0

        # Initialize modular components
        self.cache = LyricsCache()
        self.providers = [
            LRCLibProvider(),
            NetEaseProvider(),
            MusixmatchProvider()
        ]

    def romanize(self, text):
        if not text or not _NON_ASCII.search(text): return text
        try:
            if _HANGUL.search(text): return Romanizer(text).romanize()
            if _KANA.search(text):
                return " ".join([item["hepburn"].strip() for item in _kakasi.convert(text) if item["hepburn"].strip()])
            if _HAN.search(text): return " ".join(lazy_pinyin(text))
        except Exception as e:
            print("Romanize error:", e)
        return unidecode(text)

    def clean_text(self, text):
        if not text: return ""
        text = "".join(c for c in text.strip() if 32 <= ord(c) <= 126)
        return re.sub(r"\s+", " ", text).strip()

    def _time_to_ms(self, stamp):
        m = re.match(r"\[(\d+):(\d+)\.(\d+)\]", stamp)
        if not m: return None
        return int(m.group(1)) * 60000 + int(m.group(2)) * 1000 + int(m.group(3)) * 10

    def fetch(self, title, artist, album="", duration=0):
        key = f"{artist}::{title}"
        now = time.time()

        if key == self.cached_song and self.resolved:
            return
        if key == self.cached_song and (now - self.last_attempt) < self.RETRY_COOLDOWN_SECONDS:
            return

        self.cached_song = key
        self.parsed_lyrics = []
        self.has_lyrics = False
        self.resolved = False
        self.last_attempt = now

        print(f"\n[LyricsEngine] Resolving: {artist} - {title}")

        # 1. Check Local Cache
        raw_synced = self.cache.get(artist, title)
        
        # 2. Check Providers sequentially if not in cache
        if not raw_synced:
            for provider in self.providers:
                raw_synced = provider.fetch(title, artist, album, duration)
                if raw_synced:
                    self.cache.set(artist, title, raw_synced)
                    break

        if not raw_synced:
            print("[LyricsEngine] Lyrics not found anywhere.")
            self.resolved = True
            return

        # 3. Parse and format the raw lyrics
        last_time = -1
        for line in raw_synced.splitlines():
            line = line.strip()
            if not line: continue
            
            m = re.match(r"(\[\d+:\d+\.\d+\])(.*)", line)
            if not m: continue
            
            timestamp = self._time_to_ms(m.group(1))
            if timestamp is None or timestamp == last_time: continue
            last_time = timestamp
            
            lyric = self.clean_text(self.romanize(m.group(2).strip()))
            if lyric:
                self.parsed_lyrics.append({"time": timestamp, "text": lyric})

        self.has_lyrics = len(self.parsed_lyrics) > 0
        self.resolved = True
        print(f"[LyricsEngine] Ready with {len(self.parsed_lyrics)} lines.")

    def current(self, progress):
        if not self.has_lyrics:
            return ("No synced lyrics", "", False)

        current, nxt = "", ""
        for i, line in enumerate(self.parsed_lyrics):
            if progress < line["time"]: break
            current = line["text"]
            if i + 1 < len(self.parsed_lyrics):
                nxt = self.parsed_lyrics[i + 1]["text"]

        return (current[:28], nxt[:28], True)

# Export the singleton
lyrics = LyricsEngine()