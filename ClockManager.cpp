#include "ClockManager.h"

ClockManager::ClockManager()
{
}

bool ClockManager::begin()
{
    // India Standard Time (UTC+5:30)
    configTime(
        19800,
        0,
        "pool.ntp.org",
        "time.nist.gov"
    );

    refreshClock();

    return true;
}

void ClockManager::update()
{
    if (millis() - lastUpdate >= UPDATE_INTERVAL)
    {
        lastUpdate = millis();
        refreshClock();
    }
}

const ClockInfo& ClockManager::getClock() const
{
    return clock;
}

bool ClockManager::isSynced() const
{
    return clock.synced;
}

void ClockManager::refreshClock()
{
    struct tm timeinfo;

    // Default getLocalTime() timeout is 5000ms - if NTP hasn't
    // synced yet it blocks the whole loop for 5 full seconds,
    // and this gets called every second from update(), so it can
    // stall boot for a long time. Passing 10ms makes it just
    // check instantly and try again next tick instead.
    if (!getLocalTime(&timeinfo, 10))
    {
        clock.synced = false;
        return;
    }

    clock.synced = true;

    char buffer[20];

    // 24-hour time
    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M",
        &timeinfo
    );

    clock.time24 = buffer;

    // Day
    strftime(
        buffer,
        sizeof(buffer),
        "%A",
        &timeinfo
    );

    clock.day = buffer;

    // Date
    strftime(
        buffer,
        sizeof(buffer),
        "%d %b %Y",
        &timeinfo
    );

    clock.date = buffer;
}