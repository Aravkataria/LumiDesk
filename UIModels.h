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

    uint32_t duration = 0;

    // Actual Spotify progress
    uint32_t progress = 0;

    bool playing = false;

    // Animated progress (0.0 -> 1.0)
    float animatedProgress = 0.0f;
};

#endif