#include "WeatherManager.h"

WeatherManager::WeatherManager()
{
}

bool WeatherManager::begin()
{
    ready = true;
    lastRefresh = 0;
    return true;
}

void WeatherManager::update()
{
    if (!ready)
        return;

    if (millis() - lastRefresh >= REFRESH_INTERVAL)
    {
        refresh();
    }
}

bool WeatherManager::refresh()
{
    lastRefresh = millis();

    return fetchWeather();
}

bool WeatherManager::fetchWeather()
{
    /*
        TODO

        Open-Meteo implementation

        WeatherAPI implementation

        API key from .env

        HTTPS request

        Parse JSON

    */

    return false;
}

bool WeatherManager::isReady() const
{
    return ready;
}

const WeatherData& WeatherManager::getWeather() const
{
    return weather;
}