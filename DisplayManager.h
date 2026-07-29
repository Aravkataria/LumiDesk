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

    //--------------------------------------------------
    // Display
    //--------------------------------------------------

    void begin();

    void beginFrame();

    void endFrame();

    void setContrast(
        uint8_t value
    );

    void drawWipeMask(
        int x,
        int width
    );

    int getTextWidth(const String& text);

    //--------------------------------------------------
    // Screens
    //--------------------------------------------------

    void drawBoot();

    void drawLoading(
        const String& text
    );

    void drawPlayer(
        const SongInfo& song,
        const ClockInfo& clock,
        int titleX
    );

    void drawIdle(
        const IdleInfo& idle
    );

    void drawError(
        const String& message
    );

    //--------------------------------------------------
    // Notification
    //--------------------------------------------------

    void drawNotification(
        const String& title,
        const String& message,
        int yOffset
    );

    //--------------------------------------------------
    // Progress
    //--------------------------------------------------

    void drawProgressBar(
        int x,
        int y,
        int w,
        int h,
        float progress
    );

private:

    //--------------------------------------------------
    // OLED
    //--------------------------------------------------

    U8G2_SH1106_128X64_NONAME_F_HW_I2C display;

    static constexpr int X_OFFSET = 0;

    //--------------------------------------------------
    // Primitive Wrappers
    //--------------------------------------------------

    void drawStr(
        int x,
        int y,
        const char* text
    );

    void drawFrame(
        int x,
        int y,
        int w,
        int h
    );

    void drawBox(
        int x,
        int y,
        int w,
        int h
    );

    void drawHLine(
        int x,
        int y,
        int w
    );

    void drawRBox(
        int x,
        int y,
        int w,
        int h,
        int r
    );

    //--------------------------------------------------
    // Shared UI
    //--------------------------------------------------

    void drawHeader(
        const char* title,
        const ClockInfo& clock
    );

    void drawTimeBar(
        const SongInfo& song
    );

    void drawIdleWeather(
        const WeatherInfo& weather
    );

    //--------------------------------------------------
    // Helpers
    //--------------------------------------------------

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
};

#endif