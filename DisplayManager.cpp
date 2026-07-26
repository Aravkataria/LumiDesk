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

//----------------------------------------------------
// Helper Functions
//----------------------------------------------------

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

    int width = display.getStrWidth(text.c_str());

    display.drawStr(
        (128 - width) / 2,
        y,
        text.c_str()
    );
}

void DisplayManager::drawTimeBar(
    const SongInfo& song)
{
    char current[8];
    char total[8];

    sprintf(
        current,
        "%u:%02u",
        song.progress / 60000,
        (song.progress / 1000) % 60
    );

    sprintf(
        total,
        "%u:%02u",
        song.duration / 60000,
        (song.duration / 1000) % 60
    );

    display.setFont(u8g2_font_5x7_tf);

    display.drawStr(0, 63, current);

    display.drawStr(103, 63, total);

    drawProgressBar(
        28,
        59,
        70,
        4,
        song.animatedProgress
    );
}

//----------------------------------------------------
// Boot Screen
//----------------------------------------------------

void DisplayManager::drawBoot()
{
    display.setFont(u8g2_font_logisoso18_tf);

    int width = display.getStrWidth("Spotify");

    display.drawStr(
        (128 - width) / 2,
        28,
        "Spotify"
    );

    display.setFont(u8g2_font_6x12_tf);

    width = display.getStrWidth("OLED Pro");

    display.drawStr(
        (128 - width) / 2,
        48,
        "OLED Pro"
    );
}

//----------------------------------------------------
// Loading Screen
//----------------------------------------------------

void DisplayManager::drawLoading(
    const String& text)
{
    drawCenteredText(
        20,
        "Connecting...",
        u8g2_font_7x14B_tf
    );

    display.drawFrame(
        14,
        34,
        100,
        10
    );

    display.drawBox(
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
//----------------------------------------------------
// Main Player Screen
//----------------------------------------------------

void DisplayManager::drawPlayer(
    const SongInfo& song,
    int titleX)
{
    //--------------------------------------------------
    // Header
    //--------------------------------------------------

    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(0, 10, "Spotify");

    if (song.playing)
        display.drawStr(120, 10, ">");
    else
        display.drawStr(116, 10, "||");

    display.drawHLine(0, 14, 128);

    //--------------------------------------------------
    // Song Title
    //--------------------------------------------------

    display.setFont(u8g2_font_7x14B_tf);

    int titleWidth =
        display.getStrWidth(song.title.c_str());

    if (titleWidth <= 124)
    {
        display.drawStr(
            (128 - titleWidth) / 2,
            28,
            song.title.c_str()
        );
    }
    else
    {
        display.drawStr(
            titleX,
            28,
            song.title.c_str()
        );
    }

    //--------------------------------------------------
    // Artist
    //--------------------------------------------------

    String artist =
        fitText(
            song.artist,
            124,
            u8g2_font_5x7_tf
        );

    drawCenteredText(
        38,
        artist,
        u8g2_font_5x7_tf
    );

    //--------------------------------------------------
    // Current Lyric
    //--------------------------------------------------

    display.drawHLine(0, 42, 128);

    display.setFont(u8g2_font_6x12_tf);

    String lyric;

    if(song.hasLyrics)
    {
        lyric =
            fitText(
                song.currentLyric,
                112,
                u8g2_font_6x12_tf
            );
    }
    else
    {
        lyric = "No synced lyrics";
    }

    display.drawStr(
        2,
        54,
        "\x99"
    );

    display.drawStr(
        12,
        54,
        lyric.c_str()
    );

    //--------------------------------------------------
    // Bottom Time Bar
    //--------------------------------------------------

    drawTimeBar(song);
}
//----------------------------------------------------
// Idle Screen
//----------------------------------------------------

void DisplayManager::drawIdle()
{
    drawCenteredText(
        26,
        "Spotify",
        u8g2_font_logisoso18_tf
    );

    drawCenteredText(
        52,
        "Waiting for playback...",
        u8g2_font_5x7_tf
    );
}

//----------------------------------------------------
// Error Screen
//----------------------------------------------------

void DisplayManager::drawError(
    const String& message)
{
    drawCenteredText(
        20,
        "ERROR",
        u8g2_font_7x14B_tf
    );

    drawCenteredText(
        42,
        fitText(
            message,
            120,
            u8g2_font_6x12_tf
        ),
        u8g2_font_6x12_tf
    );
}

//----------------------------------------------------
// Notification
//----------------------------------------------------

void DisplayManager::drawNotification(
    const String& title,
    const String& message,
    int yOffset)
{
    display.drawRBox(
        2,
        yOffset,
        124,
        22,
        3
    );

    display.setDrawColor(0);

    display.setFont(u8g2_font_5x7_tf);

    display.drawStr(
        8,
        yOffset + 8,
        title.c_str()
    );

    String text =
        fitText(
            message,
            108,
            u8g2_font_6x12_tf
        );

    display.setFont(u8g2_font_6x12_tf);

    display.drawStr(
        8,
        yOffset + 19,
        text.c_str()
    );

    display.setDrawColor(1);
}

//----------------------------------------------------
// Progress Bar
//----------------------------------------------------

void DisplayManager::drawProgressBar(
    int x,
    int y,
    int w,
    int h,
    float progress)
{
    if(progress < 0.0f)
        progress = 0.0f;

    if(progress > 1.0f)
        progress = 1.0f;

    display.drawFrame(
        x,
        y,
        w,
        h
    );

    int fill =
        (w - 2) * progress;

    if(fill > 0)
    {
        display.drawBox(
            x + 1,
            y + 1,
            fill,
            h - 2
        );
    }
}