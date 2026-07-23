import time
import requests

from auth import auth
from models import SongInfo


class SpotifyService:


    API_URL = "https://api.spotify.com/v1"


    def __init__(self):

        self.current_song = SongInfo()

        self.last_update = 0

        self.cache_time = 1.0



    def headers(self):

        token = auth.get_access_token()

        return {

            "Authorization": f"Bearer {token}"

        }



    def get_song(self):


        now = time.time()


        if now - self.last_update < self.cache_time:

            return self.current_song



        self.last_update = now



        token = auth.get_access_token()


        if token is None:

            print("Spotify not authenticated")

            return self.current_song



        try:


            response = requests.get(

                f"{self.API_URL}/me/player",

                headers=self.headers()

            )


        except Exception as e:

            print(
                "Spotify Request Error:",
                e
            )

            return self.current_song



        if response.status_code == 204:

            print("Spotify returned 204 - no active playback")

            self.current_song.playing = False

            return self.current_song



        if response.status_code != 200:

            print("Spotify API ERROR")
            print("Status:", response.status_code)
            print(response.text)

            return self.current_song

        playback = response.json()



        item = playback.get("item")



        if item is None:

            return self.current_song



        song = SongInfo()



        song.song_id = item["id"]



        song.title = item["name"]



        song.artist = ", ".join(

            artist["name"]

            for artist in item["artists"]

        )



        song.album = (

            item["album"]["name"]

        )



        song.duration = (

            item["duration_ms"]

        )



        song.progress = (

            playback.get(

                "progress_ms",

                0

            )

        )



        song.playing = (

            playback.get(

                "is_playing",

                False

            )

        )



        device = playback.get(
            "device"
        )


        if device:

            song.device = device.get(
                "name",
                ""
            )



        song.shuffle = playback.get(

            "shuffle_state",

            False

        )



        song.repeat = playback.get(

            "repeat_state",

            "off"

        )



        images = (

            item["album"]["images"]

        )


        if images:

            song.album_art = (

                images[0]["url"]

            )



        self.current_song = song



        return song



    def get_json(self):


        song = self.get_song()


        return {

            "title": song.title,

            "artist": song.artist,

            "album": song.album,

            "duration": song.duration,

            "progress": song.progress,

            "playing": song.playing,

            "device": song.device,

            "shuffle": song.shuffle,

            "repeat": song.repeat,

            "album_art": song.album_art,

            "song_id": song.song_id

        }



spotify = SpotifyService()