#include "IdleManager.h"

void IdleManager::begin()
{
    idle = false;
    pauseStart = 0;
}

void IdleManager::update(bool isPlaying)
{
    if (isPlaying)
    {
        reset();
        return;
    }

    if (pauseStart == 0)
    {
        pauseStart = millis();
    }

    if (!idle &&
        millis() - pauseStart >= IDLE_TIMEOUT)
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