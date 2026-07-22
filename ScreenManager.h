#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <Arduino.h>

#include "UIModels.h"
#include "MarqueeManager.h"

class DisplayManager;

class ScreenManager
{
private:
    ScreenType currentScreen;

    SongInfo currentSong;

    MarqueeManager marquee;

public:
    ScreenManager();

    void setScreen(ScreenType screen);

    ScreenType getScreen();

    void setSong(const SongInfo& song);

    void update();

    void draw(DisplayManager& display);
};

#endif