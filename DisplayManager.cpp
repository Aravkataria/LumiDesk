#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : display(U8G2_R0)
{
}

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

void DisplayManager::drawBoot()
{
    display.setFont(u8g2_font_logisoso18_tf);

    int w = display.getStrWidth("Spotify");
    display.drawStr((128 - w) / 2, 28, "Spotify");

    display.setFont(u8g2_font_6x12_tf);

    w = display.getStrWidth("OLED Pro");
    display.drawStr((128 - w) / 2, 48, "OLED Pro");
}

void DisplayManager::drawLoading(const String& text)
{
    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(8, 18, "Loading...");

    display.drawFrame(8, 28, 112, 10);
    display.drawBox(10, 30, 40, 6);

    display.drawStr(8, 54, text.c_str());
}

void DisplayManager::drawPlayer(const SongInfo& song, int titleX)
{
    // Header
    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(0, 10, "Spotify");

    if (song.playing)
        display.drawStr(116, 10, ">");
    else
        display.drawStr(116, 10, "||");

    display.drawHLine(0, 14, 128);

    // Song title
    display.setFont(u8g2_font_7x14B_tf);
    display.drawStr(titleX, 30, song.title.c_str());

    // Artist
    display.setFont(u8g2_font_6x12_tf);
    display.drawStr(0, 46, song.artist.c_str());

    // Smooth animated progress
    drawProgressBar(
        0,
        56,
        128,
        6,
        song.animatedProgress
    );
}

void DisplayManager::drawIdle()
{
    display.setFont(u8g2_font_logisoso20_tf);

    display.drawStr(18, 34, "22:41");

    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(26, 60, "Waiting Spotify...");
}

void DisplayManager::drawError(const String& message)
{
    display.setFont(u8g2_font_7x14B_tf);

    display.drawStr(0, 18, "ERROR");

    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(0, 40, message.c_str());
}

void DisplayManager::drawNotification(
    const String& title,
    const String& message,
    int yOffset)
{
    display.drawBox(0, yOffset, 128, 24);

    display.setDrawColor(0);

    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(6, yOffset + 9, title.c_str());
    display.drawStr(6, yOffset + 20, message.c_str());

    display.setDrawColor(1);
}

void DisplayManager::drawProgressBar(
    int x,
    int y,
    int w,
    int h,
    float progress)
{
    if (progress < 0)
        progress = 0;

    if (progress > 1)
        progress = 1;

    display.drawFrame(x, y, w, h);

    int fill = (w - 2) * progress;

    display.drawBox(
        x + 1,
        y + 1,
        fill,
        h - 2
    );
}