#include "ScreenManager.h"
#include "DisplayManager.h"

ScreenManager::ScreenManager()
{
    currentScreen = ScreenType::BOOT;

    marquee.setDisplayWidth(128);
}

void ScreenManager::setScreen(ScreenType screen)
{
    currentScreen = screen;
}

ScreenType ScreenManager::getScreen()
{
    return currentScreen;
}

void ScreenManager::setSong(const SongInfo& song)
{
    // Only restart marquee if title changed
    if (song.title != currentSong.title)
    {
        int width = song.title.length() * 7;
        marquee.setText(song.title, width);
    }

    currentSong = song;
}

void ScreenManager::update()
{
    marquee.update();
}

void ScreenManager::draw(DisplayManager& display)
{
    switch (currentScreen)
    {
        case ScreenType::BOOT:
        {
            display.drawBoot();
            break;
        }

        case ScreenType::LOADING:
        {
            display.drawLoading("Connecting...");
            break;
        }

        // Main Screen
        case ScreenType::PLAYER:
        {
            display.drawPlayer(
                currentSong,
                marquee.offset()
            );
            break;
        }

        // Lyrics screen removed
        case ScreenType::LYRICS:
        {
            display.drawPlayer(
                currentSong,
                marquee.offset()
            );
            break;
        }

        case ScreenType::IDLE:
        {
            display.drawIdle();
            break;
        }

        case ScreenType::ERROR_SCREEN:
        {
            display.drawError("Unknown Error");
            break;
        }

        default:
        {
            display.drawPlayer(
                currentSong,
                marquee.offset()
            );
            break;
        }
    }
}