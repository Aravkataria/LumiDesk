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

    display.setContrast(255);

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

void DisplayManager::setContrast(uint8_t value)
{
    display.setContrast(value);
}

void DisplayManager::drawWipeMask(int x, int width)
{
    if (width <= 0)
        return;

    display.setDrawColor(0);
    display.drawBox(x, 0, width, 64);
    display.setDrawColor(1);
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
//--------------------------------------------------
// Main Player Screen
//--------------------------------------------------

void DisplayManager::drawPlayer(
    const SongInfo& song,
    const ClockInfo& clock,
    int titleX)
{
    //--------------------------------------------------
    // Header
    //--------------------------------------------------

    drawHeader(
        song.source.c_str(),
        clock
    );

    //--------------------------------------------------
    // Whether the lyrics row below will actually show
    // anything - a real lyric, or the Spotify "no lyrics"
    // placeholder. Everything else (YouTube, other sites
    // with no captions) has nothing to put there, so the
    // whole layout below the title shifts down to fill
    // the space instead of leaving it empty.
    //--------------------------------------------------

    bool isSpotify = (song.source == "Spotify");
    bool showLyricsRow = song.hasLyrics || isSpotify;

    int artistY = showLyricsRow ? 38 : 44;
    int dividerY = showLyricsRow ? 42 : 50;

    //--------------------------------------------------
    // Song Title
    //--------------------------------------------------

    display.setFont(
        u8g2_font_7x14B_tf
    );

    int titleWidth =
        display.getStrWidth(
            song.title.c_str()
        );

    if(titleWidth <= 122)
    {
        drawStr(
            (128 - titleWidth) / 2,
            28,
            song.title.c_str()
        );
    }
    else
    {
        drawStr(
            titleX,
            28,
            song.title.c_str()
        );
    }

    //--------------------------------------------------
    // Artist (skipped entirely when there's no reliable
    // creator name for this source - e.g. a random site)
    //--------------------------------------------------

    if (song.artist.length() > 0)
    {
        display.setFont(
            u8g2_font_5x7_tf
        );

        String artist =
            fitText(
                song.artist,
                122,
                u8g2_font_5x7_tf
            );

        drawCenteredText(
            artistY,
            artist,
            u8g2_font_5x7_tf
        );
    }

    //--------------------------------------------------
    // Divider - only when the lyrics row below is going
    // to show something (a real lyric, or the Spotify
    // "no lyrics" placeholder). Otherwise there's nothing
    // to separate from, so skip it.
    //--------------------------------------------------

    if (showLyricsRow)
    {
        drawHLine(
            0,
            dividerY,
            128
        );
    }

    //--------------------------------------------------
    // Lyrics - real songs (Spotify) always show a line,
    // falling back to "No synced lyrics" when there isn't
    // one yet. Anything else (YouTube, other sites) has no
    // caption source at all, so it shows nothing rather
    // than a placeholder.
    //--------------------------------------------------

    if (showLyricsRow)
    {
        display.setFont(
            u8g2_font_6x12_tf
        );

        String lyric;

        if (song.hasLyrics)
        {
            lyric =
                fitText(
                    song.currentLyric,
                    122,
                    u8g2_font_6x12_tf
                );
        }
        else
        {
            lyric = "No synced lyrics";
        }

        // Centered, karaoke-style - the icon is dropped here since a
        // short centered word (or two) reads cleaner on its own than
        // icon + left-aligned text.

        drawCenteredText(
            54,
            lyric,
            u8g2_font_6x12_tf
        );
    }

    //--------------------------------------------------
    // Playback Indicator
    //--------------------------------------------------

    display.setFont(
        u8g2_font_5x7_tf
    );

    if(song.playing)
    {
        drawBox(
            118,
            45,
            2,
            9
        );

        drawBox(
            123,
            45,
            2,
            9
        );
    }
    else
    {
        display.drawTriangle(
            118,
            45,
            118,
            55,
            124,
            50
        );
    }

    //--------------------------------------------------
    // Bottom Time Bar
    //--------------------------------------------------

    drawTimeBar(
        song
    );
}
//--------------------------------------------------
// Idle Weather
//--------------------------------------------------

void DisplayManager::drawIdleWeather(
    const WeatherInfo& weather)
{
    if(!weather.valid)
    {
        drawCenteredText(
            60,
            "Connecting...",
            u8g2_font_5x7_tf
        );
        return;
    }

    char line[24];

    snprintf(
        line,
        sizeof(line),
        "%.0f C  %s",
        weather.temperature,
        weather.condition.c_str()
    );

    drawCenteredText(
        60,
        fitText(
            line,
            124,
            u8g2_font_6x12_tf
        ),
        u8g2_font_6x12_tf
    );
}

//--------------------------------------------------
// Idle Screen
//--------------------------------------------------

void DisplayManager::drawIdle(
    const IdleInfo& idle)
{
    display.setFont(
        u8g2_font_logisoso18_tf
    );

    int width =
        display.getStrWidth(
            idle.clock.time24.c_str()
        );

    drawStr(
        (128 - width) / 2,
        22,
        idle.clock.time24.c_str()
    );

    drawCenteredText(
        36,
        idle.clock.day,
        u8g2_font_6x12_tf
    );

    drawCenteredText(
        47,
        idle.clock.date,
        u8g2_font_5x7_tf
    );

    drawIdleWeather(
        idle.weather
    );
}

//--------------------------------------------------
// Error Screen
//--------------------------------------------------

void DisplayManager::drawError(
    const String& message)
{
    drawCenteredText(
        22,
        "ERROR",
        u8g2_font_7x14B_tf
    );

    drawCenteredText(
        44,
        fitText(
            message,
            120,
            u8g2_font_6x12_tf
        ),
        u8g2_font_6x12_tf
    );
}

//--------------------------------------------------
// Notification
//--------------------------------------------------

void DisplayManager::drawNotification(
    const String& title,
    const String& message,
    int yOffset)
{
    drawRBox(
        2,
        yOffset,
        124,
        22,
        3
    );

    display.setDrawColor(0);

    display.setFont(
        u8g2_font_5x7_tf
    );

    drawStr(
        8,
        yOffset + 8,
        title.c_str()
    );

    display.setFont(
        u8g2_font_6x12_tf
    );

    String text =
        fitText(
            message,
            108,
            u8g2_font_6x12_tf
        );

    drawStr(
        8,
        yOffset + 19,
        text.c_str()
    );

    display.setDrawColor(1);
}

//--------------------------------------------------
// Progress Bar
//--------------------------------------------------

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

    drawFrame(
        x,
        y,
        w,
        h
    );

    int fill =
        (w - 2) * progress;

    if(fill > 0)
    {
        drawBox(
            x + 1,
            y + 1,
            fill,
            h - 2
        );
    }
}