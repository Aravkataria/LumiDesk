#pragma once

#include <Arduino.h>

struct WeatherData
{
    float temperature = 0.0f;
    float feelsLike = 0.0f;

    uint8_t humidity = 0;

    String condition;
    String icon;

    bool valid = false;

    unsigned long lastUpdate = 0;
};