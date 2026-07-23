#include "SpotifyClient.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

SpotifyClient::SpotifyClient()
{
    connected = false;
    lastRequest = 0;
}

void SpotifyClient::begin(
    const char* wifiSSID,
    const char* wifiPassword,
    const char* url)
{
    ssid = wifiSSID;
    password = wifiPassword;
    serverURL = url;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println();
    Serial.println("========== Spotify Client ==========");
    Serial.println("Connecting to WiFi...");
}

void SpotifyClient::update()
{
    // Wait for WiFi
    if (WiFi.status() != WL_CONNECTED)
    {
        connected = false;
        return;
    }

    if (!connected)
    {
        connected = true;

        Serial.println();
        Serial.println("WiFi Connected!");

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());

        Serial.println();
    }

    // Poll every 3 seconds
    if (millis() - lastRequest < 3000)
        return;

    lastRequest = millis();

    WiFiClient client;

    Serial.println("--------------------------------");
    Serial.println("Connecting to server...");

    // IMPORTANT:
    // Replace with YOUR PC's IP if different
    if (!client.connect("192.168.1.34", 5000))
    {
        Serial.println("Connection FAILED!");
        return;
    }

    Serial.println("Connected!");
    Serial.println("Sending HTTP request...");

    client.println("GET /spotify HTTP/1.1");
    client.println("Host: 192.168.1.34");
    client.println("Connection: close");
    client.println();

    unsigned long timeout = millis();
    String response = "";

    while (client.connected() && (millis() - timeout < 5000))
    {
        while (client.available())
        {
            char c = client.read();
            response += c;
            timeout = millis();
        }
    }

    client.stop();

    Serial.println();
    Serial.println("========== SERVER RESPONSE ==========");
    Serial.println(response);
    Serial.println("=====================================");

    // Find JSON body
    int jsonStart = response.indexOf('{');

    if (jsonStart == -1)
    {
        Serial.println("No JSON found.");
        return;
    }

    String json = response.substring(jsonStart);

    Serial.println("JSON:");
    Serial.println(json);

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, json);

    if (error)
    {
        Serial.print("JSON Parse Error: ");
        Serial.println(error.c_str());
        return;
    }

    currentSong.title = doc["title"] | "";
    currentSong.artist = doc["artist"] | "";
    currentSong.progress = doc["progress"] | 0;
    currentSong.duration = doc["duration"] | 1;
    currentSong.playing = doc["playing"] | false;

    Serial.println("Song updated successfully!");
}

bool SpotifyClient::isConnected()
{
    return connected;
}

SongInfo SpotifyClient::getSong()
{
    return currentSong;
}