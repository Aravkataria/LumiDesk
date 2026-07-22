#include "DisplayManager.h"
#include "NotificationManager.h"
#include "ScreenManager.h"

DisplayManager display;
NotificationManager notifications;
ScreenManager screen;

SongInfo song;

unsigned long lastFrame = 0;

const uint16_t FRAME_TIME = 33;

void setup()
{
    display.begin();

    song.title = "Blinding Lights";
    song.artist = "The Weeknd";
    song.duration = 200;
    song.progress = 60;
    song.animatedProgress = (float)song.progress / song.duration;
    song.playing = true;

    screen.setSong(song);
    screen.setScreen(ScreenType::PLAYER);

    notifications.show(
        "NOW PLAYING",
        song.title
    );
}

void loop()
{
    unsigned long now = millis();

    if (now - lastFrame < FRAME_TIME)
        return;

    lastFrame = now;

    notifications.update();

    // Demo playback
    song.progress++;

    if (song.progress > song.duration)
        song.progress = 0;

    // Smooth progress animation
    float targetProgress = 0.0f;

    if (song.duration > 0)
    {
        targetProgress = (float)song.progress / song.duration;
    }

    song.animatedProgress +=
        (targetProgress - song.animatedProgress) * 0.15f;

    screen.setSong(song);

    screen.update();

    display.beginFrame();

    screen.draw(display);

    notifications.draw(display);

    display.endFrame();
}