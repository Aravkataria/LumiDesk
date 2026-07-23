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
const uint16_t FRAME_TIME = 33;

// ---------------------------
// WiFi Configuration
// ---------------------------

const char* WIFI_SSID = "WIFI_NAME";
const char* WIFI_PASSWORD = "WIFI_PASS";

// Replace with your computer's IP address
const char* SERVER_URL = "http://192.168.1.34:5000/spotify";

// ---------------------------

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
    song.artist = "Starting...";
    song.duration = 100;
    song.progress = 0;
    song.animatedProgress = 0.0f;
    song.playing = false;

    screen.setSong(song);
    screen.setScreen(ScreenType::PLAYER);

    notifications.show(
        "Spotify OLED",
        "Starting..."
    );
}

void loop()
{
    unsigned long now = millis();

    // Update Spotify client
    spotify.update();

    if (spotify.isConnected())
    {
        SongInfo newSong = spotify.getSong();

        // Detect song change
        if (newSong.title != song.title)
        {
            notifications.show(
                "Now Playing",
                newSong.title
            );
        }

        // Smooth progress animation
        float targetProgress = 0.0f;

        if (newSong.duration > 0)
        {
            targetProgress =
                (float)newSong.progress /
                (float)newSong.duration;
        }

        song.animatedProgress +=
            (targetProgress - song.animatedProgress) * 0.15f;

        // Copy latest Spotify data
        song.title = newSong.title;
        song.artist = newSong.artist;
        song.progress = newSong.progress;
        song.duration = newSong.duration;
        song.playing = newSong.playing;
    }
    else
    {
        song.title = "Connecting...";
        song.artist = "Waiting for WiFi...";
        song.progress = 0;
        song.duration = 100;
        song.playing = false;

        song.animatedProgress +=
            (0.0f - song.animatedProgress) * 0.15f;
    }

    notifications.update();

    screen.setSong(song);
    screen.update();

    if (now - lastFrame >= FRAME_TIME)
    {
        lastFrame = now;

        display.beginFrame();

        screen.draw(display);

        notifications.draw(display);

        display.endFrame();
    }
}
