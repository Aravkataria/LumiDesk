#include "DisplayManager.h"
#include "NotificationManager.h"
#include "ScreenManager.h"
#include "SpotifyClient.h"
#include "IdleManager.h"
#include "ClockManager.h"
#include "WeatherManager.h"
#include "secrets.h"
#include <HTTPClient.h>

DisplayManager display;
NotificationManager notifications;
ScreenManager screen;
SpotifyClient spotify;
IdleManager idleManager;
ClockManager clockManager;
WeatherManager weatherManager;

SongInfo song;
unsigned long lastFrame = 0;
const uint16_t FRAME_TIME = 16; // ~60 FPS

// WiFi
const char* WIFI_SSID = ENV_WIFI_SSID;
const char* WIFI_PASSWORD = ENV_WIFI_PASSWORD;

// Backend auth — must match the "security.api_key" value LumiDesk's
// tray shows via "Copy API key (for ESP setup)". Add
// #define ENV_API_KEY "..." to secrets.h with that value.
const char* API_KEY = ENV_API_KEY;

// Backend URL
String spotifyUrl = String(SERVER_URL) + "/spotify";
String weatherUrl = String(SERVER_URL) + "/weather";
String volumeUrl  = String(SERVER_URL) + "/volume";

// --- Button (temporary manual screen override) ---
const int BUTTON_PIN = 25;
bool manualOverride = false;
ScreenType manualScreen = ScreenType::PLAYER;
unsigned long lastOverrideTime = 0;
const unsigned long OVERRIDE_TIMEOUT = 8000; // ms the manual peek holds before auto takes back over

int rawReading = HIGH;
int stableState = HIGH;
unsigned long lastRawChangeTime = 0;
const unsigned long DEBOUNCE_MS = 50;

// --- Potentiometer (volume) ---
const int POT_PIN = 34;
const int POT_SAMPLES = 16;       // oversample to average out ADC noise
int lastSentVolume = -1;
int pendingVolume = -1;           // last computed value, awaiting stability
unsigned long pendingSince = 0;
const unsigned long STABLE_MS = 300;   // value must hold steady this long before sending
const int VOL_DEADBAND = 3;            // ignore changes smaller than this
unsigned long lastVolPost = 0;
const unsigned long VOL_POST_INTERVAL = 150;

int readPotAveraged()
{
  long sum = 0;
  for (int i = 0; i < POT_SAMPLES; i++)
  {
    sum += analogRead(POT_PIN);
    delayMicroseconds(200);
  }
  int raw = sum / POT_SAMPLES;          // 0-4095, smoothed
  return map(raw, 0, 4095, 0, 100);     // 0-100%
}

void handleButton()
{
  int reading = digitalRead(BUTTON_PIN);

  if (reading != rawReading)
  {
    rawReading = reading;
    lastRawChangeTime = millis();
  }

  if ((millis() - lastRawChangeTime) > DEBOUNCE_MS)
  {
    if (rawReading != stableState)
    {
      stableState = rawReading;

      if (stableState == LOW)
      {
        // Real, debounced press - peek at the other screen.
        manualScreen = (screen.getScreen() == ScreenType::PLAYER)
                         ? ScreenType::IDLE
                         : ScreenType::PLAYER;
        manualOverride = true;
        lastOverrideTime = millis();
        Serial.print("Button pressed -> manual screen: ");
        Serial.println(manualScreen == ScreenType::IDLE ? "IDLE" : "PLAYER");
      }
    }
  }
}

void handleVolume()
{
  if (millis() - lastVolPost < VOL_POST_INTERVAL) return;

  int volume = readPotAveraged();

  if (pendingVolume < 0 || abs(volume - pendingVolume) >= VOL_DEADBAND)
  {
    pendingVolume = volume;
    pendingSince = millis();
    return;
  }

  bool isStable = (millis() - pendingSince) >= STABLE_MS;
  bool differsFromSent = (lastSentVolume < 0) || (abs(pendingVolume - lastSentVolume) >= VOL_DEADBAND);

  if (isStable && differsFromSent)
  {
    HTTPClient http;
    http.begin(volumeUrl + "?level=" + String(pendingVolume));
    http.addHeader("X-API-Key", API_KEY);
    int code = http.GET();
    Serial.printf("Volume POST -> %d (HTTP %d)\n", pendingVolume, code);
    http.end();

    lastSentVolume = pendingVolume;
    lastVolPost = millis();
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // POT_PIN needs no pinMode call - analogRead handles ADC pins directly

  display.begin();
  idleManager.begin();
  clockManager.begin();
  weatherManager.begin(weatherUrl);
  spotify.begin(
    WIFI_SSID,
    WIFI_PASSWORD,
    spotifyUrl
  );

  song.title = "Connecting...";
  song.artist = "";
  song.album = "";
  song.currentLyric = "";
  song.nextLyric = "";
  song.duration = 100;
  song.progress = 0;
  song.playing = false;
  song.hasLyrics = false;
  song.animatedProgress = 0;

  screen.setSong(song);
  screen.setScreen(ScreenType::PLAYER);

  notifications.show(
    "Spotify OLED",
    "Starting..."
  );
}

void loop()
{
  spotify.update();

  if (spotify.isConnected())
  {
    SongInfo newSong = spotify.getSong();

    if (
      newSong.active &&
      (
        newSong.title != song.title ||
        newSong.artist != song.artist
      )
    )
    {
      notifications.show(
        "Now Playing",
        newSong.title
      );
    }

    float target = 0.0f;
    if (newSong.duration > 0)
    {
      target =
        (float)newSong.progress /
        (float)newSong.duration;
    }

    // Smooth progress animation
    song.animatedProgress +=
      (target - song.animatedProgress) * 0.08f;

    song.title = newSong.title;
    song.artist = newSong.artist;
    song.album = newSong.album;
    song.source = newSong.source;
    song.progress = newSong.progress;
    song.duration = newSong.duration;
    song.playing = newSong.playing;
    song.currentLyric = newSong.currentLyric;
    song.nextLyric = newSong.nextLyric;
    song.hasLyrics = newSong.hasLyrics;
    song.active = newSong.active;

    idleManager.update(song.playing, song.active);
  }
  else
  {
    song.title = "Connecting...";
    song.artist = "Waiting for backend...";
    song.album = "";
    song.source = "Media";
    song.currentLyric = "";
    song.nextLyric = "";
    song.progress = 0;
    song.duration = 100;
    song.playing = false;
    song.hasLyrics = false;
    song.active = false;
    song.animatedProgress *= 0.9f;
    idleManager.update(false, false);
  }

  notifications.update();
  clockManager.update();
