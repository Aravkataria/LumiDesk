import asyncio
import threading

from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager
)


class MediaService:


    def __init__(self):

        self.data = {

            "title": "",
            "artist": "",
            "album": "",

            "playing": False,

            "position": 0,
            "duration": 0

        }


        self.loop = asyncio.new_event_loop()


        thread = threading.Thread(
            target=self.start_loop,
            daemon=True
        )

        thread.start()



    def start_loop(self):

        asyncio.set_event_loop(
            self.loop
        )

        self.loop.run_until_complete(
            self.update_loop()
        )



    async def update_loop(self):

        while True:

            try:

                manager = await (
                    GlobalSystemMediaTransportControlsSessionManager
                    .request_async()
                )


                session = (
                    manager
                    .get_current_session()
                )


                if session:


                    info = await (
                        session
                        .try_get_media_properties_async()
                    )


                    self.data["title"] = (
                        info.title
                    )


                    self.data["artist"] = (
                        info.artist
                    )


                    self.data["album"] = (
                        info.album_title
                    )


                    playback = (
                        session
                        .get_playback_info()
                    )


                    self.data["playing"] = (
                        playback
                        .playback_status
                        == 4
                    )


            except Exception as e:

                print(
                    "Media Error:",
                    e
                )


            await asyncio.sleep(1)



    def get_song(self):

        return self.data



media = MediaService()