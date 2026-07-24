using Dubya.WindowsMediaController;

Console.WriteLine("Spotify Media Helper");
Console.WriteLine("--------------------");

var mediaManager = new MediaManager();

await mediaManager.StartAsync();

Console.WriteLine("Waiting for media session...\n");

while (true)
{
    try
    {
        var session = mediaManager.CurrentSession;

        if (session != null)
        {
            var media = await session.GetMediaPropertiesAsync();

            Console.Clear();

            Console.WriteLine("Title   : " + media.Title);
            Console.WriteLine("Artist  : " + media.Artist);
            Console.WriteLine("Album   : " + media.AlbumTitle);

            var playback = session.ControlSession.GetPlaybackInfo();

            Console.WriteLine("Playing : " +
                (playback.PlaybackStatus.ToString()));

            Console.WriteLine();

            Console.WriteLine("Press Ctrl+C to exit.");
        }
        else
        {
            Console.Clear();
            Console.WriteLine("No active media session...");
        }
    }
    catch (Exception ex)
    {
        Console.WriteLine(ex.Message);
    }

    await Task.Delay(1000);
}