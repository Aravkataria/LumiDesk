#include "DisplayManager.h"
#include "NotificationManager.h"
#include "ScreenManager.h"
#include "SpotifyClient.h"

DisplayManager display;
NotificationManager notifications;
ScreenManager screen;
SpotifyClient spotify;

SongInfo song;

unsigned long lastFrame = 0;
const uint16_t FRAME_TIME = 16;      // ~60 FPS

// WiFi
const char* WIFI_SSID = "WIFI_ID";
const char* WIFI_PASSWORD = "WIFI_PASS";

// CHANGE THIS
const char* SERVER_URL = "http://192.168.x.x:8000/spotify";

void setup()
{
    Serial.begin(115200);

    display.begin();

    spotify.begin(
        WIFI_SSID,
        WIFI_PASSWORD,
        SERVER_URL
    );

    song.title = "Connecting...";
    song.artist = "";
    song.album = "";

    song.currentLyric = "";
    song.nextLyric = "";

    song.duration = 100;
    song.progress = 0;

    song.playing = false;
    song.hasLyrics = false;

    song.animatedProgress = 0;

    screen.setSong(song);

    // ALWAYS PLAYER
    screen.setScreen(ScreenType::PLAYER);

    notifications.show(
        "Spotify OLED",
        "Starting..."
    );
}

void loop()
{
    spotify.update();

    if (spotify.isConnected())
    {
        SongInfo newSong = spotify.getSong();

        if (newSong.title != song.title)
        {
            notifications.show(
                "Now Playing",
                newSong.title
            );
        }

        float target = 0;

        if (newSong.duration > 0)
        {
            target =
                (float)newSong.progress /
                (float)newSong.duration;
        }

        // smoother animation
        song.animatedProgress +=
            (target - song.animatedProgress) * 0.08f;

        song.title = newSong.title;
        song.artist = newSong.artist;
        song.album = newSong.album;

        song.progress = newSong.progress;
        song.duration = newSong.duration;

        song.playing = newSong.playing;

        song.currentLyric = newSong.currentLyric;
        song.nextLyric = newSong.nextLyric;
        song.hasLyrics = newSong.hasLyrics;
    }
    else
    {
        song.title = "Connecting...";
        song.artist = "Waiting for backend...";
        song.album = "";

        song.currentLyric = "";
        song.nextLyric = "";

        song.progress = 0;
        song.duration = 100;

        song.playing = false;
        song.hasLyrics = false;

        song.animatedProgress *= 0.9f;
    }

    notifications.update();

    screen.setSong(song);

    screen.update();

    // NEVER SWITCH SCREENS
    screen.setScreen(ScreenType::PLAYER);

    if (millis() - lastFrame >= FRAME_TIME)
    {
        lastFrame = millis();

        display.beginFrame();

        screen.draw(display);

        notifications.draw(display);

        display.endFrame();
    }
}