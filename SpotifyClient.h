#ifndef SPOTIFY_CLIENT_H
#define SPOTIFY_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "UIModels.h"

class SpotifyClient
{
private:

    const char* ssid;
    const char* password;

    // Store URL safely
    String serverURL;

    bool connected;

    SongInfo currentSong;

    unsigned long lastRequest;

    HTTPClient http;

public:

    SpotifyClient();

    void begin(
        const char* wifiSSID,
        const char* wifiPassword,
        const String& url
    );

    void update();

    bool isConnected();

    SongInfo getSong();
};

#endif