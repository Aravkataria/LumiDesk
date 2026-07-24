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
    String title;
    String artist;
    String album;

    String currentLyric;
    String nextLyric;

    uint32_t duration = 0;
    uint32_t progress = 0;

    bool playing = false;
    bool hasLyrics = false;

    float animatedProgress = 0.0f;
};

#endif