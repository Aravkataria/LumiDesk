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
    Serial.println("========== Spotify OLED ==========");
    Serial.println("Connecting to WiFi...");
}

void SpotifyClient::update()
{
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

        Serial.print("IP: ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());

        Serial.println();
    }

    // Poll backend every 500 ms
    if (millis() - lastRequest < 500)
        return;

    lastRequest = millis();

    WiFiClient client;

    //-------------------------------------------------
    // Parse URL
    //-------------------------------------------------

    String url = String(serverURL);

    url.replace("http://", "");

    int slash = url.indexOf('/');

    String hostPort = url.substring(0, slash);
    String path = url.substring(slash);

    String host = hostPort;
    int port = 80;

    int colon = hostPort.indexOf(':');

    if (colon >= 0)
    {
        host = hostPort.substring(0, colon);
        port = hostPort.substring(colon + 1).toInt();
    }

    //-------------------------------------------------

    Serial.println("--------------------------------");
    Serial.print("Connecting to ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);

    if (!client.connect(host.c_str(), port))
    {
        Serial.println("Server connection failed.");
        return;
    }

    client.print(
        String("GET ") + path + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Connection: close\r\n\r\n"
    );

    unsigned long timeout = millis();

    String response;

    while (client.connected() &&
           millis() - timeout < 5000)
    {
        while (client.available())
        {
            response += (char)client.read();
            timeout = millis();
        }
    }

    client.stop();

    int jsonStart = response.indexOf('{');

    if (jsonStart < 0)
    {
        Serial.println("No JSON received.");
        return;
    }

    String json = response.substring(jsonStart);

    JsonDocument doc;

    auto error = deserializeJson(doc, json);

    if (error)
    {
        Serial.print("JSON Error: ");
        Serial.println(error.c_str());
        return;
    }

    //-------------------------------------------------
    // Spotify Info
    //-------------------------------------------------

    currentSong.title = doc["title"] | "";
    currentSong.artist = doc["artist"] | "";
    currentSong.album = doc["album"] | "";

    currentSong.progress = doc["progress"] | 0;
    currentSong.duration = doc["duration"] | 1;

    currentSong.playing = doc["playing"] | false;

    //-------------------------------------------------
    // Lyrics
    //-------------------------------------------------

    currentSong.currentLyric =
        doc["current_lyric"] | "";

    currentSong.nextLyric =
        doc["next_lyric"] | "";

    currentSong.hasLyrics =
        doc["has_lyrics"] | false;

    Serial.println();

    Serial.println("========== Spotify ==========");

    Serial.print("Title : ");
    Serial.println(currentSong.title);

    Serial.print("Artist: ");
    Serial.println(currentSong.artist);

    Serial.print("Album : ");
    Serial.println(currentSong.album);

    if (currentSong.hasLyrics)
    {
        Serial.println();

        Serial.print("Current: ");
        Serial.println(currentSong.currentLyric);

        Serial.print("Next   : ");
        Serial.println(currentSong.nextLyric);
    }

    Serial.println("=============================");
}

bool SpotifyClient::isConnected()
{
    return connected;
}

SongInfo SpotifyClient::getSong()
{
    return currentSong;
}