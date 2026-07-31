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

    # Free synced-lyrics sources only give one timestamp per LINE,
    # never per word. To get a word-by-word "karaoke" feel without
    # real word-level timing data, we estimate each word's start time
    # by splitting the line's duration proportionally across its
    # words (longer words get a slightly bigger share). It's an
    # approximation, but it updates far more often than showing a
    # whole line for 3-4 seconds at a stretch, and it's just cheap
    # arithmetic done once when the lyrics are fetched - no extra
    # RAM or network cost per frame.
    MIN_WORD_MS = 120

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

    def _split_words(self, text, start_ms, end_ms):
        words = text.split()
        if not words:
            return []

        total = max(end_ms - start_ms, self.MIN_WORD_MS * len(words))
        weights = [len(w) + 1 for w in words]  # +1 so short words still get some time
        weight_sum = sum(weights)

        result = []
        t = start_ms
        for w, wt in zip(words, weights):
            result.append({"time": t, "text": w})
            t += max(self.MIN_WORD_MS, int(total * wt / weight_sum))

        return result

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

        # 3. Parse the raw line-level synced lyrics
        raw_lines = []
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
                raw_lines.append({"time": timestamp, "text": lyric})

        # 4. Estimate per-word timing within each line using the gap
        #    to the next line's start (or the track's end for the
        #    last line) as that line's total duration.
        duration_ms = max(duration * 1000, 0)
        self.parsed_lyrics = []

        for i, entry in enumerate(raw_lines):
            start = entry["time"]
            if i + 1 < len(raw_lines):
                end = raw_lines[i + 1]["time"]
            else:
                end = max(start + 3000, duration_ms)

            words = self._split_words(entry["text"], start, end)

            self.parsed_lyrics.append({
                "time": start,
                "text": entry["text"],
                "words": words
            })

        self.has_lyrics = len(self.parsed_lyrics) > 0
        self.resolved = True
        print(f"[LyricsEngine] Ready with {len(self.parsed_lyrics)} lines.")

    def current(self, progress):
        if not self.has_lyrics:
            return ("No synced lyrics", "", False)

        # Find the current line.
        line_idx = -1
        for i, line in enumerate(self.parsed_lyrics):
            if progress < line["time"]: break
            line_idx = i

        if line_idx == -1:
            return ("", "", True)

        line = self.parsed_lyrics[line_idx]
        words = line["words"]

        if not words:
            return (line["text"][:28], "", True)

        # Find the current word within that line.
        word_idx = 0
        for i, w in enumerate(words):
            if progress < w["time"]: break
            word_idx = i

        # Show the current word plus the next one - a small
        # karaoke-style window instead of the whole line.
        current_words = words[word_idx:word_idx + 2]
        current_text = " ".join(w["text"] for w in current_words)

        if word_idx + 2 < len(words):
            next_words = words[word_idx + 2:word_idx + 4]
            next_text = " ".join(w["text"] for w in next_words)
        elif line_idx + 1 < len(self.parsed_lyrics):
            next_line_words = self.parsed_lyrics[line_idx + 1]["words"]
            next_text = " ".join(w["text"] for w in next_line_words[:2])
        else:
            next_text = ""

        return (current_text[:28], next_text[:28], True)

# Export the singleton
lyrics = LyricsEngine()