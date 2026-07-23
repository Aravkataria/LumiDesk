from dataclasses import dataclass


@dataclass
class SongInfo:

    title: str = ""

    artist: str = ""

    album: str = ""

    duration: int = 0

    progress: int = 0

    playing: bool = False

    device: str = ""

    shuffle: bool = False

    repeat: str = "off"

    album_art: str = ""

    song_id: str = ""