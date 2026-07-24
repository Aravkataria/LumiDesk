#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "UIModels.h"

class DisplayManager
{
public:

    DisplayManager();

    void begin();

    void beginFrame();
    void endFrame();

    int getTextWidth(const String& text);

    void drawBoot();

    void drawLoading(const String& text);

    void drawPlayer(
        const SongInfo& song,
        int titleX
    );

    // NEW
    void drawLyrics(
        const SongInfo& song
    );

    void drawIdle();

    void drawError(
        const String& message
    );

    void drawNotification(
        const String& title,
        const String& message,
        int yOffset
    );

    void drawProgressBar(
        int x,
        int y,
        int w,
        int h,
        float progress
    );

private:

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
};

#endif