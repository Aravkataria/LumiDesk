import asyncio

from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)


async def main():

    print("Searching for media session...")

    sessions = await MediaManager.request_async()

    session = sessions.get_current_session()

    if session is None:
        print("No media session found.")
        return

    info = await session.try_get_media_properties_async()

    playback = session.get_playback_info()

    timeline = session.get_timeline_properties()

    print("\n========== SUCCESS ==========\n")

    print("Title      :", info.title)
    print("Artist     :", info.artist)
    print("Album      :", info.album_title)

    print("")

    print("Status     :", playback.playback_status.name)

    print("")

    print("Position   :", timeline.position.seconds)
    print("Duration   :", timeline.end_time.seconds)

    print("\n=============================\n")


asyncio.run(main())