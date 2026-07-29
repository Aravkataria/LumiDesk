#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <Arduino.h>
#include <time.h>

#include "UIModels.h"

class ClockManager
{
public:
    ClockManager();

    bool begin();

    void update();

    const ClockInfo& getClock() const;

    bool isSynced() const;

private:
    ClockInfo clock;

    unsigned long lastUpdate = 0;

    static constexpr uint32_t UPDATE_INTERVAL = 1000;

    void refreshClock();
};

#endif