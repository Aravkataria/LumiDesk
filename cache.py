import json
import os

class LyricsCache:
    def __init__(self, filepath="lyrics_cache.json"):
        # filepath is kept for compatibility with existing callers,
        # but is no longer used - caching is in-memory only, so
        # nothing gets written to disk.
        self.filepath = filepath
        self.cache = {}

    def get(self, artist, title):
        key = f"{artist}::{title}".lower()
        return self.cache.get(key)

    def set(self, artist, title, synced_lyrics):
        key = f"{artist}::{title}".lower()
        self.cache[key] = synced_lyrics
        print(f"[Cache] Cached lyrics for {artist} - {title} (in memory)")