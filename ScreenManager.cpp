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
    // Only reset marquee when the title changes
    if (song.title != currentSong.title)
    {
        // Approximate text width (8 px per character)
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
            display.drawLyrics(currentSong);
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