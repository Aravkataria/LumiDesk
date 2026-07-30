import json
import socket
import urllib.request
import urllib.parse
import urllib.error

# ==========================================
# NETWORK HANG FIX
# Forces Python to use IPv4.
# ==========================================
old_getaddrinfo = socket.getaddrinfo
def new_getaddrinfo(*args, **kwargs):
    responses = old_getaddrinfo(*args, **kwargs)
    return [r for r in responses if r[0] == socket.AF_INET]
socket.getaddrinfo = new_getaddrinfo
# ==========================================

class BaseProvider:
    def fetch(self, title, artist, album="", duration=0):
        raise NotImplementedError("Providers must implement the fetch method.")

class LRCLibProvider(BaseProvider):
    def __init__(self):
        self.headers = {"User-Agent": "LumiDesk/1.0"}
        self.timeout = 5

    def fetch(self, title, artist, album="", duration=0):
        print(f"[LRCLIB] Searching for {artist} - {title}")
        
        # 1. Try exact match
        params = {"track_name": title, "artist_name": artist}
        if duration: params["duration"] = duration
        
        query = urllib.parse.urlencode(params)
        req = urllib.request.Request(f"https://lrclib.net/api/get?{query}", headers=self.headers)
        
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as response:
                if response.status == 200:
                    data = json.loads(response.read().decode('utf-8'))
                    if data.get("syncedLyrics"):
                        return data.get("syncedLyrics")
        except Exception:
            pass # Fallback to search
            
        # 2. Try broad search
        query = urllib.parse.urlencode({"track_name": title, "artist_name": artist})
        req = urllib.request.Request(f"https://lrclib.net/api/search?{query}", headers=self.headers)
        
        try:
            with urllib.request.urlopen(req, timeout=self.timeout) as response:
                if response.status == 200:
                    results = json.loads(response.read().decode('utf-8'))
                    results = [r for r in results if r.get("syncedLyrics")]
                    if results:
                        return results[0].get("syncedLyrics")
        except Exception as e:
            print(f"[LRCLIB] Error: {e}")
            
        return None

class NetEaseProvider(BaseProvider):
    def fetch(self, title, artist, album="", duration=0):
        print(f"[NetEase] Searching for {artist} - {title}")
        # Add your NetEase NCM API logic here. 
        # Usually involves hitting http://music.163.com/api/search/get/ to get an ID, 
        # then hitting /api/song/lyric?id=ID
        return None

class MusixmatchProvider(BaseProvider):
    def fetch(self, title, artist, album="", duration=0):
        print(f"[Musixmatch] Searching for {artist} - {title}")
        # Add your Musixmatch macro-token API logic here.
        return None