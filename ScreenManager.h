#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <Arduino.h>

#include "UIModels.h"
#include "MarqueeManager.h"

class DisplayManager;

class ScreenManager
{
private:
    enum class TransitionPhase
    {
        NONE,
        FADE_OUT,
        FADE_IN
    };

    ScreenType currentScreen;
    ScreenType pendingScreen;

    TransitionPhase transitionPhase;
    unsigned long transitionStart;

    // Total transition is roughly 2x this - one half to fade the
    // old screen out, one half to fade the new one in.
    static constexpr uint16_t TRANSITION_HALF_MS = 110;

    SongInfo currentSong;
    IdleInfo currentIdle;

    MarqueeManager marquee;

    static float easeOutQuad(float t);

    void drawCurrentScreen(DisplayManager& display);

public:
    ScreenManager();

    void setScreen(ScreenType screen);
    ScreenType getScreen();

    void setSong(const SongInfo& song);
    void setIdleInfo(const IdleInfo& idle);

    void update();

    void draw(DisplayManager& display);
};

#endif