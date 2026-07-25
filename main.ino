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

const char* WIFI_SSID = "WIFI_ID";
const char* WIFI_PASSWORD = "WIFI_PASS";

<<<<<<< HEAD
// Replace with your PC's LOCAL IP
const char* SERVER_URL = "http://192.168.1.34:8000/spotify";
=======
// IMPORTANT:
// Replace this with YOUR COMPUTER'S LOCAL IP
// Example:
// http://192.168.1.34:8000/spotify
const char* SERVER_URL = "http://192.168.x.x:8000/spotify";
>>>>>>> 13fa58a6fa534d5be6ff023b61fc90a00f182a8c

// ---------------------------

// Auto screen switching
bool showLyrics = false;
unsigned long lastScreenSwitch = 0;
const unsigned long SCREEN_SWITCH_TIME = 6000;

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
    song.album = "";

    song.currentLyric = "";
    song.nextLyric = "";
    song.hasLyrics = false;

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

        float targetProgress = 0.0f;

        if (newSong.duration > 0)
        {
            targetProgress =
                (float)newSong.progress /
                (float)newSong.duration;
        }

        song.animatedProgress +=
            (targetProgress - song.animatedProgress) * 0.15f;

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
        song.artist = "Waiting for WiFi...";
        song.album = "";

        song.currentLyric = "";
        song.nextLyric = "";
        song.hasLyrics = false;

        song.progress = 0;
        song.duration = 100;

        song.playing = false;

        song.animatedProgress +=
            (0.0f - song.animatedProgress) * 0.15f;
    }

    notifications.update();

    screen.setSong(song);

    // -----------------------------
    // Automatic Player/Lyrics switch
    // -----------------------------
    if (millis() - lastScreenSwitch >= SCREEN_SWITCH_TIME)
    {
        lastScreenSwitch = millis();

        if (song.hasLyrics)
        {
            showLyrics = !showLyrics;
        }
        else
        {
            showLyrics = false;
        }
    }

    if (showLyrics)
        screen.setScreen(ScreenType::LYRICS);
    else
        screen.setScreen(ScreenType::PLAYER);

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