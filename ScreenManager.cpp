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
    // Only reset the marquee when the title changes
    if (song.title != currentSong.title)
    {
        // Temporary width estimate
        // Later we'll replace this with DisplayManager::getTextWidth()
        marquee.setText(song.title, song.title.length() * 8);
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

        case ScreenType::PLAYER:
        {
            display.drawPlayer(
                currentSong,
                marquee.offset()
            );
            break;
        }

        case ScreenType::LYRICS:
        {
            // Placeholder for now
            display.drawLoading("Lyrics...");
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
            display.drawError("Invalid Screen");
            break;
        }
    }
}