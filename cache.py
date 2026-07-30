import json
import os

class LyricsCache:
    def __init__(self, filepath="lyrics_cache.json"):
        self.filepath = filepath
        self.cache = self._load()

    def _load(self):
        if os.path.exists(self.filepath):
            try:
                with open(self.filepath, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except Exception as e:
                print(f"[Cache] Failed to load cache: {e}")
        return {}

    def _save(self):
        try:
            with open(self.filepath, 'w', encoding='utf-8') as f:
                json.dump(self.cache, f, ensure_ascii=False, indent=4)
        except Exception as e:
            print(f"[Cache] Failed to save cache: {e}")

    def get(self, artist, title):
        key = f"{artist}::{title}".lower()
        return self.cache.get(key)

    def set(self, artist, title, synced_lyrics):
        key = f"{artist}::{title}".lower()
        self.cache[key] = synced_lyrics
        self._save()
        print(f"[Cache] Saved lyrics for {artist} - {title}")