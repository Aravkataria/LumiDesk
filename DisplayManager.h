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

    // Frame rendering
    void beginFrame();
    void endFrame();

    // Utilities
    int getTextWidth(const String& text);

    // Screens
    void drawBoot();
    void drawLoading(const String& text);
    void drawPlayer(const SongInfo& song, int titleX);
    void drawIdle();
    void drawError(const String& message);

    // Overlay
    void drawNotification(const String& title,
                          const String& message,
                          int yOffset);

private:

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

    void drawProgressBar(int x,
                         int y,
                         int w,
                         int h,
                         float progress);
};

#endif