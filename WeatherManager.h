#pragma once

#include <Arduino.h>
#include "managers/WeatherData.h"

class WeatherManager
{
public:
    WeatherManager();

    bool begin();

    void update();

    bool refresh();

    bool isReady() const;

    const WeatherData& getWeather() const;

private:
    WeatherData weather;

    bool ready = false;

    unsigned long lastRefresh = 0;

    static constexpr uint32_t REFRESH_INTERVAL =
        5UL * 60UL * 1000UL;

    bool fetchWeather();
};