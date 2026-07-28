#include "DisplayManager.h"
#include "NotificationManager.h"
#include "ScreenManager.h"
#include "SpotifyClient.h"
#include "IdleManager.h"
#include "ClockManager.h"
#include "WeatherManager.h"

DisplayManager display;
NotificationManager notifications;
ScreenManager screen;
SpotifyClient spotify;
IdleManager idleManager;
ClockManager clockManager;
WeatherManager weatherManager;

SongInfo song;

unsigned long lastFrame = 0;
const uint16_t FRAME_TIME = 16;      // ~60 FPS

// WiFi
const char* WIFI_SSID = "WIFI_ID";
const char* WIFI_PASSWORD = "WIFI_PASS";

// Backend URL
const char* SERVER_URL = "http://192.168.x.x:8000/spotify";
const char* WEATHER_URL = "http://192.168.x.x:8000/weather";

void setup()
{
    Serial.begin(115200);

    display.begin();

    idleManager.begin();

    clockManager.begin();

    weatherManager.begin(WEATHER_URL);

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

        if (
            newSong.title != song.title ||
            newSong.artist != song.artist
        )
        {
            notifications.show(
                "Now Playing",
                newSong.title
            );
        }

        float target = 0.0f;

        if (newSong.duration > 0)
        {
            target =
                (float)newSong.progress /
                (float)newSong.duration;
        }

        // Smooth progress animation
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

        // Update idle timer
        idleManager.update(song.playing);
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

        idleManager.update(false);
    }

    notifications.update();

    clockManager.update();
    weatherManager.update();

    IdleInfo idle;
    idle.clock = clockManager.getClock();
    idle.weather = weatherManager.getWeather();

    screen.setSong(song);
    screen.setIdleInfo(idle);

    // Automatically switch screens
    if (idleManager.isIdle())
    {
        screen.setScreen(ScreenType::IDLE);
    }
    else
    {
        screen.setScreen(ScreenType::PLAYER);
    }

    screen.update();

    if (millis() - lastFrame >= FRAME_TIME)
    {
        lastFrame = millis();

        display.beginFrame();

        screen.draw(display);

        notifications.draw(display);

        display.endFrame();
    }
}