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

    // Screens
    void drawBoot();
    void drawLoading(const String& text);

    // ONLY MAIN PLAYER SCREEN
    void drawPlayer(
        const SongInfo& song,
        int titleX
    );

    void drawIdle();

    void drawError(
        const String& message
    );

    // Notifications
    void drawNotification(
        const String& title,
        const String& message,
        int yOffset
    );

    // Progress
    void drawProgressBar(
        int x,
        int y,
        int w,
        int h,
        float progress
    );

private:

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;

    //---------------------------------------------------
    // Helpers
    //---------------------------------------------------

    String fitText(
        const String& text,
        int maxWidth,
        const uint8_t* font
    );

    void drawCenteredText(
        int y,
        const String& text,
        const uint8_t* font
    );

    void drawTimeBar(
        const SongInfo& song
    );
};

#endif