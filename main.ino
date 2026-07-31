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

// Backend URL
String spotifyUrl = String(SERVER_URL) + "/spotify";
String weatherUrl = String(SERVER_URL) + "/weather";
String volumeUrl  = String(SERVER_URL) + "/volume";

// --- Button (manual screen toggle: PLAYER <-> IDLE) ---
const int BUTTON_PIN = 25;
bool manualIdleScreen = false;   // false = player, true = idle

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_MS = 50;

// --- Potentiometer (volume) ---
const int POT_PIN = 34;
int lastVolume = -1;
unsigned long lastVolPost = 0;
const unsigned long VOL_POST_INTERVAL = 200; // ms, throttle HTTP calls

void handleButton()
{
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState)
  {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS)
  {
    if (reading == LOW && lastButtonState == HIGH)
    {
      // button just pressed
      manualIdleScreen = !manualIdleScreen;
      Serial.println(manualIdleScreen ? "Button: -> IDLE" : "Button: -> PLAYER");
    }
  }
  lastButtonState = reading;
}

void handleVolume()
{
  if (millis() - lastVolPost < VOL_POST_INTERVAL) return;

  int raw = analogRead(POT_PIN);            // 0-4095
  int volume = map(raw, 0, 4095, 0, 100);   // 0-100%

  if (abs(volume - lastVolume) >= 2)        // ignore ADC jitter
  {
    HTTPClient http;
    http.begin(volumeUrl + "?level=" + String(volume));
    int code = http.GET();
    Serial.printf("Volume POST -> %d (HTTP %d)\n", volume, code);
    http.end();

    lastVolume = volume;
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
  weatherManager.update();

  handleButton();
  handleVolume();

  IdleInfo idle;
  idle.clock = clockManager.getClock();
  idle.weather = weatherManager.getWeather();
  screen.setSong(song);
  screen.setIdleInfo(idle);

  // Screen selection is now fully manual: the button toggles between
  // PLAYER and IDLE. No automatic switching based on playback state.
  screen.setScreen(manualIdleScreen ? ScreenType::IDLE : ScreenType::PLAYER);

  screen.update();

  if (millis() - lastFrame >= FRAME_TIME)
  {
    lastFrame = millis();
    display.beginFrame();
    screen.draw(display);
    notifications.draw(display);
    display.endFrame();
  }
}
