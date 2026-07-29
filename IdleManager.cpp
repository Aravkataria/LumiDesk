#include "IdleManager.h"

void IdleManager::begin()
{
    idle = false;
    pauseStart = 0;
}

void IdleManager::update(bool isPlaying, bool hasSession)
{
    if (isPlaying)
    {
        reset();
        return;
    }

    if (!hasSession)
    {
        idle = true;
        return;
    }

    if (pauseStart == 0)
    {
        pauseStart = millis();
    }

    if (!idle &&
        millis() - pauseStart >= IDLE_TIMEOUT_PAUSED)
    {
        idle = true;
    }
}

void IdleManager::reset()
{
    idle = false;
    pauseStart = 0;
}

bool IdleManager::isIdle() const
{
    return idle;
}