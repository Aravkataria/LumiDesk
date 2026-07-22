#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Arduino.h>

class DisplayManager;

class NotificationManager
{
private:

    String title;
    String message;

    bool visible;
    bool leaving;

    unsigned long showStart;

    float position;
    float target;

    const float speed = 0.18f;

public:

    NotificationManager();

    void show(const String& t,
              const String& m);

    void update();

    void draw(DisplayManager& display);

    bool isVisible();
};

#endif