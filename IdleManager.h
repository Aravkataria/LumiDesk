#pragma once

#include <Arduino.h>

class IdleManager
{
public:
    void begin();

    // hasSession: false when there's no active media session at all
    // (Spotify closed / "Nothing Playing") - nothing to resume, so
    // this switches to idle immediately instead of waiting.
    void update(bool isPlaying, bool hasSession);

    bool isIdle() const;

    void reset();

private:
    bool idle = false;

    unsigned long pauseStart = 0;

    static constexpr uint32_t IDLE_TIMEOUT_PAUSED =
        10UL * 60UL * 1000UL;   // 10 minutes - song loaded, just paused
};