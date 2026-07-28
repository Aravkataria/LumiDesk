#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : display(U8G2_R0)
{
}

//--------------------------------------------------
// Display
//--------------------------------------------------

void DisplayManager::begin()
{
    Wire.begin();

    display.begin();

    display.enableUTF8Print();

    display.clearBuffer();
    display.sendBuffer();
}

void DisplayManager::beginFrame()
{
    display.clearBuffer();
}

void DisplayManager::endFrame()
{
    display.sendBuffer();
}

int DisplayManager::getTextWidth(const String& text)
{
    display.setFont(u8g2_font_7x14B_tf);
    return display.getStrWidth(text.c_str());
}

//--------------------------------------------------
// Primitive Wrappers
//--------------------------------------------------

void DisplayManager::drawStr(
    int x,
    int y,
    const char* text)
{
    display.drawStr(
        x + X_OFFSET,
        y,
        text
    );
}

void DisplayManager::drawFrame(
    int x,
    int y,
    int w,
    int h)
{
    display.drawFrame(
        x + X_OFFSET,
        y,
        w,
        h
    );
}

void DisplayManager::drawBox(
    int x,
    int y,
    int w,
    int h)
{
    display.drawBox(
        x + X_OFFSET,
        y,
        w,
        h
    );
}

void DisplayManager::drawHLine(
    int x,
    int y,
    int w)
{
    display.drawHLine(
        x + X_OFFSET,
        y,
        w
    );
}

void DisplayManager::drawRBox(
    int x,
    int y,
    int w,
    int h,
    int r)
{
    display.drawRBox(
        x + X_OFFSET,
        y,
        w,
        h,
        r
    );
}

//--------------------------------------------------
// Helpers
//--------------------------------------------------

String DisplayManager::fitText(
    const String& text,
    int maxWidth,
    const uint8_t* font)
{
    display.setFont(font);

    if (display.getStrWidth(text.c_str()) <= maxWidth)
        return text;

    String value = text;

    while (value.length() > 0)
    {
        value.remove(value.length() - 1);

        String temp = value + "...";

        if (display.getStrWidth(temp.c_str()) <= maxWidth)
            return temp;
    }

    return "...";
}

void DisplayManager::drawCenteredText(
    int y,
    const String& text,
    const uint8_t* font)
{
    display.setFont(font);

    int width =
        display.getStrWidth(text.c_str());

    drawStr(
        (128 - width) / 2,
        y,
        text.c_str()
    );
}

//--------------------------------------------------
// Shared Header
//--------------------------------------------------

void DisplayManager::drawHeader(
    const char* title,
    const ClockInfo& clock)
{
    display.setFont(
        u8g2_font_6x12_tf
    );

    drawStr(
        0,
        10,
        title
    );

    if (clock.synced)
    {
        int width =
            display.getStrWidth(
                clock.time24.c_str()
            );

        drawStr(
            128 - width,
            10,
            clock.time24.c_str()
        );
    }

    drawHLine(
        0,
        14,
        128
    );
}

//--------------------------------------------------
// Time Bar
//--------------------------------------------------

void DisplayManager::drawTimeBar(
    const SongInfo& song)
{
    char current[8];
    char total[8];

    snprintf(
        current,
        sizeof(current),
        "%u:%02u",
        song.progress / 60000,
        (song.progress / 1000) % 60
    );

    snprintf(
        total,
        sizeof(total),
        "%u:%02u",
        song.duration / 60000,
        (song.duration / 1000) % 60
    );

    display.setFont(
        u8g2_font_5x7_tf
    );

    drawStr(
        0,
        63,
        current
    );

    drawStr(
        103,
        63,
        total
    );

    drawProgressBar(
        28,
        59,
        70,
        4,
        song.animatedProgress
    );
}

//--------------------------------------------------
// Boot
//--------------------------------------------------

void DisplayManager::drawBoot()
{
    display.setFont(
        u8g2_font_logisoso18_tf
    );

    int width =
        display.getStrWidth(
            "LumiDesk"
        );

    drawStr(
        (128 - width) / 2,
        28,
        "LumiDesk"
    );

    display.setFont(
        u8g2_font_6x12_tf
    );

    width =
        display.getStrWidth(
            "Starting..."
        );

    drawStr(
        (128 - width) / 2,
        48,
        "Starting..."
    );
}

//--------------------------------------------------
// Loading
//--------------------------------------------------

void DisplayManager::drawLoading(
    const String& text)
{
    drawCenteredText(
        22,
        "Connecting...",
        u8g2_font_7x14B_tf
    );

    drawFrame(
        14,
        34,
        100,
        10
    );

    drawBox(
        16,
        36,
        40,
        6
    );

    drawCenteredText(
        58,
        text,
        u8g2_font_5x7_tf
    );
}
