#include "SpotifyClient.h"

SpotifyClient::SpotifyClient()
{
    connected = false;
    lastRequest = 0;
}

void SpotifyClient::begin(
    const char* wifiSSID,
    const char* wifiPassword,
    const String& url)
{
    ssid = wifiSSID;
    password = wifiPassword;
    serverURL = url;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.println();
    Serial.println("====================================");
    Serial.println(" Spotify OLED");
    Serial.println("====================================");
    Serial.println("Connecting to WiFi...");
}

void SpotifyClient::update()
{
    //--------------------------------------------------
    // WiFi
    //--------------------------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        connected = false;
        return;
    }

    if (!connected)
    {
        connected = true;

        Serial.println();
        Serial.println("WiFi Connected");

        Serial.print("IP : ");
        Serial.println(WiFi.localIP());

        Serial.print("RSSI : ");
        Serial.println(WiFi.RSSI());

        Serial.println();
    }

    //--------------------------------------------------
    // Poll every 500ms
    //--------------------------------------------------

    if (millis() - lastRequest < 500)
        return;

    lastRequest = millis();

    //--------------------------------------------------
    // HTTP Request
    //--------------------------------------------------

    http.begin(serverURL);

    http.addHeader("X-API-Key", API_KEY);

    http.setTimeout(3000);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print("HTTP Error : ");
        Serial.println(httpCode);

        http.end();
        return;
    }

    String payload = http.getString();

    http.end();

    //--------------------------------------------------
    // JSON
    //--------------------------------------------------

    JsonDocument doc;

    DeserializationError err =
        deserializeJson(doc, payload);

    if (err)
    {
        Serial.print("JSON Error : ");
        Serial.println(err.c_str());
        return;
    }

    //--------------------------------------------------
    // API Version
    //--------------------------------------------------

    int apiVersion = doc["api_version"] | 1;

    if (apiVersion != 2)
    {
        Serial.println("Backend version mismatch.");
    }

    //--------------------------------------------------
    // Song
    //--------------------------------------------------

    currentSong.title =
        String((const char*)doc["title"]);

    currentSong.artist =
        String((const char*)doc["artist"]);

    currentSong.album =
        String((const char*)doc["album"]);

    currentSong.source =
        doc["source"] | "Spotify";

    currentSong.progress =
        doc["progress"] | 0;

    currentSong.duration =
        doc["duration"] | 1;

    currentSong.playing =
        doc["playing"] | false;

    currentSong.active =
        doc["active"] | false;

    //--------------------------------------------------
    // Lyrics
    //--------------------------------------------------

    currentSong.currentLyric =
        String((const char*)doc["current_lyric"]);

    currentSong.nextLyric =
        String((const char*)doc["next_lyric"]);

    currentSong.hasLyrics =
        doc["has_lyrics"] | false;

    //--------------------------------------------------
    // Debug
    //--------------------------------------------------

    Serial.println("----------------------------");

    Serial.println(currentSong.title);

    Serial.println(currentSong.artist);

    Serial.print("Source: ");
    Serial.println(currentSong.source);

    if (currentSong.hasLyrics)
    {
        Serial.print("♪ ");
        Serial.println(currentSong.currentLyric);
    }

    Serial.println("----------------------------");
}

bool SpotifyClient::isConnected()
{
    return connected;
}

SongInfo SpotifyClient::getSong()
{
    return currentSong;
}