#include "WeatherManager.h"
#include <ArduinoJson.h>

WeatherManager::WeatherManager()
{
}

bool WeatherManager::begin(const String& url)
{
    weatherURL = url;

    ready = true;
    lastRefresh = 0;   // Force immediate update

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
    if (weatherURL.isEmpty() || WiFi.status() != WL_CONNECTED)
        return false;

    HTTPClient http;

    http.begin(weatherURL);
    http.setTimeout(5000);

    int code = http.GET();

    if (code != HTTP_CODE_OK)
    {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;

    DeserializationError err = deserializeJson(doc, payload);

    if (err)
        return false;

    weather.temperature = doc["temperature"] | 0.0f;
    weather.feelsLike   = doc["feelsLike"] | 0.0f;
    weather.humidity    = doc["humidity"] | 0;
    weather.condition   = doc["condition"] | "--";
    weather.icon        = doc["icon"] | "";

    weather.valid = true;
    weather.lastUpdate = millis();

    return true;
}

bool WeatherManager::isReady() const
{
    return ready;
}

const WeatherInfo& WeatherManager::getWeather() const
{
    return weather;
}