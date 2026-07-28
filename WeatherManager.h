#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "UIModels.h"

class WeatherManager
{
public:
    WeatherManager();

    bool begin(const char* url);

    void update();

    bool refresh();

    bool isReady() const;

    const WeatherInfo& getWeather() const;

private:
    WeatherInfo weather;

    const char* weatherURL = nullptr;

    bool ready = false;

    unsigned long lastRefresh = 0;

    static constexpr uint32_t REFRESH_INTERVAL =
        5UL * 60UL * 1000UL;

    bool fetchWeather();
};