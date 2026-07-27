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

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

    // Panel turned out to be SH1106, not SSD1306 - the SH1106 u8g2
    // driver (above) already handles that controller's column offset
    // internally, so this stays at 0. Keeping the wrapper functions
    // below so there's a single place to nudge things if any residual
    // misalignment shows up after switching drivers.
    static constexpr int X_OFFSET = 0;

    //---------------------------------------------------
    // Offset-compensated draw wrappers
    // (call these instead of display.drawXxx directly)
    //---------------------------------------------------

    void drawStr(int x, int y, const char* text)
    {
        display.drawStr(x + X_OFFSET, y, text);
    }

    void drawFrame(int x, int y, int w, int h)
    {
        display.drawFrame(x + X_OFFSET, y, w, h);
    }

    void drawBox(int x, int y, int w, int h)
    {
        display.drawBox(x + X_OFFSET, y, w, h);
    }

    void drawHLine(int x, int y, int w)
    {
        display.drawHLine(x + X_OFFSET, y, w);
    }

    void drawRBox(int x, int y, int w, int h, int r)
    {
        display.drawRBox(x + X_OFFSET, y, w, h, r);
    }

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