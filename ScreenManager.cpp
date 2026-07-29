#include "ScreenManager.h"
#include "DisplayManager.h"

ScreenManager::ScreenManager()
{
    currentScreen = ScreenType::BOOT;
    pendingScreen = ScreenType::BOOT;

    transitionPhase = TransitionPhase::NONE;
    transitionStart = 0;

    marquee.setDisplayWidth(128);
}

void ScreenManager::setScreen(ScreenType screen)
{
    // Already showing it - nothing to do. This matters because
    // main.ino calls setScreen() every loop, not just on change.
    if (screen == currentScreen && transitionPhase == TransitionPhase::NONE)
        return;

    // Already mid-transition toward this exact target.
    if (transitionPhase != TransitionPhase::NONE && pendingScreen == screen)
        return;

    if (transitionPhase == TransitionPhase::NONE)
    {
        // Fresh transition.
        pendingScreen = screen;
        transitionPhase = TransitionPhase::FADE_OUT;
        transitionStart = millis();
    }
    else
    {
        // A transition is already running (e.g. rapid play/pause
        // flapping) - just retarget where it lands, keep the
        // current fade timing so it doesn't jump/restart.
        pendingScreen = screen;
    }
}

ScreenType ScreenManager::getScreen()
{
    return currentScreen;
}

void ScreenManager::setSong(const SongInfo& song)
{
    // Restart marquee only if title changed
    if (song.title != currentSong.title)
    {
        int width = song.title.length() * 7;
        marquee.setText(song.title, width);
    }

    currentSong = song;
}

void ScreenManager::setIdleInfo(const IdleInfo& idle)
{
    currentIdle = idle;
}

float ScreenManager::easeOutQuad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

void ScreenManager::update()
{
    marquee.update();

    if (transitionPhase == TransitionPhase::NONE)
        return;

    unsigned long elapsed = millis() - transitionStart;

    if (transitionPhase == TransitionPhase::FADE_OUT)
    {
        if (elapsed >= TRANSITION_HALF_MS)
        {
            // Old screen has faded to black - swap content, then
            // fade the new one back in.
            currentScreen = pendingScreen;
            transitionPhase = TransitionPhase::FADE_IN;
            transitionStart = millis();
        }
    }
    else if (transitionPhase == TransitionPhase::FADE_IN)
    {
        if (elapsed >= TRANSITION_HALF_MS)
        {
            transitionPhase = TransitionPhase::NONE;
        }
    }
}

void ScreenManager::draw(DisplayManager& display)
{
    drawCurrentScreen(display);

    if (transitionPhase == TransitionPhase::NONE)
        return;

    float t = (float)(millis() - transitionStart) / (float)TRANSITION_HALF_MS;

    if (t > 1.0f)
        t = 1.0f;

    float eased = easeOutQuad(t);

    if (transitionPhase == TransitionPhase::FADE_OUT)
    {
        // Old screen is currently drawn above. Grow a black mask
        // in from the left, erasing it left-to-right down to
        // nothing (fully black) by the time this half ends.
        int hidden = (int)(128.0f * eased);
        display.drawWipeMask(0, hidden);
    }
    else
    {
        // New screen is currently drawn above. Shrink the black
        // mask away from the left, revealing it left-to-right -
        // continues the same direction as the fade-out.
        int revealed = (int)(128.0f * eased);
        display.drawWipeMask(revealed, 128 - revealed);
    }
}

void ScreenManager::drawCurrentScreen(DisplayManager& display)
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
                currentIdle.clock,
                marquee.offset()
            );
            break;
        }

        case ScreenType::LYRICS:
        {
            // Reserved for future use
            display.drawPlayer(
                currentSong,
                currentIdle.clock,
                marquee.offset()
            );
            break;
        }

        case ScreenType::IDLE:
        {
            display.drawIdle(currentIdle);
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
                currentIdle.clock,
                marquee.offset()
            );
            break;
        }
    }
}