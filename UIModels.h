#ifndef UI_MODELS_H
#define UI_MODELS_H

#include <Arduino.h>

//--------------------------------------------------
// Screen Types
//--------------------------------------------------

enum class ScreenType
{
    BOOT,
    LOADING,
    PLAYER,
    LYRICS,
    IDLE,
    ERROR_SCREEN
};

//--------------------------------------------------
// Song Information
//--------------------------------------------------

struct SongInfo
{
    // Metadata
    String title;
    String artist;
    String album;

    // Lyrics
    String currentLyric;
    String nextLyric;

    bool hasLyrics = false;

    // Playback
    uint32_t duration = 0;
    uint32_t progress = 0;

    bool playing = false;

    // UI Animation
    float animatedProgress = 0.0f;

    // Cached formatted strings
    String elapsedText = "0:00";
    String durationText = "0:00";

    // UI flags
    bool titleChanged = false;
    bool lyricChanged = false;
};

//--------------------------------------------------
// Clock Information
//--------------------------------------------------

struct ClockInfo
{
    String time24 = "--:--";
    String day = "--------";
    String date = "-- --- ----";

    bool synced = false;
};

//--------------------------------------------------
// Weather Information
//--------------------------------------------------

struct WeatherInfo
{
    float temperature = 0.0f;
    float feelsLike = 0.0f;

    uint8_t humidity = 0;

    String condition = "--";
    String icon = "";

    bool valid = false;

    unsigned long lastUpdate = 0;
};

//--------------------------------------------------
// Wi-Fi Information
//--------------------------------------------------

struct WiFiInfo
{
    bool connected = false;

    int8_t signal = -100;
};

//--------------------------------------------------
// Idle Screen Information
//--------------------------------------------------

struct IdleInfo
{
    ClockInfo clock;
    WeatherInfo weather;
    WiFiInfo wifi;
};

#endif