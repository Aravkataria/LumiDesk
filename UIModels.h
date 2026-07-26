#ifndef UI_MODELS_H
#define UI_MODELS_H

#include <Arduino.h>

enum class ScreenType
{
    BOOT,
    LOADING,
    PLAYER,
    LYRICS,
    IDLE,
    ERROR_SCREEN
};

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

    // Cached formatted time strings
    String elapsedText = "0:00";
    String durationText = "0:00";

    // Display flags
    bool titleChanged = false;
    bool lyricChanged = false;
};

#endif