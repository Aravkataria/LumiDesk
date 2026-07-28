#pragma once

#include <Arduino.h>

class IdleManager
{
public:
    void begin();

    void update(bool isPlaying);

    bool isIdle() const;

    void reset();

private:
    bool idle = false;

    unsigned long pauseStart = 0;

    static constexpr uint32_t IDLE_TIMEOUT =
        10UL * 60UL * 1000UL;   // 10 minutes
};